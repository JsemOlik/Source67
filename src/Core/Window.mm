#include "Window.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Events/WindowEvent.h"
#include "Logger.h"
#include "stb_image.h"
#include <filesystem>
#include <glad/glad.h>


#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <Cocoa/Cocoa.h>
#endif

#include <GLFW/glfw3.h>

namespace S67 {

static bool s_GLFWInitialized = false;

static void GLFWErrorCallback(int error, const char *description) {
  S67_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
}

Window *Window::Create(const WindowProps &props) { return new Window(props); }

Window::Window(const WindowProps &props) { Init(props); }

Window::~Window() { Shutdown(); }

void Window::Init(const WindowProps &props) {
  m_Data.Title = props.Title;
  m_Data.Width = props.Width;
  m_Data.Height = props.Height;

  S67_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width,
                props.Height);

  if (!s_GLFWInitialized) {
    int success = glfwInit();
    S67_CORE_ASSERT(success, "Could not initialize GLFW!");
    glfwSetErrorCallback(GLFWErrorCallback);
    s_GLFWInitialized = true;
  }

#ifdef __APPLE__
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER,
                 GLFW_FALSE); // Sometimes helps with ImGui coords
#else
  glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
#endif

  m_Window = glfwCreateWindow((int)props.Width, (int)props.Height,
                              m_Data.Title.c_str(), nullptr, nullptr);
  glfwMakeContextCurrent(m_Window);
  glfwSetWindowUserPointer(m_Window, &m_Data);

  int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  S67_CORE_ASSERT(status, "Failed to initialize GLAD!");

  // Set GLFW callbacks
  glfwSetWindowSizeCallback(
      m_Window, [](GLFWwindow *window, int width, int height) {
        WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);
        data.Width = width;
        data.Height = height;

        if (data.EventCallback) {
          WindowResizeEvent event(width, height);
          data.EventCallback(event);
        }
      });

  glfwSetWindowCloseCallback(m_Window, [](GLFWwindow *window) {
    WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);
    if (data.EventCallback) {
      WindowCloseEvent event;
      data.EventCallback(event);
    }
  });

  glfwSetKeyCallback(m_Window, [](GLFWwindow *window, int key, int scancode,
                                  int action, int mods) {
    WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);

    if (data.EventCallback) {
      switch (action) {
      case GLFW_PRESS: {
        KeyPressedEvent event(key, 0);
        data.EventCallback(event);
        break;
      }
      case GLFW_RELEASE: {
        KeyReleasedEvent event(key);
        data.EventCallback(event);
        break;
      }
      case GLFW_REPEAT: {
        KeyPressedEvent event(key, 1);
        data.EventCallback(event);
        break;
      }
      }
    }
  });

  glfwSetMouseButtonCallback(
      m_Window, [](GLFWwindow *window, int button, int action, int mods) {
        WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);

        if (data.EventCallback) {
          switch (action) {
          case GLFW_PRESS: {
            MouseButtonPressedEvent event(button);
            data.EventCallback(event);
            break;
          }
          case GLFW_RELEASE: {
            MouseButtonReleasedEvent event(button);
            data.EventCallback(event);
            break;
          }
          }
        }
      });

  glfwSetScrollCallback(
      m_Window, [](GLFWwindow *window, double xOffset, double yOffset) {
        WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);

        if (data.EventCallback) {
          MouseScrolledEvent event((float)xOffset, (float)yOffset);
          data.EventCallback(event);
        }
      });

  glfwSetCursorPosCallback(
      m_Window, [](GLFWwindow *window, double xPos, double yPos) {
        WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);

        if (data.EventCallback) {
          MouseMovedEvent event((float)xPos, (float)yPos);
          data.EventCallback(event);
        }
      });

  glfwSetDropCallback(
      m_Window, [](GLFWwindow *window, int count, const char **paths) {
        WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);

        std::vector<std::string> pathList;
        for (int i = 0; i < count; i++) {
          pathList.push_back(paths[i]);
        }

        if (data.EventCallback) {
          WindowDropEvent event(pathList);
          data.EventCallback(event);
        }
      });

  S67_CORE_INFO("Window initialized successfully");
}
void Window::SetCursorLocked(bool locked) {
  glfwSetInputMode(m_Window, GLFW_CURSOR,
                   locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void Window::SetIcon(const std::string &path) {
  int width, height, channels;
  stbi_set_flip_vertically_on_load(0); // GLFW expects top-left
  stbi_uc *data = stbi_load(path.c_str(), &width, &height, &channels, 4);
  if (data) {
    GLFWimage icon[1];
    icon[0].width = width;
    icon[0].height = height;
    icon[0].pixels = data;
    glfwSetWindowIcon(m_Window, 1, icon);
    stbi_image_free(data);
    S67_CORE_INFO("Applied window icon from {0}", path);

#ifdef __APPLE__
    // On macOS, glfwSetWindowIcon doesn't set the Dock icon.
    // We need to use Cocoa's setApplicationIconImage.
    @autoreleasepool {
      std::filesystem::path absPath = std::filesystem::absolute(path);
      NSString *nsPath =
          [NSString stringWithUTF8String:absPath.string().c_str()];
      NSImage *image = [[NSImage alloc] initWithContentsOfFile:nsPath];
      if (image) {
        [NSApp setApplicationIconImage:image];
        S67_CORE_INFO("Applied macOS Dock icon from {0}", absPath.string());
      } else {
        S67_CORE_ERROR("Failed to load macOS Dock icon from {0}",
                       absPath.string());
      }
    }
#endif
  } else {
    S67_CORE_ERROR("Failed to load window icon from {0}", path);
  }
}

void Window::Shutdown() { glfwDestroyWindow(m_Window); }

void Window::OnUpdate() {
  glfwPollEvents();
  glfwSwapBuffers(m_Window);
}

} // namespace S67
