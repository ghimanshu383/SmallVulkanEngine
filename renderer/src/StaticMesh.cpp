//
// Created by ghima on 30-08-2025.
//
#include "StaticMesh.h"
#include "Texture.h"

namespace rn {
    StaticMesh::StaticMesh(RendererContext &ctx, List<rn::Vertex> &Vertices, List<std::uint32_t> &indices,
                           std::uint32_t pickId,
                           std::string &textureId, bool calculateNormals)
            : mRenderContext{ctx}, mVertList{Vertices}, mIndicesList{indices}, mPickId{pickId}, mTextureId{textureId},
              mCalculateNormals{calculateNormals} {
        mIndicesCount = indices.size();
        Init();
    }

    void StaticMesh::CalculateAverageNormals() {
        for (size_t i = 0; i < mIndicesList.size(); i += 3) {
            Vertex &verOne = mVertList[mIndicesList[i]];
            Vertex &vertTwo = mVertList[mIndicesList[i + 1]];
            Vertex &vertThree = mVertList[mIndicesList[i + 2]];

            glm::vec3 normalVecOne = {vertTwo.pos - verOne.pos };
            glm::vec3 normalVecTwo = {vertThree.pos - verOne.pos };
            glm::vec3 normal = glm::normalize(glm::cross(normalVecOne, normalVecTwo));

            verOne.normals += normal;
            vertTwo.normals += normal;
            vertThree.normals += normal;
        }
        for (Vertex &vert: mVertList) {
            vert.normals = glm::normalize(vert.normals);
        }
    }

    void StaticMesh::Init() {


        if (mCalculateNormals) {
            CalculateAverageNormals();
        }
        // Creating the vertex buffers;
        CreateMeshBuffer<Vertex>(mVertList, (VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
                                 mVertexBuffer, mVertexBufferMemory, "VertexBuffer");
        // Creating the indices mesh
        CreateMeshBuffer<std::uint32_t>(mIndicesList,
                                        (VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
                                        mIndexBuffer, mIndexBufferMemory, "IndexBuffer");
    }

    StaticMesh::~StaticMesh() {
        vkDestroyBuffer(mRenderContext.logicalDevice, mVertexBuffer, nullptr);
        vkDestroyBuffer(mRenderContext.logicalDevice, mIndexBuffer, nullptr);
        vkFreeMemory(mRenderContext.logicalDevice, mVertexBufferMemory, nullptr);
        vkFreeMemory(mRenderContext.logicalDevice, mIndexBufferMemory, nullptr);
    }
}