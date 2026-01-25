#define TINYOBJLOADER_IMPLEMENTATION
#include "Mesh.h"
#include "tinyobjloader/tiny_obj_loader.h"
#include "Core/Logger.h"
#include <glm/glm.hpp>
#include <unordered_map>

namespace S67 {

    struct OBJVertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoord;

        bool operator==(const OBJVertex& other) const {
            return Position == other.Position && Normal == other.Normal && TexCoord == other.TexCoord;
        }
    };
}

namespace std {
    template<> struct hash<S67::OBJVertex> {
        size_t operator()(const S67::OBJVertex& vertex) const {
            return ((hash<float>()(vertex.Position.x) ^ (hash<float>()(vertex.Position.y) << 1)) >> 1) ^
                   (hash<float>()(vertex.Position.z) << 1) ^
                   (hash<float>()(vertex.Normal.x) << 1) ^
                   (hash<float>()(vertex.Normal.y) << 1) ^
                   (hash<float>()(vertex.Normal.z) << 1) ^
                   (hash<float>()(vertex.TexCoord.x) << 1) ^
                   (hash<float>()(vertex.TexCoord.y) << 1);
        }
    };
}

namespace S67 {

    Ref<VertexArray> MeshLoader::LoadOBJ(const std::string& path) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
            S67_CORE_ERROR("Failed to load OBJ: {0}", err);
            return nullptr;
        }

        std::vector<OBJVertex> vertices;
        std::vector<uint32_t> indices;
        std::unordered_map<OBJVertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                OBJVertex vertex{};

                vertex.Position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                if (index.normal_index >= 0) {
                    vertex.Normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                }

                if (index.texcoord_index >= 0) {
                    vertex.TexCoord = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = (uint32_t)vertices.size();
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }

        Ref<VertexArray> va = VertexArray::Create();
        Ref<VertexBuffer> vb = VertexBuffer::Create((float*)vertices.data(), (uint32_t)(vertices.size() * sizeof(OBJVertex)));
        vb->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_Normal" },
            { ShaderDataType::Float2, "a_TexCoord" }
        });
        va->AddVertexBuffer(vb);

        Ref<IndexBuffer> ib = IndexBuffer::Create(indices.data(), (uint32_t)indices.size());
        va->SetIndexBuffer(ib);

        return va;
    }

}
