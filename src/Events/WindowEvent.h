#pragma once

#include <vector>
#include "Event.h"
#include <sstream>

namespace S67 {

    class WindowResizeEvent : public Event {
    public:
        WindowResizeEvent(unsigned int width, unsigned int height)
            : m_Width(width), m_Height(height) {}

        inline unsigned int GetWidth() const { return m_Width; }
        inline unsigned int GetHeight() const { return m_Height; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
            return ss.str();
        }

        EVENT_CLASS_TYPE(WindowResize)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    private:
        unsigned int m_Width, m_Height;
    };

    class WindowCloseEvent : public Event {
    public:
        WindowCloseEvent() = default;

        EVENT_CLASS_TYPE(WindowClose)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class WindowDropEvent : public Event {
    public:
        WindowDropEvent(const std::vector<std::string>& paths)
            : m_Paths(paths) {}

        const std::vector<std::string>& GetPaths() const { return m_Paths; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "WindowDropEvent: " << m_Paths.size() << " files";
            return ss.str();
        }

        EVENT_CLASS_TYPE(WindowDrop)
        EVENT_CLASS_CATEGORY(EventCategoryApplication | EventCategoryInput)
    private:
        std::vector<std::string> m_Paths;
    };

}
