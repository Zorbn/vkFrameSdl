#include "../vkFrame/renderer.hpp"

/*
 * 2d:
 * Render 2d sprites.
 */

struct VertexData {
    alignas(16) glm::vec3 pos;
    alignas(16) glm::vec3 color;
    alignas(8) glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(VertexData);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(VertexData, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(VertexData, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(VertexData, texCoord);

        return attributeDescriptions;
    }
};

struct InstanceData {
public:
    alignas(16) glm::vec3 pos;
    alignas(8) glm::vec2 size;
    float texIndex;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 1;
        bindingDescription.stride = sizeof(InstanceData);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        attributeDescriptions.resize(3);

        attributeDescriptions[0].binding = 1;
        attributeDescriptions[0].location = 3;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(InstanceData, pos);

        attributeDescriptions[1].binding = 1;
        attributeDescriptions[1].location = 4;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(InstanceData, size);

        attributeDescriptions[2].binding = 1;
        attributeDescriptions[2].location = 5;
        attributeDescriptions[2].format = VK_FORMAT_R32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(InstanceData, texIndex);

        return attributeDescriptions;
    }
};

struct UniformBufferData {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

const std::vector<VertexData> spriteVertices = {
    {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
    {{1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
    {{0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
};

const std::vector<uint16_t> spriteIndices = {0, 2, 1, 0, 3, 2};

class App {
private:
    Pipeline pipeline;
    RenderPass renderPass;

    Image textureImage;
    VkImageView textureImageView;
    VkSampler textureSampler;

    UniformBuffer<UniformBufferData> ubo;
    Model<VertexData, uint16_t, InstanceData> spriteModel;

    std::vector<VkClearValue> clearValues;

public:
    void init(VulkanState& vulkanState, SDL_Window* window, int32_t width, int32_t height) {
        vulkanState.swapchain.create(vulkanState.device, vulkanState.physicalDevice,
                                     vulkanState.surface, width, height);

        vulkanState.commands.createPool(vulkanState.physicalDevice, vulkanState.device,
                                        vulkanState.surface);
        vulkanState.commands.createBuffers(vulkanState.device, vulkanState.maxFramesInFlight);

        textureImage = Image::createTextureArray("res/cubesImg.png", vulkanState.allocator,
                                                 vulkanState.commands, vulkanState.graphicsQueue,
                                                 vulkanState.device, false, 16, 16, 4);
        textureImageView = textureImage.createTextureView(vulkanState.device);
        textureSampler = textureImage.createTextureSampler(
            vulkanState.physicalDevice, vulkanState.device, VK_FILTER_NEAREST, VK_FILTER_NEAREST);

        spriteModel = Model<VertexData, uint16_t, InstanceData>::fromVerticesAndIndices(
            spriteVertices, spriteIndices, 3, vulkanState.allocator, vulkanState.commands,
            vulkanState.graphicsQueue, vulkanState.device);
        std::vector<InstanceData> instances = {
            InstanceData{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(32.0f, 32.0f), 0.0},
            InstanceData{glm::vec3(32.0f, 0.0f, 0.0f), glm::vec2(32.0f, 32.0f), 1.0},
            InstanceData{glm::vec3(32.0f, 32.0f, 0.0f), glm::vec2(32.0f, 32.0f), 2.0}};
        spriteModel.updateInstances(instances, vulkanState.commands, vulkanState.allocator,
                                    vulkanState.graphicsQueue, vulkanState.device);

        ubo.create(vulkanState.maxFramesInFlight, vulkanState.allocator);

        renderPass.create(vulkanState.physicalDevice, vulkanState.device, vulkanState.allocator,
                          vulkanState.swapchain, true, true);

        pipeline.createDescriptorSetLayout(
            vulkanState.device, [&](std::vector<VkDescriptorSetLayoutBinding>& bindings) {
                VkDescriptorSetLayoutBinding uboLayoutBinding{};
                uboLayoutBinding.binding = 0;
                uboLayoutBinding.descriptorCount = 1;
                uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                uboLayoutBinding.pImmutableSamplers = nullptr;
                uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                samplerLayoutBinding.binding = 1;
                samplerLayoutBinding.descriptorCount = 1;
                samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                samplerLayoutBinding.pImmutableSamplers = nullptr;
                samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                bindings.push_back(uboLayoutBinding);
                bindings.push_back(samplerLayoutBinding);
            });
        pipeline.createDescriptorPool(
            vulkanState.maxFramesInFlight, vulkanState.device,
            [&](std::vector<VkDescriptorPoolSize> poolSizes) {
                poolSizes.resize(2);
                poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                poolSizes[0].descriptorCount = static_cast<uint32_t>(vulkanState.maxFramesInFlight);
                poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                poolSizes[1].descriptorCount = static_cast<uint32_t>(vulkanState.maxFramesInFlight);
            });
        pipeline.createDescriptorSets(
            vulkanState.maxFramesInFlight, vulkanState.device,
            [&](std::vector<VkWriteDescriptorSet>& descriptorWrites, VkDescriptorSet descriptorSet,
                uint32_t i) {
                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = ubo.getBuffer(i);
                bufferInfo.offset = 0;
                bufferInfo.range = ubo.getDataSize();

                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = textureImageView;
                imageInfo.sampler = textureSampler;

                descriptorWrites.resize(2);

                descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[0].dstSet = descriptorSet;
                descriptorWrites[0].dstBinding = 0;
                descriptorWrites[0].dstArrayElement = 0;
                descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrites[0].descriptorCount = 1;
                descriptorWrites[0].pBufferInfo = &bufferInfo;

                descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[1].dstSet = descriptorSet;
                descriptorWrites[1].dstBinding = 1;
                descriptorWrites[1].dstArrayElement = 0;
                descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[1].descriptorCount = 1;
                descriptorWrites[1].pImageInfo = &imageInfo;

                vkUpdateDescriptorSets(vulkanState.device,
                                       static_cast<uint32_t>(descriptorWrites.size()),
                                       descriptorWrites.data(), 0, nullptr);
            });
        pipeline.create<VertexData, InstanceData>("res/2dShader.vert.spv", "res/2dShader.frag.spv",
                                                  vulkanState.device, renderPass, false);

        clearValues.resize(2);
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};
    }

    void update(VulkanState& vulkanState) {}

    void render(VulkanState& vulkanState, VkCommandBuffer commandBuffer, uint32_t imageIndex,
                uint32_t currentFrame) {
        const VkExtent2D& extent = vulkanState.swapchain.getExtent();

        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time =
            std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime)
                .count();

        UniformBufferData uboData{};
        uboData.model = glm::mat4(1.0f);
        uboData.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                   glm::vec3(0.0f, 1.0f, 0.0f));
        uboData.proj = glm::ortho(0.0f, static_cast<float>(extent.width), 0.0f,
                                  static_cast<float>(extent.height), 0.1f, 10.0f);

        ubo.update(uboData);

        vulkanState.commands.beginBuffer(currentFrame);

        renderPass.begin(imageIndex, commandBuffer, extent, clearValues);
        pipeline.bind(commandBuffer, currentFrame);

        spriteModel.draw(commandBuffer);

        renderPass.end(commandBuffer);

        vulkanState.commands.endBuffer(currentFrame);
    }

    void resize(VulkanState& vulkanState, int32_t width, int32_t height) {
        renderPass.recreate(vulkanState.physicalDevice, vulkanState.device, vulkanState.allocator,
                            vulkanState.swapchain);
    }

    void cleanup(VulkanState& vulkanState) {
        pipeline.cleanup(vulkanState.device);
        renderPass.cleanup(vulkanState.allocator, vulkanState.device);

        ubo.destroy(vulkanState.allocator);

        vkDestroySampler(vulkanState.device, textureSampler, nullptr);
        vkDestroyImageView(vulkanState.device, textureImageView, nullptr);
        textureImage.destroy(vulkanState.allocator);

        spriteModel.destroy(vulkanState.allocator);
    }

    int run() {
        Renderer renderer;

        std::function<void(VulkanState&, SDL_Window*, int32_t, int32_t)> initCallback =
            [&](VulkanState& vulkanState, SDL_Window* window, int32_t width, int32_t height) {
                this->init(vulkanState, window, width, height);
            };

        std::function<void(VulkanState&)> updateCallback = [&](VulkanState vulkanState) {
            this->update(vulkanState);
        };

        std::function<void(VulkanState&, VkCommandBuffer, uint32_t, uint32_t)> renderCallback =
            [&](VulkanState& vulkanState, VkCommandBuffer commandBuffer, uint32_t imageIndex,
                uint32_t currentFrame) {
                this->render(vulkanState, commandBuffer, imageIndex, currentFrame);
            };

        std::function<void(VulkanState&, int32_t, int32_t)> resizeCallback =
            [&](VulkanState& vulkanState, int32_t width, int32_t height) {
                this->resize(vulkanState, width, height);
            };

        std::function<void(VulkanState&)> cleanupCallback = [&](VulkanState& vulkanState) {
            this->cleanup(vulkanState);
        };

        try {
            renderer.run("2d", 640, 480, 2, initCallback, updateCallback, renderCallback,
                         resizeCallback, cleanupCallback);
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
};

int main() {
    App app;
    return app.run();
}