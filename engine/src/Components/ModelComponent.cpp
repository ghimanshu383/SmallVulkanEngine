//
// Created by ghima on 14-09-2025.
//
#include <fstream>

#include "Components/ModelComponent.h"
#include "Core/Constants.h"
#include "Components/MeshComponent.h"

namespace vk {
    List<std::string> ModelComponent::mTextures = {};
    ModelComponent::ModelComponent(class vk::GameObject *gameObject, const std::string &id,
                                   const std::string &objectFile) : Component(gameObject, id), mMeshCompList{} {
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
        List<std::string> textureNames{};

        return true;
    }

    void ModelComponent::LoadTextureMaterials(const aiScene *scene, List<std::string> &textureList) {
        mTextures.resize(scene->mNumMaterials);

        for(size_t i = 0; i < scene->mNumMaterials; i++) {
            aiMaterial* material = scene->mMaterials[i];
            // Insert the texture file name in the string;
            mTextures[i] = "";
            if(material->GetTextureCount(aiTextureType_DIFFUSE)) {

            }
        }
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