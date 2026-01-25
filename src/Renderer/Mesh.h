#pragma once

#include "VertexArray.h"
#include <string>

namespace S67 {

    class MeshLoader {
    public:
        static Ref<VertexArray> LoadOBJ(const std::string& path);
    };

}
