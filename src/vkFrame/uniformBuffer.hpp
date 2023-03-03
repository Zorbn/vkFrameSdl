#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "buffer.hpp"

template <typename T> class UniformBuffer {
public:
    void create(const uint32_t maxFramesInFlight, VmaAllocator allocator) {
        VkDeviceSize bufferByteSize = sizeof(T);

        buffers.resize(maxFramesInFlight);
        buffersMapped.resize(maxFramesInFlight);

        for (size_t i = 0; i < maxFramesInFlight; i++) {
            buffers[i] =
                Buffer(allocator, bufferByteSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true);
            buffers[i].map(allocator, &buffersMapped[i]);
        }
    }

    void update(const T& data) {
        size_t bufferCount = buffersMapped.size();
        for (size_t i = 0; i < bufferCount; i++) {
            memcpy(buffersMapped[i], &data, sizeof(T));
        }
    }

    const VkBuffer& getBuffer(uint32_t i) { return buffers[i].getBuffer(); }

    size_t getDataSize() { return sizeof(T); }

    void destroy(VmaAllocator allocator) {
        size_t bufferCount = buffers.size();
        for (size_t i = 0; i < bufferCount; i++) {
            buffers[i].unmap(allocator);
            buffers[i].destroy(allocator);
        }
    }

private:
    std::vector<Buffer> buffers;
    std::vector<void*> buffersMapped;
};