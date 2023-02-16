#ifndef PROCEDURAL_MESH_HPP
#define PROCEDURAL_MESH_HPP

#include <cstdint>

#include "../device/device.hpp"
#include "../model/model.hpp"

namespace Engine::Procedural {
    class ProceduralMesh {
    public:
        ProceduralMesh(Device &device, uint32_t resolution) : device(device), resolution(resolution) {
            // Clear the vectors to avoid reallocations.
            vertices.clear();
            indices.clear();
        }
        ~ProceduralMesh() = default;

        ProceduralMesh(const ProceduralMesh&) = delete;
        ProceduralMesh& operator=(const ProceduralMesh&) = delete;

        virtual void generateModel() = 0;

        std::unique_ptr<Model> getModel() { return std::make_unique<Model>(device, builder); }
    protected:
        Device &device;

        uint32_t resolution;

        std::vector<Model::Vertex> vertices{};
        std::vector<uint32_t> indices{};

        Model::Builder builder;
    };
}

#endif
