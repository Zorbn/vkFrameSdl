#pragma once

#include <cinttypes>

template <typename V, typename I, typename D> class Model {
public:
    static Model<V, I, D> fromVerticesAndIndices(const std::vector<V>& vertices,
                                                 const std::vector<I> indices,
                                                 const size_t maxInstances, VmaAllocator allocator,
                                                 Commands& commands, VkQueue graphicsQueue,
                                                 VkDevice device) {
        Model model = create(maxInstances, allocator, commands, graphicsQueue, device);
        model.size = indices.size();

        model.indexBuffer =
            Buffer::fromIndices(allocator, commands, graphicsQueue, device, indices);
        model.vertexBuffer =
            Buffer::fromVertices(allocator, commands, graphicsQueue, device, vertices);

        return model;
    }

    static Model<V, I, D> create(const size_t maxInstances, VmaAllocator allocator,
                                 Commands& commands, VkQueue graphicsQueue, VkDevice device) {
        Model model;

        size_t instanceByteSize = maxInstances * sizeof(D);
        model.instanceStagingBuffer =
            Buffer(allocator, instanceByteSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
        model.instanceBuffer =
            Buffer(allocator, instanceByteSize,
                   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, false);

        return model;
    };

    void draw(VkCommandBuffer commandBuffer) {
        if (vertexBuffer.getSize() == 0 || instanceBuffer.getSize() == 0 ||
            indexBuffer.getSize() == 0)
            return;

        if (instanceCount < 1)
            return;

        VkIndexType indexType = VK_INDEX_TYPE_UINT16;

        if (sizeof(I) == 4)
            indexType = VK_INDEX_TYPE_UINT32;

        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.getBuffer(), offsets);
        vkCmdBindVertexBuffers(commandBuffer, 1, 1, &instanceBuffer.getBuffer(), offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer.getBuffer(), 0, indexType);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(size),
                         static_cast<uint32_t>(instanceCount), 0, 0, 0);
    }

    void update(const std::vector<V>& vertices, const std::vector<I>& indices, Commands& commands,
                VmaAllocator allocator, VkQueue graphicsQueue, VkDevice device) {
        size = indices.size();

        vkDeviceWaitIdle(device);

        indexBuffer.destroy(allocator);
        vertexBuffer.destroy(allocator);

        indexBuffer = Buffer::fromIndices(allocator, commands, graphicsQueue, device, indices);
        vertexBuffer = Buffer::fromVertices(allocator, commands, graphicsQueue, device, vertices);
    }

    void updateInstances(const std::vector<D>& instances, Commands& commands,
                         VmaAllocator allocator, VkQueue graphicsQueue, VkDevice device) {
        instanceCount = instances.size();
        instanceStagingBuffer.setData(instances.data());
        instanceStagingBuffer.copyTo(allocator, graphicsQueue, device, commands, instanceBuffer);
    }

    void destroy(VmaAllocator allocator) {
        vertexBuffer.destroy(allocator);
        indexBuffer.destroy(allocator);
        instanceStagingBuffer.destroy(allocator);
        instanceBuffer.destroy(allocator);
    }

private:
    Buffer vertexBuffer;
    Buffer indexBuffer;
    Buffer instanceBuffer;
    Buffer instanceStagingBuffer;
    size_t size = 0;
    size_t instanceCount = 0;
};