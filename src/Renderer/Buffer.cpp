#include "Buffer.h"
#include <glad/glad.h>

namespace S67 {

    // --- VertexBuffer -------------------------------------------------------

    class OpenGLVertexBuffer : public VertexBuffer {
    public:
        OpenGLVertexBuffer(float* vertices, uint32_t size) {
            glGenBuffers(1, &m_RendererID);
            glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
            glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
        }

        virtual ~OpenGLVertexBuffer() {
            if (m_RendererID != 0) {
                glDeleteBuffers(1, &m_RendererID);
            }
        }

        // Delete copy operations
        OpenGLVertexBuffer(const OpenGLVertexBuffer&) = delete;
        OpenGLVertexBuffer& operator=(const OpenGLVertexBuffer&) = delete;

        // Implement move operations
        OpenGLVertexBuffer(OpenGLVertexBuffer&& other) noexcept
            : m_RendererID(other.m_RendererID),
              m_Layout(std::move(other.m_Layout)) {
            other.m_RendererID = 0;
        }

        OpenGLVertexBuffer& operator=(OpenGLVertexBuffer&& other) noexcept {
            if (this != &other) {
                // Clean up existing resource
                if (m_RendererID != 0) {
                    glDeleteBuffers(1, &m_RendererID);
                }
                
                // Move data
                m_RendererID = other.m_RendererID;
                m_Layout = std::move(other.m_Layout);
                
                // Nullify moved-from object
                other.m_RendererID = 0;
            }
            return *this;
        }

        virtual void Bind() const override {
            glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        }

        virtual void Unbind() const override {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
        virtual const BufferLayout& GetLayout() const override { return m_Layout; }

    private:
        uint32_t m_RendererID = 0;
        BufferLayout m_Layout;
    };

    Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size) {
        return CreateRef<OpenGLVertexBuffer>(vertices, size);
    }

    // --- IndexBuffer --------------------------------------------------------

    class OpenGLIndexBuffer : public IndexBuffer {
    public:
        OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
            : m_Count(count) {
            glGenBuffers(1, &m_RendererID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
        }

        virtual ~OpenGLIndexBuffer() {
            if (m_RendererID != 0) {
                glDeleteBuffers(1, &m_RendererID);
            }
        }

        // Delete copy operations
        OpenGLIndexBuffer(const OpenGLIndexBuffer&) = delete;
        OpenGLIndexBuffer& operator=(const OpenGLIndexBuffer&) = delete;

        // Implement move operations
        OpenGLIndexBuffer(OpenGLIndexBuffer&& other) noexcept
            : m_RendererID(other.m_RendererID),
              m_Count(other.m_Count) {
            other.m_RendererID = 0;
            other.m_Count = 0;
        }

        OpenGLIndexBuffer& operator=(OpenGLIndexBuffer&& other) noexcept {
            if (this != &other) {
                // Clean up existing resource
                if (m_RendererID != 0) {
                    glDeleteBuffers(1, &m_RendererID);
                }
                
                // Move data
                m_RendererID = other.m_RendererID;
                m_Count = other.m_Count;
                
                // Nullify moved-from object
                other.m_RendererID = 0;
                other.m_Count = 0;
            }
            return *this;
        }

        virtual void Bind() const override {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
        }

        virtual void Unbind() const override {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        virtual uint32_t GetCount() const override { return m_Count; }

    private:
        uint32_t m_RendererID = 0;
        uint32_t m_Count = 0;
    };

    Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count) {
        return CreateRef<OpenGLIndexBuffer>(indices, count);
    }

}
