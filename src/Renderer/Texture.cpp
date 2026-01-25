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

            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(data);
        }

        virtual ~OpenGLTexture2D() {
            glDeleteTextures(1, &m_RendererID);
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
        uint32_t m_Width, m_Height;
        uint32_t m_RendererID;
    };

    Ref<Texture2D> Texture2D::Create(const std::string& path) {
        return CreateRef<OpenGLTexture2D>(path);
    }

}
