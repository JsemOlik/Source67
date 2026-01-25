#pragma once

#include "VertexArray.h"

namespace S67 {

    class Renderer {
    public:
        static void Init();
        static void OnWindowResize(uint32_t width, uint32_t height);

        static void BeginScene();
        static void EndScene();

        static void Submit(const Ref<VertexArray>& vertexArray);
    };

}
