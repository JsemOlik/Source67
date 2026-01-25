#include <cstdint>
#include <memory>
#include "VertexArray.h"
#include "Camera.h"
#include "Shader.h"

namespace S67 {

    class Renderer {
    public:
        static void Init();
        static void OnWindowResize(uint32_t width, uint32_t height);

        static void BeginScene(const Camera& camera);
        static void EndScene();

        static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

    private:
        struct SceneData {
            glm::mat4 ViewProjectionMatrix;
        };

        static SceneData* s_SceneData;
    };

}
