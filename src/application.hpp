#ifndef APPLICATION_H
#define APPLICATION_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <memory>
#include <chrono>
#include <vector>
#include <array>

// Misc utils
#include "utils/window/window.hpp"
#include "utils/device/device.hpp"
#include "utils/entity/entity.hpp"
#include "utils/renderer/renderer.hpp"
#include "utils/input/keyboard_movement_controller/keyboardmovementcontroller.hpp"
#include "utils/descriptors/descriptors.hpp"
#include "utils/texture/texture.hpp"

// Procedural geometry
#include "utils/procedural/quad/quad.hpp"
#include "utils/procedural/cube/cube.hpp"

// Render systems
#include "rendersystems/simple/simplerendersystem.hpp"
#include "rendersystems/billboard/billboardrendersystem.hpp"

namespace Engine {
    class Application {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        static constexpr float FOV = glm::radians(60.0f);

        static constexpr float NEAR_PLANE = 0.1f;
        static constexpr float FAR_PLANE = 100.0f;

        Application();
        ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        void run();
    private:
        Window window{WIDTH, HEIGHT, "Vulkan test window"};
        Device device{window};
        Renderer renderer{window, device};
        Entity::Map entities;

        std::unique_ptr<DescriptorPool> globalPool{};

        void loadEntities();
    };
}


#endif
