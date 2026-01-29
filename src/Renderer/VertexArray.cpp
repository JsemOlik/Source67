#include "VertexArray.h"
#include <glad/glad.h>

namespace S67 {

    static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {
        switch (type) {
            case ShaderDataType::Float:    return GL_FLOAT;
            case ShaderDataType::Float2:   return GL_FLOAT;
            case ShaderDataType::Float3:   return GL_FLOAT;
            case ShaderDataType::Float4:   return GL_FLOAT;
            case ShaderDataType::Mat3:     return GL_FLOAT;
            case ShaderDataType::Mat4:     return GL_FLOAT;
            case ShaderDataType::Int:      return GL_INT;
            case ShaderDataType::Int2:     return GL_INT;
            case ShaderDataType::Int3:     return GL_INT;
            case ShaderDataType::Int4:     return GL_INT;
            case ShaderDataType::Bool:     return GL_BOOL;
            default:                       break;
        }
        S67_CORE_ASSERT(false, "Unknown ShaderDataType!");
        return 0;
    }

    class OpenGLVertexArray : public VertexArray {
    public:
        OpenGLVertexArray() {
            glGenVertexArrays(1, &m_RendererID);
        }

        virtual ~OpenGLVertexArray() {
            if (m_RendererID != 0) {
                glDeleteVertexArrays(1, &m_RendererID);
            }
        }

        // Delete copy operations
        OpenGLVertexArray(const OpenGLVertexArray&) = delete;
        OpenGLVertexArray& operator=(const OpenGLVertexArray&) = delete;

        // Implement move operations
        OpenGLVertexArray(OpenGLVertexArray&& other) noexcept
            : m_RendererID(other.m_RendererID),
              m_VertexBuffers(std::move(other.m_VertexBuffers)),
              m_IndexBuffer(std::move(other.m_IndexBuffer)) {
            other.m_RendererID = 0;
        }

        OpenGLVertexArray& operator=(OpenGLVertexArray&& other) noexcept {
            if (this != &other) {
                // Clean up existing resource
                if (m_RendererID != 0) {
                    glDeleteVertexArrays(1, &m_RendererID);
                }
                
                // Move data
                m_RendererID = other.m_RendererID;
                m_VertexBuffers = std::move(other.m_VertexBuffers);
                m_IndexBuffer = std::move(other.m_IndexBuffer);
                
                // Nullify moved-from object
                other.m_RendererID = 0;
            }
            return *this;
        }

        virtual void Bind() const override {
            glBindVertexArray(m_RendererID);
        }

        virtual void Unbind() const override {
            glBindVertexArray(0);
        }

        virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override {
            S67_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

            glBindVertexArray(m_RendererID);
            vertexBuffer->Bind();

            uint32_t index = 0;
            const auto& layout = vertexBuffer->GetLayout();
            for (const auto& element : layout) {
                glEnableVertexAttribArray(index);
                glVertexAttribPointer(index,
                                      element.GetComponentCount(),
                                      ShaderDataTypeToOpenGLBaseType(element.Type),
                                      element.Normalized ? GL_TRUE : GL_FALSE,
                                      layout.GetStride(),
                                      (const void*)element.Offset);
                index++;
            }

            m_VertexBuffers.push_back(vertexBuffer);
        }

        virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override {
            glBindVertexArray(m_RendererID);
            indexBuffer->Bind();

            m_IndexBuffer = indexBuffer;
        }

        virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
        virtual const Ref<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }

    private:
        uint32_t m_RendererID = 0;
        std::vector<Ref<VertexBuffer>> m_VertexBuffers;
        Ref<IndexBuffer> m_IndexBuffer;
    };

    Ref<VertexArray> VertexArray::Create() {
        return CreateRef<OpenGLVertexArray>();
    }

}
