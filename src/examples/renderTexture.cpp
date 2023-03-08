#include "../vkFrame/renderer.hpp"

/*
 * RenderTexture:
 * Uses multiple passes to draw to a model onto itself.
 */

struct VertexData {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 texCoord;

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
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(VertexData, texCoord);

        return attributeDescriptions;
    }
};

struct InstanceData {
    glm::vec3 pos;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 1;
        bindingDescription.stride = sizeof(InstanceData);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        attributeDescriptions.resize(1);

        attributeDescriptions[0].binding = 1;
        attributeDescriptions[0].location = 3;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = 0;

        return attributeDescriptions;
    }
};

struct UniformBufferData {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

const int32_t mapSize = 4;

const std::array<int32_t, mapSize* mapSize* mapSize> voxelData = {
    1, 0, 0, 0, 0, 4, 0, 0, 0, 0, 3, 0, 0, 0, 0, 2,

    0, 0, 0, 1, 0, 0, 3, 0, 0, 4, 0, 0, 2, 0, 0, 0,

    3, 2, 1, 4, 2, 0, 0, 1, 1, 0, 0, 2, 4, 1, 2, 3,

    0, 0, 0, 0, 0, 1, 2, 0, 0, 4, 3, 0, 0, 0, 0, 0,
};

const std::array<std::array<glm::vec3, 4>, 6> cubeVertices = {{
    // Forward
    {
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0),
        glm::vec3(1, 1, 0),
        glm::vec3(1, 0, 0),
    },
    // Backward
    {
        glm::vec3(0, 0, 1),
        glm::vec3(0, 1, 1),
        glm::vec3(1, 1, 1),
        glm::vec3(1, 0, 1),
    },
    // Right
    {
        glm::vec3(1, 0, 0),
        glm::vec3(1, 0, 1),
        glm::vec3(1, 1, 1),
        glm::vec3(1, 1, 0),
    },
    // Left
    {
        glm::vec3(0, 0, 0),
        glm::vec3(0, 0, 1),
        glm::vec3(0, 1, 1),
        glm::vec3(0, 1, 0),
    },
    // Up
    {
        glm::vec3(0, 1, 0),
        glm::vec3(0, 1, 1),
        glm::vec3(1, 1, 1),
        glm::vec3(1, 1, 0),
    },
    // Down
    {
        glm::vec3(0, 0, 0),
        glm::vec3(0, 0, 1),
        glm::vec3(1, 0, 1),
        glm::vec3(1, 0, 0),
    },
}};

const std::array<std::array<glm::vec2, 4>, 6> cubeUvs = {{
    // Forward
    {
        glm::vec2(1, 1),
        glm::vec2(1, 0),
        glm::vec2(0, 0),
        glm::vec2(0, 1),
    },
    // Backward
    {
        glm::vec2(0, 1),
        glm::vec2(0, 0),
        glm::vec2(1, 0),
        glm::vec2(1, 1),
    },
    // Right
    {
        glm::vec2(1, 1),
        glm::vec2(0, 1),
        glm::vec2(0, 0),
        glm::vec2(1, 0),
    },
    // Left
    {
        glm::vec2(0, 1),
        glm::vec2(1, 1),
        glm::vec2(1, 0),
        glm::vec2(0, 0),
    },
    // Up
    {
        glm::vec2(0, 1),
        glm::vec2(0, 0),
        glm::vec2(1, 0),
        glm::vec2(1, 1),
    },
    // Down
    {
        glm::vec2(0, 1),
        glm::vec2(0, 0),
        glm::vec2(1, 0),
        glm::vec2(1, 1),
    },
}};

const std::array<std::array<uint16_t, 6>, 6> cubeIndices = {{
    {0, 1, 2, 0, 2, 3}, // Forward
    {0, 2, 1, 0, 3, 2}, // Backward
    {0, 2, 1, 0, 3, 2}, // Right
    {0, 1, 2, 0, 2, 3}, // Left
    {0, 1, 2, 0, 2, 3}, // Up
    {0, 2, 1, 0, 3, 2}, // Down
}};

const std::array<std::array<int32_t, 3>, 6> directions = {{
    {0, 0, -1}, // Forward
    {0, 0, 1},  // Backward
    {1, 0, 0},  // Right
    {-1, 0, 0}, // Left
    {0, 1, 0},  // Up
    {0, -1, 0}, // Down
}};

class App {
private:
    Pipeline pipeline;
    Pipeline finalPipeline;
    RenderPass renderPass;
    RenderPass finalRenderPass;

    Image textureImage;
    VkImageView textureImageView;
    VkSampler textureSampler;

    Image colorImage;
    VkImageView colorImageView;
    VkSampler colorSampler;

    Image depthImage;
    VkImageView depthImageView;

    UniformBuffer<UniformBufferData> ubo;
    Model<VertexData, uint16_t, InstanceData> voxelModel;

    std::vector<VertexData> voxelVertices;
    std::vector<uint16_t> voxelIndices;
    std::vector<VkClearValue> clearValues;

public:
    int32_t getVoxel(size_t x, size_t y, size_t z) {
        if (x < 0 || x >= mapSize || y < 0 || y >= mapSize || z < 0 || z >= mapSize) {
            return 0;
        }

        return voxelData[x + y * mapSize + z * mapSize * mapSize];
    }

    void generateVoxelMesh() {
        for (size_t x = 0; x < mapSize; x++)
            for (size_t y = 0; y < mapSize; y++)
                for (size_t z = 0; z < mapSize; z++) {
                    int32_t voxel = getVoxel(x, y, z);

                    if (voxel == 0)
                        continue;

                    for (size_t face = 0; face < 6; face++) {
                        if (getVoxel(x + +directions[face][0], y + directions[face][1],
                                     z + directions[face][2]) != 0)
                            continue;

                        size_t vertexCount = voxelVertices.size();
                        for (uint16_t index : cubeIndices[face]) {
                            voxelIndices.push_back(index + vertexCount);
                        }

                        for (size_t i = 0; i < 4; i++) {
                            glm::vec3 vertex = cubeVertices[face][i];
                            glm::vec2 uv = cubeUvs[face][i];

                            voxelVertices.push_back(VertexData{
                                vertex + glm::vec3(x, y, z),
                                glm::vec3(1.0, 1.0, 1.0),
                                glm::vec3(uv.x, uv.y, voxel - 1),
                            });
                        }
                    }
                }
    }

    void init(VulkanState& vulkanState, SDL_Window* window, int32_t width, int32_t height) {
        vulkanState.swapchain.create(vulkanState.device, vulkanState.physicalDevice,
                                     vulkanState.surface, width, height);

        vulkanState.commands.createPool(vulkanState.physicalDevice, vulkanState.device,
                                        vulkanState.surface);
        vulkanState.commands.createBuffers(vulkanState.device, vulkanState.maxFramesInFlight);

        textureImage = Image::createTextureArray("res/cubesImg.png", vulkanState.allocator,
                                                 vulkanState.commands, vulkanState.graphicsQueue,
                                                 vulkanState.device, true, 16, 16, 4);
        textureImageView = textureImage.createTextureView(vulkanState.device);
        textureSampler = textureImage.createTextureSampler(
            vulkanState.physicalDevice, vulkanState.device, VK_FILTER_NEAREST, VK_FILTER_NEAREST);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.maxLod = 1.0f;

        if (vkCreateSampler(vulkanState.device, &samplerInfo, nullptr, &colorSampler) !=
            VK_SUCCESS) {
            throw std::runtime_error("Failed to create color sampler!");
        }

        generateVoxelMesh();
        voxelModel = Model<VertexData, uint16_t, InstanceData>::fromVerticesAndIndices(
            voxelVertices, voxelIndices, 2, vulkanState.allocator, vulkanState.commands,
            vulkanState.graphicsQueue, vulkanState.device);
        std::vector<InstanceData> instances = {InstanceData{glm::vec3(0.0, 0.0, 0.0)}, InstanceData{glm::vec3(-2.0, 0.0, -5.0)}};
        voxelModel.updateInstances(instances, vulkanState.commands, vulkanState.allocator,
                                   vulkanState.graphicsQueue, vulkanState.device);

        const VkExtent2D& extent = vulkanState.swapchain.getExtent();
        ubo.create(vulkanState.maxFramesInFlight, vulkanState.allocator);

        renderPass.createCustom(
            vulkanState.device, vulkanState.swapchain,
            [&] {
                VkAttachmentDescription colorAttachment{};
                colorAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
                colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                VkFormat depthFormat = renderPass.findDepthFormat(vulkanState.physicalDevice);
                VkAttachmentDescription depthAttachment{};
                depthAttachment.format = depthFormat;
                depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
                depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                VkAttachmentReference colorAttachmentRef{};
                colorAttachmentRef.attachment = 0;
                colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                VkAttachmentReference depthAttachmentRef{};
                depthAttachmentRef.attachment = 1;
                depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                VkSubpassDescription subpass{};
                subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                subpass.colorAttachmentCount = 1;
                subpass.pColorAttachments = &colorAttachmentRef;
                subpass.pDepthStencilAttachment = &depthAttachmentRef;

                std::array<VkSubpassDependency, 2> dependencies;

                dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
                dependencies[0].dstSubpass = 0;
                dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
                dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                dependencies[0].dstAccessMask =
                    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

                dependencies[1].srcSubpass = 0;
                dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
                dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
                dependencies[1].srcAccessMask =
                    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

                std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

                VkRenderPassCreateInfo renderPassInfo{};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                renderPassInfo.pAttachments = attachments.data();
                renderPassInfo.subpassCount = 1;
                renderPassInfo.pSubpasses = &subpass;
                renderPassInfo.dependencyCount = dependencies.size();
                renderPassInfo.pDependencies = dependencies.data();

                VkRenderPass renderPass;

                if (vkCreateRenderPass(vulkanState.device, &renderPassInfo, nullptr, &renderPass) !=
                    VK_SUCCESS) {
                    throw std::runtime_error("Failed to create render pass!");
                }

                return renderPass;
            },
            [&](const VkExtent2D& extent) {
                colorImage = Image(vulkanState.allocator, extent.width, extent.height,
                                   VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                   VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                colorImageView =
                    colorImage.createView(VK_IMAGE_ASPECT_COLOR_BIT, vulkanState.device);

                VkFormat depthFormat = renderPass.findDepthFormat(vulkanState.physicalDevice);
                depthImage = Image(vulkanState.allocator, extent.width, extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                depthImageView = depthImage.createView(VK_IMAGE_ASPECT_DEPTH_BIT, vulkanState.device);
            },
            [=] {
                vkDestroyImageView(vulkanState.device, colorImageView, nullptr);
                colorImage.destroy(vulkanState.allocator);

                vkDestroyImageView(vulkanState.device, depthImageView, nullptr);
                depthImage.destroy(vulkanState.allocator);
            },
            [&](std::vector<VkImageView>& attachments, VkImageView imageView) {
                attachments.push_back(colorImageView);
                attachments.push_back(depthImageView);
            });

        finalRenderPass.create(vulkanState.physicalDevice, vulkanState.device,
                               vulkanState.allocator, vulkanState.swapchain, true, false);

        finalPipeline.createDescriptorSetLayout(
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

                VkDescriptorSetLayoutBinding depthSamplerLayoutBinding{};
                depthSamplerLayoutBinding.binding = 2;
                depthSamplerLayoutBinding.descriptorCount = 1;
                depthSamplerLayoutBinding.descriptorType =
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                depthSamplerLayoutBinding.pImmutableSamplers = nullptr;
                depthSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                bindings.push_back(uboLayoutBinding);
                bindings.push_back(samplerLayoutBinding);
                bindings.push_back(depthSamplerLayoutBinding);
            });
        finalPipeline.createDescriptorPool(
            vulkanState.maxFramesInFlight, vulkanState.device,
            [&](std::vector<VkDescriptorPoolSize> poolSizes) {
                poolSizes.resize(3);
                poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                poolSizes[0].descriptorCount = static_cast<uint32_t>(vulkanState.maxFramesInFlight);
                poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                poolSizes[1].descriptorCount = static_cast<uint32_t>(vulkanState.maxFramesInFlight);
                poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                poolSizes[2].descriptorCount = static_cast<uint32_t>(vulkanState.maxFramesInFlight);
            });
        finalPipeline.createDescriptorSets(
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

                VkDescriptorImageInfo depthImageInfo{};
                depthImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                depthImageInfo.imageView = colorImageView;
                depthImageInfo.sampler = colorSampler;

                descriptorWrites.resize(3);

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

                descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[2].dstSet = descriptorSet;
                descriptorWrites[2].dstBinding = 2;
                descriptorWrites[2].dstArrayElement = 0;
                descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[2].descriptorCount = 1;
                descriptorWrites[2].pImageInfo = &depthImageInfo;

                vkUpdateDescriptorSets(vulkanState.device,
                                       static_cast<uint32_t>(descriptorWrites.size()),
                                       descriptorWrites.data(), 0, nullptr);
            });
        finalPipeline.create<VertexData, InstanceData>("res/renderTextureFinalShader.vert.spv",
                                                       "res/renderTextureFinalShader.frag.spv",
                                                       vulkanState.device, finalRenderPass, false);

        pipeline.createDescriptorSetLayout(
            vulkanState.device, [&](std::vector<VkDescriptorSetLayoutBinding>& bindings) {
                VkDescriptorSetLayoutBinding uboLayoutBinding{};
                uboLayoutBinding.binding = 0;
                uboLayoutBinding.descriptorCount = 1;
                uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                uboLayoutBinding.pImmutableSamplers = nullptr;
                uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                bindings.push_back(uboLayoutBinding);
            });
        pipeline.createDescriptorPool(vulkanState.maxFramesInFlight, vulkanState.device,
                                      [&](std::vector<VkDescriptorPoolSize> poolSizes) {
                                          poolSizes.resize(1);
                                          poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                                          poolSizes[0].descriptorCount =
                                              static_cast<uint32_t>(vulkanState.maxFramesInFlight);
                                      });
        pipeline.createDescriptorSets(
            vulkanState.maxFramesInFlight, vulkanState.device,
            [&](std::vector<VkWriteDescriptorSet>& descriptorWrites, VkDescriptorSet descriptorSet,
                size_t i) {
                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = ubo.getBuffer(i);
                bufferInfo.offset = 0;
                bufferInfo.range = ubo.getDataSize();

                descriptorWrites.resize(1);

                descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[0].dstSet = descriptorSet;
                descriptorWrites[0].dstBinding = 0;
                descriptorWrites[0].dstArrayElement = 0;
                descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrites[0].descriptorCount = 1;
                descriptorWrites[0].pBufferInfo = &bufferInfo;

                vkUpdateDescriptorSets(vulkanState.device,
                                       static_cast<uint32_t>(descriptorWrites.size()),
                                       descriptorWrites.data(), 0, nullptr);
            });
        pipeline.create<VertexData, InstanceData>("res/renderTextureShader.vert.spv",
                                                  "res/renderTextureShader.frag.spv",
                                                  vulkanState.device, renderPass, false);

        clearValues.resize(2);
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};
    }

    void update(VulkanState& vulkanState) {}

    void render(VulkanState& vulkanState, VkCommandBuffer commandBuffer, uint32_t imageIndex,
                uint32_t currentFrame) {
        const VkExtent2D& extent = vulkanState.swapchain.getExtent();

        UniformBufferData uboData{};
        uboData.model =
            glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        uboData.view = glm::lookAt(glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                   glm::vec3(0.0f, 0.0f, 1.0f));
        uboData.proj =
            glm::perspective(glm::radians(45.0f), extent.width / (float)extent.height, 0.1f, 20.0f);
        uboData.proj[1][1] *= -1;

        ubo.update(uboData);

        vulkanState.commands.beginBuffer(currentFrame);

        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        renderPass.begin(imageIndex, commandBuffer, extent, clearValues);
        pipeline.bind(commandBuffer, currentFrame);

        voxelModel.draw(commandBuffer);

        renderPass.end(commandBuffer);

        clearValues[0].color = {{0.0f, 0.0f, 1.0f, 1.0f}};
        finalRenderPass.begin(imageIndex, commandBuffer, extent, clearValues);
        finalPipeline.bind(commandBuffer, currentFrame);

        voxelModel.draw(commandBuffer);

        finalRenderPass.end(commandBuffer);

        vulkanState.commands.endBuffer(currentFrame);
    }

    void resize(VulkanState& vulkanState, int32_t width, int32_t height) {
        renderPass.recreate(vulkanState.physicalDevice, vulkanState.device, vulkanState.allocator,
                            vulkanState.swapchain);
        finalRenderPass.recreate(vulkanState.physicalDevice, vulkanState.device,
                                 vulkanState.allocator, vulkanState.swapchain);
        finalPipeline.recreate<VertexData, InstanceData>(
            vulkanState.device, vulkanState.maxFramesInFlight, finalRenderPass);
    }

    void cleanup(VulkanState& vulkanState) {
        pipeline.cleanup(vulkanState.device);
        finalPipeline.cleanup(vulkanState.device);
        renderPass.cleanup(vulkanState.allocator, vulkanState.device);
        finalRenderPass.cleanup(vulkanState.allocator, vulkanState.device);

        ubo.destroy(vulkanState.allocator);

        vkDestroySampler(vulkanState.device, colorSampler, nullptr);

        vkDestroySampler(vulkanState.device, textureSampler, nullptr);
        vkDestroyImageView(vulkanState.device, textureImageView, nullptr);
        textureImage.destroy(vulkanState.allocator);

        voxelModel.destroy(vulkanState.allocator);
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
            renderer.run("Render Texture", 640, 480, 2, initCallback, updateCallback,
                         renderCallback, resizeCallback, cleanupCallback);
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