#include "billboardrendersystem.hpp"

namespace Engine {
    struct PointLightPushConstant {
        glm::vec4 position{};
        glm::vec4 color{};
        float radius = 0.0f;
    };

    BillboardRenderSystem::BillboardRenderSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : device(device) {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }
    BillboardRenderSystem::~BillboardRenderSystem() {
        vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
    }

    void BillboardRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        // This is for push constants
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PointLightPushConstant);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create pipeline layout!");
    }
    void BillboardRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipeline = std::make_unique<Pipeline>(device,
                                              "../res/shaders/compiled/billboard.vert.spv",
                                              "../res/shaders/compiled/billboard.frag.spv",
                                              pipelineConfig);
    }
    void BillboardRenderSystem::update(FrameInfo &frameInfo, GlobalUbo &ubo) {
        int index = 0;
        for (auto &kv : frameInfo.entities) {
            auto &ent = kv.second;
            if(!ent.hasComponent(ComponentType::POINT_LIGHT)) continue;

            ubo.pointLights[index].position = glm::vec4(ent.getTransformComponent()->position, 1.0f);
            ubo.pointLights[index].color = glm::vec4(ent.color, ent.getPointLightComponent()->intensity);
            index++;
        } ubo.pointLightCount = index;
    }
    void BillboardRenderSystem::render(FrameInfo &frameInfo) {
        pipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelineLayout,
                                0,
                                1,
                                &frameInfo.globalDescriptorSet,
                                0,
                                nullptr);
        for (auto &kv : frameInfo.entities) {
            auto &ent = kv.second;
            if(!ent.hasComponent(ComponentType::POINT_LIGHT)) continue;

            PointLightPushConstant push{};
            push.position = glm::vec4(ent.getTransformComponent()->position, 1.0f);
            push.color = glm::vec4(ent.color, ent.getPointLightComponent()->intensity);
            push.radius = ent.getTransformComponent()->scale.x;

            vkCmdPushConstants(frameInfo.commandBuffer,
                               pipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               0,
                               sizeof(PointLightPushConstant),
                               &push);
            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        }
    }
}