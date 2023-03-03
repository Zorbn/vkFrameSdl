#pragma once

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../deps/stb_image.h"

#include <vk_mem_alloc.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#include "buffer.hpp"
#include "commands.hpp"
#include "model.hpp"
#include "pipeline.hpp"
#include "queueFamilyIndices.hpp"
#include "swapchain.hpp"
#include "uniformBuffer.hpp"

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator);

struct VulkanState {
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;
    VkQueue graphicsQueue;
    VmaAllocator allocator;
    Swapchain swapchain;
    Commands commands;
    uint32_t maxFramesInFlight;
};

class Renderer {
public:
    void
    run(const std::string& windowTitle, const uint32_t windowWidth, const uint32_t windowHeight,
        const uint32_t maxFramesInFlight,
        std::function<void(VulkanState& vulkanState, SDL_Window* window, int32_t width,
                           int32_t height)>
            initCallback,
        std::function<void(VulkanState& vulkanState)> updateCallback,
        std::function<void(VulkanState& vulkanState, VkCommandBuffer commandBuffer,
                           uint32_t imageIndex, uint32_t currentFrame)>
            renderCallback,
        std::function<void(VulkanState& vulkanState, int32_t width, int32_t height)> resizeCallback,
        std::function<void(VulkanState& vulkanState)> cleanupCallback);

private:
    SDL_Window* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkQueue presentQueue;

    VulkanState vulkanState;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

    bool framebufferResized = false;

    void initWindow(const std::string& windowTitle, const uint32_t windowWidth,
                    const uint32_t windowHeight);

    static void framebufferResizeCallback(SDL_Window* window, int width, int height);

    void initVulkan(const uint32_t maxFramesInFlight,
                    std::function<void(VulkanState& vulkanState, SDL_Window* window, int32_t width,
                                       int32_t height)>
                        initCallback);
    void createInstance();
    void createAllocator();
    void createLogicalDevice();

    void mainLoop(std::function<void(VulkanState& vulkanState, VkCommandBuffer commandBuffer,
                                     uint32_t imageIndex, uint32_t currentFrame)>
                      renderCallback,
                  std::function<void(VulkanState& vulkanState)> updateCallback,
                  std::function<void(VulkanState& vulkanState, int32_t width, int32_t height)>
                      resizeCallback);
    void drawFrame(std::function<void(VulkanState& vulkanState, VkCommandBuffer commandBuffer,
                                      uint32_t imageIndex, uint32_t currentFrame)>
                       renderCallback,
                   std::function<void(VulkanState& vulkanState, int32_t width, int32_t height)>
                       resizeCallback);
    void waitWhileMinimized();

    void cleanup(std::function<void(VulkanState& vulkanState)> cleanupCallback);

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();

    void createSurface();

    void pickPhysicalDevice();

    bool hasStencilComponent(VkFormat format);

    void createUniformBuffers();

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void createSyncObjects();

    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();

    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
};