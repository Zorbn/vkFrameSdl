#include "buffer.hpp"

Buffer::Buffer() {}

Buffer::Buffer(VmaAllocator allocator, VkDeviceSize byteSize, VkBufferUsageFlags usage,
               bool cpuAccessible)
    : byteSize(byteSize) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = byteSize;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    if (cpuAccessible) {
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    if (byteSize != 0 && vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &buffer,
                                         &allocation, &allocInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer!");
    }
}

void Buffer::copyTo(VmaAllocator& allocator, VkQueue graphicsQueue, VkDevice device,
                    Commands& commands, Buffer& dst) {
    if (byteSize == 0 || dst.getSize() == 0)
        return;

    VkCommandBuffer commandBuffer = commands.beginSingleTime(graphicsQueue, device);

    VkBufferCopy copyRegion{};
    copyRegion.size = dst.byteSize;
    vkCmdCopyBuffer(commandBuffer, buffer, dst.buffer, 1, &copyRegion);

    commands.endSingleTime(commandBuffer, graphicsQueue, device);
}

const VkBuffer& Buffer::getBuffer() { return buffer; }

size_t Buffer::getSize() { return byteSize; }

void Buffer::map(VmaAllocator allocator, void** data) {
    if (byteSize == 0)
        return;

    vmaMapMemory(allocator, allocation, data);
}

void Buffer::unmap(VmaAllocator allocator) {
    if (byteSize == 0)
        return;

    vmaUnmapMemory(allocator, allocation);
}

void Buffer::destroy(VmaAllocator& allocator) {
    if (byteSize == 0)
        return;

    vmaDestroyBuffer(allocator, buffer, allocation);
}

void Buffer::setData(const void* data) {
    if (byteSize == 0)
        return;

    memcpy(allocInfo.pMappedData, data, byteSize);
}