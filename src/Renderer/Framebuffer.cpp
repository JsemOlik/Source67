#include "Framebuffer.h"
#include <glad/glad.h>
#include "Core/Logger.h"

namespace S67 {

    static const uint32_t s_MaxFramebufferSize = 8192;

    class OpenGLFramebuffer : public Framebuffer {
    public:
        OpenGLFramebuffer(const FramebufferSpecification& spec)
            : m_Specification(spec) {
            Invalidate();
        }

        virtual ~OpenGLFramebuffer() {
            if (m_RendererID != 0) {
                glDeleteFramebuffers(1, &m_RendererID);
            }
            if (m_ColorAttachment != 0) {
                glDeleteTextures(1, &m_ColorAttachment);
            }
            if (m_DepthAttachment != 0) {
                glDeleteTextures(1, &m_DepthAttachment);
            }
        }

        // Delete copy operations
        OpenGLFramebuffer(const OpenGLFramebuffer&) = delete;
        OpenGLFramebuffer& operator=(const OpenGLFramebuffer&) = delete;

        // Implement move operations
        OpenGLFramebuffer(OpenGLFramebuffer&& other) noexcept
            : m_RendererID(other.m_RendererID),
              m_ColorAttachment(other.m_ColorAttachment),
              m_DepthAttachment(other.m_DepthAttachment),
              m_Specification(other.m_Specification) {
            other.m_RendererID = 0;
            other.m_ColorAttachment = 0;
            other.m_DepthAttachment = 0;
        }

        OpenGLFramebuffer& operator=(OpenGLFramebuffer&& other) noexcept {
            if (this != &other) {
                // Clean up existing resources
                if (m_RendererID != 0) {
                    glDeleteFramebuffers(1, &m_RendererID);
                }
                if (m_ColorAttachment != 0) {
                    glDeleteTextures(1, &m_ColorAttachment);
                }
                if (m_DepthAttachment != 0) {
                    glDeleteTextures(1, &m_DepthAttachment);
                }
                
                // Move data
                m_RendererID = other.m_RendererID;
                m_ColorAttachment = other.m_ColorAttachment;
                m_DepthAttachment = other.m_DepthAttachment;
                m_Specification = other.m_Specification;
                
                // Nullify moved-from object
                other.m_RendererID = 0;
                other.m_ColorAttachment = 0;
                other.m_DepthAttachment = 0;
            }
            return *this;
        }

        void Invalidate() {
            if (m_RendererID) {
                glDeleteFramebuffers(1, &m_RendererID);
                glDeleteTextures(1, &m_ColorAttachment);
                glDeleteTextures(1, &m_DepthAttachment);
            }

            glGenFramebuffers(1, &m_RendererID);
            glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

            glGenTextures(1, &m_ColorAttachment);
            glBindTexture(GL_TEXTURE_2D, m_ColorAttachment);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Specification.Width, m_Specification.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0);

            glGenTextures(1, &m_DepthAttachment);
            glBindTexture(GL_TEXTURE_2D, m_DepthAttachment);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0);

            S67_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        virtual void Bind() override {
            glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
            glViewport(0, 0, m_Specification.Width, m_Specification.Height);
        }

        virtual void Unbind() override {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        virtual void Resize(uint32_t width, uint32_t height) override {
            if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize) {
                S67_CORE_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
                return;
            }
            m_Specification.Width = width;
            m_Specification.Height = height;

            Invalidate();
        }

        virtual uint32_t GetColorAttachmentRendererID() const override { return m_ColorAttachment; }

        virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

    private:
        uint32_t m_RendererID = 0;
        uint32_t m_ColorAttachment = 0, m_DepthAttachment = 0;
        FramebufferSpecification m_Specification;
    };

    Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec) {
        return CreateRef<OpenGLFramebuffer>(spec);
    }

}
