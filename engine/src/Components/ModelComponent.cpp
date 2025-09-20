//
// Created by ghima on 14-09-2025.
//
#include <fstream>

#include "Components/ModelComponent.h"
#include "Core/Constants.h"
#include "Components/MeshComponent.h"
#include "Components/TextureComponent.h"
#include "Entity/GameObject.h"
#include "Entity/Scene.h"

namespace vk {

    ModelComponent::ModelComponent(class vk::GameObject *gameObject, const std::string &id,
                                   const std::string &objectFile) : Component(gameObject, id), mMeshCompList{},
                                                                    mFileName{objectFile} {
    }

    void ModelComponent::BeginPlay() {
        Component::BeginPlay();
        bool isModelLoaded = LoadModel(mFileName);
        if (isModelLoaded) {
            for (std::shared_ptr<TextureComponent> &textureComponent: mTextures) {
                textureComponent->BeginPlay();
            }
            for (std::shared_ptr<MeshComponent> &meshComponent: mMeshCompList) {
                meshComponent->BeginPlay();
            }
        } else {
            LOG_WARN("Failed to load the Model {}", mFileName);
        }
    }

    void ModelComponent::Tick(float DeltaTime) {
        Component::Tick(DeltaTime);
    }

    bool ModelComponent::LoadModel(const std::string &fileName) {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_FlipUVs |
                                                           aiProcess_JoinIdenticalVertices);
        if (!scene) {
            LOG_WARN("Failed to load the Model {}", fileName.c_str());
            return false;
        }
        // Loading the textures;
        LoadTextureMaterials(scene, mTextures, mTextureNames);
        LoadNode(scene->mRootNode, scene, mMeshCompList);
        return true;
    }

    void ModelComponent::LoadTextureMaterials(const aiScene *scene,
                                              List<std::shared_ptr<class TextureComponent>> &textureList,
                                              List<std::string> &textureNames) {
        textureNames.resize(scene->mNumMaterials);

        for (size_t i = 0; i < scene->mNumMaterials; i++) {
            textureNames[i] = "";
            aiMaterial *material = scene->mMaterials[i];
            // Check for the Diffuse textures.
            if (material->GetTextureCount(aiTextureType_DIFFUSE)) {
                aiString path;
                // Get path of the actual file;
                if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
                    // Get the actual File Name;
                    int idx = std::string(path.data).rfind('\\');
                    std::string fileName = std::string(path.data).substr(idx + 1);
                    textureNames[i] = fileName;
                    if (!fileName.empty()) {
                        std::shared_ptr<TextureComponent> textureComponent = std::make_shared<TextureComponent>(
                                Component::mOwningGameObject, R"(D:\cProjects\SmallVkEngine\textures\colormap.png)",
                                mOwningGameObject->GetScene()->GetRendererContext());
                        textureList.push_back(textureComponent);
                    }
                }
            }
        }
    }

    void
    ModelComponent::LoadNode(const aiNode *node, const aiScene *scene, List<std::shared_ptr<MeshComponent>> &meshList) {
        for (size_t i = 0; i < node->mNumMeshes; i++) {
            LoadMesh(scene->mMeshes[node->mMeshes[i]], meshList);
        }
        for (size_t i = 0; i < node->mNumChildren; i++) {
            LoadNode(node->mChildren[i], scene, meshList);
        }
    }

    void ModelComponent::LoadMesh(aiMesh *mesh, List<std::shared_ptr<MeshComponent>> &meshList) {
        List<rn::Vertex> vertices{};
        List<std::uint32_t> indices{};
        vertices.resize(mesh->mNumVertices);

        for (int i = 0; i < mesh->mNumVertices; i++) {
            vertices[i].pos = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
            vertices[i].color = {1, 1, 1, 1};
            // Set the Texture Coords;
            if (mesh->mTextureCoords[0]) {
                vertices[i].uv = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
            } else {
                vertices[i].uv = {0, 0};
            }
        }
        // Iterator through indices
        for (size_t i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            // Getting the indices in the indices;
            for (size_t j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }
        // Getting the texture id;
        std::string textureId = mTextureNames[mesh->mMaterialIndex];
        if (textureId.empty()) {
            textureId = mTextureNames[0];
        };
        char buffer[100]{'\0'};
        std::sprintf(buffer, "%s-mesh-%zu", id.c_str(), mMeshCompList.size() + 1);

        std::string texId = R"(D:\cProjects\SmallVkEngine\textures\colormap.png)";
        std::shared_ptr<MeshComponent> meshComponent = std::make_shared<MeshComponent>(mOwningGameObject,
                                                                                       std::string{buffer}, vertices,
                                                                                       indices, texId, true);
        meshList.push_back(meshComponent);
    }

    const std::shared_ptr<MeshComponent> ModelComponent::GetMeshComponent(std::uint32_t index) const {
        if (index > mMeshCompList.size() - 1) {
            LOG_WARN("Index Exceeds the Mesh List items");
            return nullptr;
        }
        return mMeshCompList[index];
    }

//    bool ModelComponent::LoadModel(const std::string &fileName) {
//        std::ifstream inputStream{fileName};
//        if (!inputStream) {
//            LOG_ERROR("Failed to load The Model File");
//            return false;
//        }
//        std::string lastMaterialName;
//        std::string line;
//        std::string lastName;
//        std::vector<Face> faces;
//        std::vector<std::string> subString;
//
//        std::vector<glm::vec3> positions;
//        std::vector<glm::vec3> normals;
//        std::vector<glm::vec2> textureCoords;
//
//        while (std::getline(inputStream, line)) {
//            subString.clear();
//            Constants::ParseObjectString(line, subString, ' ');
//            // Position Data;
//            if (subString[0] == "v") {
//                glm::vec3 pos{std::stof(subString[1]), std::stof(subString[2]), std::stof(subString[3])};
//                positions.push_back(pos);
//                continue;
//            }
//            // Texture Coordinates
//            if (subString[0] == "vt") {
//                glm::vec2 uv = {std::stof(subString[1]), std::stof(subString[2])};
//                textureCoords.push_back(uv);
//                continue;
//            }
//            // Normal Coordinates
//            if (subString[0] == "vn") {
//                glm::vec3 normal = {std::stof(subString[1]), std::stof(subString[2]), std::stof(subString[3])};
//                normals.push_back(normal);
//                continue;
//            }
//            // Faces
//            if (subString[0] == "f") {
//                Face face{};
//                std::vector<std::string> number;
//                for (int i = 0; i < 3; i++) {
//                    number.clear();
//                    VertexGroup vertexGroup;
//                    Constants::ParseObjectString(subString[i], number, '/');
//                    vertexGroup.v = std::stoi(number[0]) - 1;
//                    vertexGroup.n = std::stoi(number[1]) - 1;
//                    vertexGroup.t = std::stoi(number[2]) - 1;
//                    face.push_back(vertexGroup);
//                }
//                faces.push_back(face);
//                continue;
//            }
//            // Material Groups
//            // Material file
//            // Group Data
//
//            if(!faces.empty()) {
//                List<rn::Vertex> Vertices {};
//                for(Face& face : faces) {
//                    for(VertexGroup& vertexGroup : face) {
//                        rn::Vertex vertex {};
//                        vertex.pos = {};
//                    }
//                }
//            }
//        }
//        inputStream.close();
//        return true;
//    }
}