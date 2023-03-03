#pragma once

#include <vk_mem_alloc.h>

#include <stdexcept>
#include <vector>

#include "commands.hpp"
#include "queueFamilyIndices.hpp"

class Buffer {
public:
    template <typename T>
    static Buffer fromIndices(VmaAllocator allocator, Commands& commands, VkQueue graphicsQueue,
                              VkDevice device, const std::vector<T>& indices) {
        size_t indexSize = sizeof(indices[0]);

        // Only accept 16 or 32 bit types.
        if (indexSize != 2 && indexSize != 4) {
            throw std::runtime_error(
                "Incorrect size when creating index buffer, indices should be 16 or 32 bit!");
        }

        VkDeviceSize bufferByteSize = indexSize * indices.size();

        Buffer stagingBuffer(allocator, bufferByteSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
        stagingBuffer.setData(indices.data());

        Buffer indexBuffer(allocator, bufferByteSize,
                           VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                           false);

        stagingBuffer.copyTo(allocator, graphicsQueue, device, commands, indexBuffer);
        stagingBuffer.destroy(allocator);

        return indexBuffer;
    }

    template <typename T>
    static Buffer fromVertices(VmaAllocator allocator, Commands& commands, VkQueue graphicsQueue,
                               VkDevice device, const std::vector<T>& vertices) {
        VkDeviceSize bufferByteSize = sizeof(vertices[0]) * vertices.size();

        Buffer stagingBuffer(allocator, bufferByteSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
        stagingBuffer.setData(vertices.data());

        Buffer vertexBuffer(allocator, bufferByteSize,
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                            false);

        stagingBuffer.copyTo(allocator, graphicsQueue, device, commands, vertexBuffer);
        stagingBuffer.destroy(allocator);

        return vertexBuffer;
    }

    Buffer();
    Buffer(VmaAllocator allocator, VkDeviceSize byteSize, VkBufferUsageFlags usage,
           bool cpuAccessible);
    void destroy(VmaAllocator& allocator);
    void setData(const void* data);
    void copyTo(VmaAllocator& allocator, VkQueue graphicsQueue, VkDevice device, Commands& commands,
                Buffer& dst);
    const VkBuffer& getBuffer();
    size_t getSize();
    void map(VmaAllocator allocator, void** data);
    void unmap(VmaAllocator allocator);

private:
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocInfo;
    size_t byteSize = 0;
};
