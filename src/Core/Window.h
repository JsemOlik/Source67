#include "Base.h"
#include "Events/Event.h"
#include <string>
#include <functional>

struct GLFWwindow;

namespace S67 {

    struct WindowProps {
        std::string Title;
        uint32_t Width;
        uint32_t Height;

        WindowProps(const std::string& title = "Source67 Engine",
                    uint32_t width = 1280,
                    uint32_t height = 720)
            : Title(title), Width(width), Height(height) {}
    };

    class Window {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        Window(const WindowProps& props);
        ~Window();

        void OnUpdate();

        inline uint32_t GetWidth() const { return m_Data.Width; }
        inline uint32_t GetHeight() const { return m_Data.Height; }

        inline void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }

        inline void* GetNativeWindow() const { return m_Window; }

        static Window* Create(const WindowProps& props = WindowProps());

    private:
        void Init(const WindowProps& props);
        void Shutdown();

        GLFWwindow* m_Window;

        struct WindowData {
            std::string Title;
            uint32_t Width, Height;
            bool VSync;

            EventCallbackFn EventCallback;
        };

        WindowData m_Data;
    };

}
