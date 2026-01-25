#define TINYOBJLOADER_IMPLEMENTATION
#include "Mesh.h"
#include "tinyobjloader/tiny_obj_loader.h"
#include "Core/Logger.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <fstream>

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
        int uvCount = 0;

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                OBJVertex vertex{};
                vertex.Position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                // Defaults
                vertex.Normal = { 0.0f, 1.0f, 0.0f };
                vertex.TexCoord = { 0.0f, 0.0f };

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
                    uvCount++;
                } else {
                    // Auto-UV projection (Planar XY)
                    vertex.TexCoord = { vertex.Position.x, vertex.Position.y };
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = (uint32_t)vertices.size();
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }

        S67_CORE_INFO("Loaded OBJ: {0} ({1} vertices, {2} indices, {3} explicit UVs, Generated Auto-UVs for others)", path, vertices.size(), indices.size(), uvCount);

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

    Ref<VertexArray> MeshLoader::LoadSTL(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            S67_CORE_ERROR("Failed to open STL file: {0}", path);
            return nullptr;
        }

        // STL Binary header is 80 bytes, ignored.
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        if (fileSize < 84) {
            S67_CORE_ERROR("STL file too small: {0}", path);
            return nullptr;
        }
        file.seekg(80, std::ios::beg);

        uint32_t triangleCount;
        file.read(reinterpret_cast<char*>(&triangleCount), sizeof(uint32_t));

        std::vector<OBJVertex> vertices;
        std::vector<uint32_t> indices;
        vertices.reserve(triangleCount * 3);
        indices.reserve(triangleCount * 3);

        for (uint32_t i = 0; i < triangleCount; i++) {
            float normal[3];
            float v1[3], v2[3], v3[3];
            uint16_t attributeByteCount;

            file.read(reinterpret_cast<char*>(normal), 12);
            file.read(reinterpret_cast<char*>(v1), 12);
            file.read(reinterpret_cast<char*>(v2), 12);
            file.read(reinterpret_cast<char*>(v3), 12);
            file.read(reinterpret_cast<char*>(&attributeByteCount), 2);

            glm::vec3 n = { normal[0], normal[1], normal[2] };
            
            // If normal is zero or invalid, compute it from vertices
            if (glm::length(n) < 0.0001f) {
                glm::vec3 edge1 = { v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2] };
                glm::vec3 edge2 = { v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2] };
                n = glm::normalize(glm::cross(edge1, edge2));
            }

            vertices.push_back({ { v1[0], v1[1], v1[2] }, n, { v1[0], v1[1] } });
            vertices.push_back({ { v2[0], v2[1], v2[2] }, n, { v2[0], v2[1] } });
            vertices.push_back({ { v3[0], v3[1], v3[2] }, n, { v3[0], v3[1] } });

            indices.push_back(i * 3 + 0);
            indices.push_back(i * 3 + 1);
            indices.push_back(i * 3 + 2);
        }

        S67_CORE_INFO("Loaded STL: {0} ({1} triangles, Generated Auto-UVs)", path, triangleCount);

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
