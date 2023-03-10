#include "application.hpp"

namespace Engine {
    Application::Application() {
        globalPool = DescriptorPool::Builder(device)
                     .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                     .build();
        loadEntities();
    }
    Application::~Application() {
        globalPool = nullptr; // We need globalPool to be destroyed before device is, this ensures that happens.
    }

    void Application::run() {
        std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto &uboBuffer : uboBuffers) {
            uboBuffer = std::make_unique<Buffer>(
                    device,
                    sizeof(GlobalUbo),
                    1,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            uboBuffer->map();
        }

        auto globalSetLayout = DescriptorSetLayout::Builder(device)
                .addBinding(0,
                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                .addBinding(1,
                            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            VK_SHADER_STAGE_FRAGMENT_BIT).build();

        Texture texture{device, "../res/textures/texture.jpg"};

        std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (unsigned int i = 0; i < globalDescriptorSets.size(); i++) {
            VkDescriptorBufferInfo bufferInfo = uboBuffers[i]->descriptorInfo();
            VkDescriptorImageInfo imageInfo = texture.getDescriptorImageInfo();
            DescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &imageInfo)
                .build(globalDescriptorSets[i]);
        }

        SimpleRenderSystem simpleRenderSystem{device,
                                              renderer.getSwapChainRenderPass(),
                                              globalSetLayout->getDescriptorSetLayout()};
        BillboardRenderSystem billboardRenderSystem{device,
                                                    renderer.getSwapChainRenderPass(),
                                                    globalSetLayout->getDescriptorSetLayout()};
        Camera camera{};
        camera.setViewTarget(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.5f, 0.0f, 1.0f});

        auto cameraEntity = Entity::createEntity();
        cameraEntity.addComponent(std::make_unique<TransformComponent>(glm::vec3{0.0f, 0.0f, -2.5f}));
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();
        while (!window.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            deltaTime = glm::min(deltaTime, FrameInfo::MAX_DELTA_TIME);

            cameraController.moveInPlaneXZ(window.getWindow(), deltaTime, cameraEntity);
            camera.setViewXYZ(cameraEntity.getTransformComponent()->position,
                              cameraEntity.getTransformComponent()->rotation);

            float aspectRatio = renderer.getAspectRatio();
            // camera.setOrthographicProjection(aspectRatio, -1.0f, -1.0f, 1.0f);
            // camera.setOrthographicProjection(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
            camera.setPerspectiveProjection(FOV, aspectRatio, NEAR_PLANE, FAR_PLANE);

            if (auto commandBuffer = renderer.beginFrame()) {
                uint32_t frameIndex = renderer.getCurrentFrameIndex();
                FrameInfo frameInfo{frameIndex,
                                    deltaTime,
                                    commandBuffer,
                                    camera,
                                    globalDescriptorSets[frameIndex],
                                    entities};

                // Update cycle
                GlobalUbo ubo{};
                ubo.projectionMatrix = frameInfo.camera.getProjectionMatrix();
                ubo.viewMatrix = frameInfo.camera.getViewMatrix();
                ubo.inverseViewMatrix = camera.getInverseViewMatrix();
                billboardRenderSystem.update(frameInfo, ubo);
                uboBuffers[frameInfo.frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameInfo.frameIndex]->flush();

                // Render cycle
                renderer.beginSwapChainRenderPass(frameInfo.commandBuffer);
                simpleRenderSystem.renderGameObjects(frameInfo);
                billboardRenderSystem.render(frameInfo);
                renderer.endSwapChainRenderPass(frameInfo.commandBuffer);
                renderer.endFrame();
            }
        } vkDeviceWaitIdle(device.device()); // Wait for all the resource to be freed before destroying them
    }

    void Application::loadEntities() {
        entities.reserve(5);

        // Flat shaded sphere (left)
        std::shared_ptr<Model> sphereFlatModel = Model::createModelFromFile(device, "../res/models/sphere/sphere_flat.obj");
        Entity sphereFlat = Entity::createEntity();
        sphereFlat.addComponent(std::make_unique<ModelComponent>(sphereFlatModel));
        sphereFlat.addComponent(std::make_unique<TransformComponent>(glm::vec3{2.5f, 0.0f, 5.0f},
                                                                       glm::vec3{0.5f, 0.5f, 0.5f}));
        entities.emplace(sphereFlat.getId(), std::move(sphereFlat));

        // Smooth shaded sphere (right)
        std::shared_ptr<Model> sphereSmoothModel = Model::createModelFromFile(device, "../res/models/sphere/sphere_smooth.obj");
        Entity sphereSmooth = Entity::createEntity();
        sphereSmooth.addComponent(std::make_unique<ModelComponent>(sphereSmoothModel));
        sphereSmooth.addComponent(std::make_unique<TransformComponent>(glm::vec3{-2.5f, 0.0f, 5.0f},
                                                                       glm::vec3{0.5f, 0.5f, 0.5f}));
        entities.emplace(sphereSmooth.getId(), std::move(sphereSmooth));

        // Procedural quad (center)
        Procedural::Quad q(device, 128);
        q.generateModel();
        std::shared_ptr<Model> quadModel = q.getModel();
        Entity quad = Entity::createEntity();
        quad.addComponent(std::make_unique<ModelComponent>(quadModel));
        quad.addComponent(std::make_unique<TransformComponent>(glm::vec3{-2.5f, 0.0f, 5.0f},
                                                                glm::vec3{5.0f, 5.0f, 5.0f}));
        entities.emplace(quad.getId(), std::move(quad));

        // Procedural cube (center up)
        Procedural::Cube c(device, 128);
        c.generateModel();
        std::shared_ptr<Model> cubeModel = c.getModel();
        Entity cube = Entity::createEntity();
        cube.addComponent(std::make_unique<ModelComponent>(cubeModel));
        cube.addComponent(std::make_unique<TransformComponent>(glm::vec3{-0.5f, -2.0f, 5.0f}));
        entities.emplace(cube.getId(), std::move(cube));

        // Point light
        Entity pointLight = Entity::createPointLightEntity();
        pointLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
        pointLight.getTransformComponent()->position = glm::vec3(0.0f, -3.0f, 3.0f);
        entities.emplace(pointLight.getId(), std::move(pointLight));
    }
}