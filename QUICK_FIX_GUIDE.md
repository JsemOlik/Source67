# Source67 - Quick Fix Guide

**Priority fixes for immediate implementation**

This guide provides copy-paste ready fixes for the most critical issues.

---

## 🔴 CRITICAL FIXES (Do These First!)

### 1. Fix main.cpp Memory Leak (5 minutes)

**File:** `src/main.cpp`  
**Lines:** 31-33

**Replace:**
```cpp
auto app = new S67::Application(argv[0], argc > 1 ? argv[1] : "");
app->Run();
delete app;
```

**With:**
```cpp
auto app = std::make_unique<S67::Application>(argv[0], argc > 1 ? argv[1] : "");
app->Run();
// No delete needed - automatic cleanup
```

---

### 2. Fix Physics Timestep (30 minutes)

**File:** `src/Physics/PhysicsSystem.cpp`  
**Lines:** 99-102

**Add to top of file:**
```cpp
namespace {
    static float s_PhysicsAccumulator = 0.0f;
    static constexpr float FIXED_PHYSICS_DT = 1.0f / 60.0f;
    static constexpr int MAX_PHYSICS_STEPS = 5;
}
```

**Replace OnUpdate function:**
```cpp
void PhysicsSystem::OnUpdate(Timestep ts) {
    s_PhysicsAccumulator += ts.GetSeconds();
    
    int steps = 0;
    while (s_PhysicsAccumulator >= FIXED_PHYSICS_DT && steps < MAX_PHYSICS_STEPS) {
        s_PhysicsSystem->Update(FIXED_PHYSICS_DT, 1, s_TempAllocator, s_JobSystem);
        s_PhysicsAccumulator -= FIXED_PHYSICS_DT;
        steps++;
    }
    
    // Prevent spiral of death
    if (s_PhysicsAccumulator > FIXED_PHYSICS_DT * MAX_PHYSICS_STEPS) {
        s_PhysicsAccumulator = FIXED_PHYSICS_DT;
    }
}
```

---

### 3. Fix TempAllocator Leak (5 minutes)

**File:** `src/Physics/PlayerController.h`

**Add member variable:**
```cpp
class PlayerController {
private:
    JPH::TempAllocatorImpl m_TempAllocator{10 * 1024 * 1024};
    // ... rest of members
};
```

**File:** `src/Physics/PlayerController.cpp`  
**Line:** 225

**Replace:**
```cpp
JPH::TempAllocatorImpl allocator(10 * 1024 * 1024);
m_Character->Update(ts, JPH::Vec3(0, -9.81f, 0),
                    PhysicsSystem::GetBroadPhaseLayerFilter(),
                    PhysicsSystem::GetObjectLayerFilter(), bodyFilter,
                    JPH::ShapeFilter(), allocator);
```

**With:**
```cpp
m_Character->Update(ts, JPH::Vec3::sZero(),  // Gravity handled manually
                    PhysicsSystem::GetBroadPhaseLayerFilter(),
                    PhysicsSystem::GetObjectLayerFilter(), bodyFilter,
                    JPH::ShapeFilter(), m_TempAllocator);
```

---

### 4. Fix glfwTerminate Leak (30 minutes)

**File:** `src/Core/Window.mm`

**Add static counter:**
```cpp
static int s_WindowCount = 0;
```

**In Init() after successful window creation:**
```cpp
s_WindowCount++;
```

**Replace Shutdown():**
```cpp
void Window::Shutdown() { 
    if (m_Window) {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
    }
    
    s_WindowCount--;
    if (s_WindowCount == 0 && s_GLFWInitialized) {
        glfwTerminate();
        s_GLFWInitialized = false;
    }
}
```

---

### 5. Fix Window Creation Validation (10 minutes)

**File:** `src/Core/Window.mm`  
**Lines:** 57-59

**Replace:**
```cpp
m_Window = glfwCreateWindow((int)props.Width, (int)props.Height,
                            m_Data.Title.c_str(), nullptr, nullptr);
glfwMakeContextCurrent(m_Window);
```

**With:**
```cpp
m_Window = glfwCreateWindow((int)props.Width, (int)props.Height,
                            m_Data.Title.c_str(), nullptr, nullptr);
if (!m_Window) {
    S67_CORE_ERROR("Failed to create GLFW window!");
    glfwTerminate();
    s_GLFWInitialized = false;
    return;
}

glfwMakeContextCurrent(m_Window);
if (!glfwGetCurrentContext()) {
    S67_CORE_ERROR("Failed to make OpenGL context current!");
    glfwDestroyWindow(m_Window);
    m_Window = nullptr;
    glfwTerminate();
    s_GLFWInitialized = false;
    return;
}
```

---

### 6. Fix Logger Thread Safety (20 minutes)

**File:** `src/Core/Logger.h`

**Add to private section:**
```cpp
private:
    static std::mutex s_LogMutex;
```

**Update methods:**
```cpp
public:
    static const std::vector<LogEntry> GetLogHistory() { 
        std::lock_guard<std::mutex> lock(s_LogMutex);
        return s_LogHistory;
    }
    
    static void ClearLogHistory() { 
        std::lock_guard<std::mutex> lock(s_LogMutex);
        s_LogHistory.clear(); 
    }
    
    static void AddLogEntry(const LogEntry &entry) {
        std::lock_guard<std::mutex> lock(s_LogMutex);
        s_LogHistory.push_back(entry);
        
        // Limit history size
        if (s_LogHistory.size() > 10000) {
            s_LogHistory.erase(s_LogHistory.begin());
        }
    }
```

**File:** `src/Core/Logger.cpp`

**Add at top:**
```cpp
std::mutex Logger::s_LogMutex;
```

---

### 7. Fix Shader Compilation Leak (15 minutes)

**File:** `src/Renderer/Shader.cpp`  
**Lines:** 145-157

**Replace:**
```cpp
if (isCompiled == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
    
    std::vector<GLchar> infoLog(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
    
    glDeleteShader(shader);
    S67_CORE_ERROR("{0}", infoLog.data());
    S67_CORE_ASSERT(false, "Shader compilation failure!");
    break;
}
```

**With:**
```cpp
if (isCompiled == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
    
    std::vector<GLchar> infoLog(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
    
    glDeleteShader(shader);
    
    // Clean up program and all previously compiled shaders
    glDeleteProgram(program);
    for (auto id : shaderIDs) {
        glDeleteShader(id);
    }
    
    S67_CORE_ERROR("{0}", infoLog.data());
    S67_CORE_ASSERT(false, "Shader compilation failure!");
    return;  // Use return instead of break
}
```

---

## 🟠 HIGH PRIORITY FIXES

### 8. Add Rule of Five to OpenGL Classes (2-3 hours)

This template applies to: Shader, Texture, Buffer (both classes), VertexArray, Framebuffer

**Add to each class:**

```cpp
// Delete copy operations
ClassName(const ClassName&) = delete;
ClassName& operator=(const ClassName&) = delete;

// Implement move operations
ClassName(ClassName&& other) noexcept
    : m_RendererID(other.m_RendererID)
    /* move other members */ {
    other.m_RendererID = 0;
}

ClassName& operator=(ClassName&& other) noexcept {
    if (this != &other) {
        // Clean up existing resource
        glDeleteXXX(1, &m_RendererID);  // XXX = Textures/Buffers/etc
        
        // Move data
        m_RendererID = other.m_RendererID;
        /* move other members */
        
        // Nullify moved-from object
        other.m_RendererID = 0;
    }
    return *this;
}
```

**Initialize members in class definition:**
```cpp
uint32_t m_RendererID = 0;
```

---

### 9. Initialize Physics BodyID (2 minutes)

**File:** `src/Renderer/Entity.h`  
**Line:** 63

**Replace:**
```cpp
JPH::BodyID PhysicsBody;
```

**With:**
```cpp
JPH::BodyID PhysicsBody = JPH::BodyID::cInvalidBodyID;
```

---

### 10. Add Null Checks to Window Callbacks (30 minutes)

**File:** `src/Core/Window.mm`

**Pattern to apply to ALL callbacks (lines 66-164):**

```cpp
glfwSetWindowSizeCallback(
    m_Window, [](GLFWwindow *window, int width, int height) {
      WindowData *data = (WindowData *)glfwGetWindowUserPointer(window);
      if (!data) return;  // ✅ Add this line
      
      data->Width = width;
      data->Height = height;
      
      if (data->EventCallback) {
        WindowResizeEvent event(width, height);
        data->EventCallback(event);
      }
    });
```

Apply this pattern to:
- WindowSizeCallback
- WindowCloseCallback
- KeyCallback
- CharCallback
- MouseButtonCallback
- ScrollCallback
- CursorPosCallback
- WindowDropCallback

---

### 11. Fix ContentBrowser hash_value (2 minutes)

**File:** `src/ImGui/ContentBrowserPanel.cpp`

**Add at top of file after includes:**
```cpp
namespace {
    size_t hash_value(const std::filesystem::path& p) {
        return std::filesystem::hash_value(p);
    }
}
```

---

### 12. Add ImGui Context Validation (10 minutes)

**File:** `src/ImGui/ImGuiLayer.cpp`

**Update Begin() and End():**
```cpp
void ImGuiLayer::Begin() {
    if (!ImGui::GetCurrentContext()) {
        S67_CORE_ERROR("ImGui context not initialized!");
        return;
    }
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // ... rest of code
}

void ImGuiLayer::End() {
    if (!ImGui::GetCurrentContext()) return;
    
    // ... rest of code
}
```

**Update OnEvent():**
```cpp
void ImGuiLayer::OnEvent(Event &e) {
    if (m_BlockEvents && ImGui::GetCurrentContext()) {
        ImGuiIO &io = ImGui::GetIO();
        e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
        e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
    }
}
```

---

## 🛠️ QUICK CMake IMPROVEMENTS

**File:** `CMakeLists.txt`

### Pin Dependencies (Line 62, 92)

```cmake
# Jolt Physics
FetchContent_Declare(
    JoltPhysics
    GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics
    GIT_TAG v5.0.0  # ✅ Pin to specific version
)

# GLAD (pin to commit)
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG 649309d0643ea2c0ca823d2a8c14e802fe8e5e75  # ✅ Pin to commit
)
```

### Add Compiler Warnings (After line 16)

```cmake
# Compiler warnings
if(MSVC)
    target_compile_options(Source67 PRIVATE /W4)
else()
    target_compile_options(Source67 PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Sanitizers for debug builds
if(CMAKE_BUILD_TYPE MATCHES Debug)
    if(NOT MSVC)
        target_compile_options(Source67 PRIVATE -fsanitize=address,undefined)
        target_link_options(Source67 PRIVATE -fsanitize=address,undefined)
    endif()
endif()
```

### Use CONFIGURE_DEPENDS (Line 108)

```cmake
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/**/*.cpp"
    "src/**/*.c"
)
```

---

## 📝 Testing Checklist

After applying fixes, test:

- [ ] Application starts and shuts down cleanly (no leaks)
- [ ] Window creation works (test failure cases)
- [ ] Physics runs at consistent rate across framerates
- [ ] Shader compilation errors are handled gracefully
- [ ] Logging from multiple threads doesn't crash
- [ ] ImGui renders without errors
- [ ] CMake reconfiguration picks up new files
- [ ] Debug build with sanitizers runs clean
- [ ] All OpenGL resources are properly deleted

---

## 🎯 Estimated Time to Fix All Critical Issues

| Task | Time |
|------|------|
| main.cpp memory leak | 5 min |
| Physics timestep | 30 min |
| TempAllocator | 5 min |
| glfwTerminate | 30 min |
| Window validation | 10 min |
| Logger thread safety | 20 min |
| Shader leak | 15 min |
| **Total Critical** | **~2 hours** |
| Rule of Five (6 classes) | 2-3 hours |
| Window callbacks | 30 min |
| Other high priority | 1 hour |
| **Total High Priority** | **~4 hours** |
| **GRAND TOTAL** | **~6 hours** |

---

## 🚀 After Fixes

Run these commands to verify:

```bash
# Clean rebuild
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Run with sanitizers (Linux/macOS)
./build/Source67

# Check for leaks (Linux)
valgrind --leak-check=full ./build/Source67

# Run with different framerates
# Physics should remain consistent
```

---

**End of Quick Fix Guide**
