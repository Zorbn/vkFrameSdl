#pragma once

#include <cmath>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#include "../../deps/stb_image.h"

#include "buffer.hpp"

class Image {
public:
    static Image createTexture(const std::string& image, VmaAllocator allocator, Commands& commands,
                               VkQueue graphicsQueue, VkDevice device, bool enableMipmaps);
    static Image createTextureArray(const std::string& image, VmaAllocator allocator,
                                    Commands& commands, VkQueue graphicsQueue, VkDevice device,
                                    bool enableMipmaps, uint32_t width, uint32_t height,
                                    uint32_t layers);

    Image();
    Image(VkImage image, VkFormat format);
    Image(VkImage image, VmaAllocation allocation, VkFormat format);
    Image(VmaAllocator allocator, uint32_t width, uint32_t height, VkFormat format,
          VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
          uint32_t mipmapLevels = 1, uint32_t layers = 1,
          VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
    VkImageView createTextureView(VkDevice device);
    VkSampler createTextureSampler(VkPhysicalDevice physicalDevice, VkDevice device,
                                   VkFilter minFilter = VK_FILTER_LINEAR,
                                   VkFilter magFilter = VK_FILTER_LINEAR);
    VkImageView createView(VkImageAspectFlags aspectFlags, VkDevice device);
    void transitionImageLayout(Commands& commands, VkImageLayout oldLayout, VkImageLayout newLayout,
                               VkQueue graphicsQueue, VkDevice device);
    void copyFromBuffer(Buffer& src, Commands& commands, VkQueue graphicsQueue, VkDevice device,
                        uint32_t fullWidth = 0, uint32_t fullHeight = 0);
    void generateMipmaps(Commands& commands, VkQueue graphicsQueue, VkDevice device);
    void destroy(VmaAllocator allocator);
    uint32_t getWidth() const;
    uint32_t getHeight() const;

private:
    VkImage image;
    VmaAllocation allocation;
    VkFormat format = VK_FORMAT_R32G32B32_SFLOAT;
    uint32_t layerCount = 1;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t mipmapLevels = 1;

    static Buffer loadImage(const std::string& image, VmaAllocator allocator, int32_t& width,
                            int32_t& height);
    static uint32_t calcMipmapLevels(int32_t texWidth, int32_t texHeight);
};