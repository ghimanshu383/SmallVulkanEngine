//
// Created by ghima on 30-08-2025.
//

#ifndef SMALLVKENGINE_STATICMESH_H
#define SMALLVKENGINE_STATICMESH_H

#include "Utility.h"

namespace rn {
    class StaticMesh {
    private:
        List<Vertex> mVertList{};
        List<std::uint32_t> mIndicesList{};
        std::uint32_t mIndicesCount;

        VkBuffer mVertexBuffer{};
        VkBuffer mIndexBuffer{};
        VkDeviceMemory mVertexBufferMemory{};
        VkDeviceMemory mIndexBufferMemory{};
        RendererContext mRenderContext{};
        std::string mTextureId;
    public:
        StaticMesh(RendererContext &ctx, List<Vertex> &Vertices, List<std::uint32_t> &indices, std::string &textureId);

        ~StaticMesh();

        void Init();

        template<typename T>
        void CreateMeshBuffer(List<T> &meshData, VkBufferUsageFlags usageFlags, VkBuffer &buffer,
                              VkDeviceMemory &bufferMemory, std::string &&bufferName) {
            VkBuffer stagingBuffer{};
            VkDeviceMemory stagingBufferMemory{};
            VkDeviceSize bufferSize = sizeof(T) * meshData.size();
            std::string stagingBufferName = "Stating Buffer " + bufferName;
            Utility::CreateBuffer(mRenderContext, stagingBuffer, (VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
                                  stagingBufferMemory,
                                  (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
                                  bufferSize, stagingBufferName);

            void *data;
            vkMapMemory(mRenderContext.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, meshData.data(), bufferSize);
            vkUnmapMemory(mRenderContext.logicalDevice, stagingBufferMemory);

            Utility::CreateBuffer(mRenderContext, buffer,
                                  usageFlags,
                                  bufferMemory, (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT), bufferSize, bufferName);

            Utility::CopyBuffers(mRenderContext, stagingBuffer, buffer, bufferSize);
            vkDestroyBuffer(mRenderContext.logicalDevice, stagingBuffer, nullptr);
            vkFreeMemory(mRenderContext.logicalDevice, stagingBufferMemory, nullptr);
        }

        VkBuffer GetVertexBuffer() const { return mVertexBuffer; }

        VkBuffer GetIndexBuffer() const { return mIndexBuffer; }

        std::uint32_t GetStaticMeshIndicesCount() const { return mIndicesCount; }

        std::string GetTextureId() const { return mTextureId; }
    };
}
#endif //SMALLVKENGINE_STATICMESH_H
