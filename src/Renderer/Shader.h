#pragma once

#include "Core/Base.h"
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace S67 {

    class Shader {
    public:
        Shader(const std::string& filepath);
        Shader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
        ~Shader();

        void Bind() const;
        void Unbind() const;

        void SetInt(const std::string& name, int value);
        void SetFloat(const std::string& name, float value);
        void SetFloat3(const std::string& name, const glm::vec3& value);
        void SetFloat4(const std::string& name, const glm::vec4& value);
        void SetMat4(const std::string& name, const glm::mat4& value);

        const std::string& GetName() const { return m_Name; }
        const std::string& GetPath() const { return m_FilePath; }

        static Ref<Shader> Create(const std::string& filepath);
        static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);

    private:
        std::string ReadFile(const std::string& filepath);
        std::unordered_map<unsigned int, std::string> PreProcess(const std::string& source);
        void Compile(const std::unordered_map<unsigned int, std::string>& shaderSources);

        uint32_t m_RendererID;
        std::string m_Name;
        std::string m_FilePath;
    };

    class ShaderLibrary {
    public:
        void Add(const std::string& name, const Ref<Shader>& shader);
        void Add(const Ref<Shader>& shader);
        Ref<Shader> Load(const std::string& filepath);
        Ref<Shader> Load(const std::string& name, const std::string& filepath);

        Ref<Shader> Get(const std::string& name);

        bool Exists(const std::string& name) const;
    private:
        std::unordered_map<std::string, Ref<Shader>> m_Shaders;
    };

}
