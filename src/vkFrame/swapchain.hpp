#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>
#include <limits>
#include <stdexcept>
#include <vector>

#include "image.hpp"
#include "queueFamilyIndices.hpp"

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Swapchain {
public:
    void create(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                int32_t windowWidth, int32_t windowHeight,
                VkPresentModeKHR preferredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR);
    void cleanup(VmaAllocator allocator, VkDevice device);
    void recreate(VmaAllocator allocator, VkDevice device, VkPhysicalDevice physicalDevice,
                  VkSurfaceKHR surface, int32_t windowWidth, int32_t windowHeight);

    SwapchainSupportDetails querySupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes,
                                       VkPresentModeKHR preferredPresentMode);
    VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, int32_t windowWidth,
                            int32_t windowHeight);
    VkResult getNextImage(VkDevice device, VkSemaphore semaphore, uint32_t& imageIndex);

    const VkSwapchainKHR& getSwapchain();
    const VkExtent2D& getExtent();
    const VkFormat& getImageFormat();

private:
    VkSwapchainKHR swapchain;
    VkExtent2D extent;
    VkFormat imageFormat;
};