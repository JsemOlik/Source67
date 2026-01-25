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
            glDeleteBuffers(1, &m_RendererID);
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
        uint32_t m_RendererID;
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
            glDeleteBuffers(1, &m_RendererID);
        }

        virtual void Bind() const override {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
        }

        virtual void Unbind() const override {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        virtual uint32_t GetCount() const override { return m_Count; }

    private:
        uint32_t m_RendererID;
        uint32_t m_Count;
    };

    Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count) {
        return CreateRef<OpenGLIndexBuffer>(indices, count);
    }

}
