#include "Renderer/VertexArray.h"
#include "Renderer/Camera.h"
#include "Renderer/Shader.h"
#include "Renderer/Scene.h"
#include "Renderer/Light.h"

namespace S67 {

    class Renderer {
    public:
        static void Init();
        static void OnWindowResize(uint32_t width, uint32_t height);

        static void BeginScene(const Camera& camera, const DirectionalLight& dirLight);
        static void EndScene();

        static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

    private:
        struct SceneData {
            glm::mat4 ViewProjectionMatrix;
            DirectionalLight DirLight;
        };

        static SceneData* s_SceneData;
    };

}
