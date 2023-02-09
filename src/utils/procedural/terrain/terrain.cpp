#include "terrain.hpp"

namespace std {
    template<>
    struct hash<Engine::Model::Vertex> {
        size_t operator()(Engine::Model::Vertex const &vertex) const {
            size_t seed = 0;
            hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.texCoord);
            return seed;
        }
    };
}

namespace Engine::Procedural {
    Terrain::Terrain(Device &device, uint32_t resolution) : device(device), resolution(resolution) {
        vertices.clear();
        indices.clear();

        std::unordered_map<Model::Vertex, uint32_t> uniqueVertices{};
        for (uint32_t x = 0; x <= resolution - 1; x++) {
            for (uint32_t z = 0; z <= resolution - 1; z++) {
                Model::Vertex vertex1{};
                vertex1.position = {x, 0.0f, z};

                Model::Vertex vertex2{};
                vertex2.position = {x + 1, 0.0f, z};

                Model::Vertex vertex3{};
                vertex3.position = {x, 0.0f, z + 1};

                Model::Vertex vertex4{};
                vertex4.position = {x + 1, 0.0f, z + 1};

                if (uniqueVertices.count(vertex1) == 0) {
                    uniqueVertices[vertex1] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex1);
                }

                if (uniqueVertices.count(vertex2) == 0) {
                    uniqueVertices[vertex2] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex2);
                }

                if (uniqueVertices.count(vertex3) == 0) {
                    uniqueVertices[vertex3] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex3);
                }

                if (uniqueVertices.count(vertex4) == 0) {
                    uniqueVertices[vertex4] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex4);
                }

                indices.push_back(uniqueVertices[vertex1]);
                indices.push_back(uniqueVertices[vertex2]);
                indices.push_back(uniqueVertices[vertex3]);
                indices.push_back(uniqueVertices[vertex3]);
                indices.push_back(uniqueVertices[vertex2]);
                indices.push_back(uniqueVertices[vertex4]);
            }
        }
    }

    std::unique_ptr<Model> Terrain::getModel() {
        Model::Builder builder{vertices, indices};
        return std::make_unique<Model>(device, builder);
    }
}