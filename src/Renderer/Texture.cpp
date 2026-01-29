#include "Texture.h"
#include "stb_image.h"
#include <glad/glad.h>
#include "Core/Logger.h"

namespace S67 {

    class OpenGLTexture2D : public Texture2D {
    public:
        OpenGLTexture2D(const std::string& path)
            : m_Path(path) {
            int width, height, channels;
            stbi_set_flip_vertically_on_load(1);
            stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
            S67_CORE_ASSERT(data, "Failed to load image!");

            m_Width = width;
            m_Height = height;

            GLenum internalFormat = GL_RGBA8;
            GLenum dataFormat = GL_RGBA;

            glGenTextures(1, &m_RendererID);
            glBindTexture(GL_TEXTURE_2D, m_RendererID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(data);
        }

        virtual ~OpenGLTexture2D() {
            if (m_RendererID != 0) {
                glDeleteTextures(1, &m_RendererID);
            }
        }

        // Delete copy operations
        OpenGLTexture2D(const OpenGLTexture2D&) = delete;
        OpenGLTexture2D& operator=(const OpenGLTexture2D&) = delete;

        // Implement move operations
        OpenGLTexture2D(OpenGLTexture2D&& other) noexcept
            : m_Path(std::move(other.m_Path)),
              m_Width(other.m_Width),
              m_Height(other.m_Height),
              m_RendererID(other.m_RendererID) {
            other.m_RendererID = 0;
        }

        OpenGLTexture2D& operator=(OpenGLTexture2D&& other) noexcept {
            if (this != &other) {
                // Clean up existing resource
                if (m_RendererID != 0) {
                    glDeleteTextures(1, &m_RendererID);
                }
                
                // Move data
                m_Path = std::move(other.m_Path);
                m_Width = other.m_Width;
                m_Height = other.m_Height;
                m_RendererID = other.m_RendererID;
                
                // Nullify moved-from object
                other.m_RendererID = 0;
            }
            return *this;
        }

        virtual uint32_t GetWidth() const override { return m_Width; }
        virtual uint32_t GetHeight() const override { return m_Height; }
        virtual uint32_t GetRendererID() const override { return m_RendererID; }
        virtual const std::string& GetPath() const override { return m_Path; }

        virtual void Bind(uint32_t slot = 0) const override {
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_2D, m_RendererID);
        }

    private:
        std::string m_Path;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_RendererID = 0;
    };

    Ref<Texture2D> Texture2D::Create(const std::string& path) {
        return CreateRef<OpenGLTexture2D>(path);
    }

}
