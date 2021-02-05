#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>

#include "texture.hpp"

class ObjLoader {
public:
    struct MaterialInfo {
        std::string name;
        glm::vec3 ambient{};
        glm::vec3 diffuse{};
        glm::vec3 specular{};
        float shininess{};

    };
    struct loadedOBJ {
        std::vector<uint> indices;
        std::vector<float> vertices;
        std::vector<float> texCoords;
        std::vector<float> normals;
        MaterialInfo material;
    };


private:
    static std::vector<loadedOBJ> doTheSceneProcessing(const aiScene *scene) {
        std::vector<loadedOBJ> loadedMeshes;
        std::vector<MaterialInfo> materials;

        // loading textures
        LOG_S(INFO) << "Materials: " << scene->mNumMaterials;
        for (int i = 0; i < scene->mNumMaterials; ++i) {
            LOG_S(INFO) << "Mat_Name: " << scene->mMaterials[i]->GetName().C_Str();
            materials.push_back(processMaterial(scene->mMaterials[i]));
        }

        LOG_S(INFO) << "Meshes: " << scene->mNumMeshes;
        for (int i = 0; i < scene->mNumMeshes; ++i) {
            LOG_S(INFO) << "Mesh(" << i << ")";
            auto mesh = scene->mMeshes[i];
            uint num_faces = mesh->mNumFaces;

            std::vector<uint> indices;
            std::vector<float> vertices;
            std::vector<float> texCoords;
            std::vector<float> normals;
            // trying to extract indices
            for (int j = 0; j < num_faces; ++j) {
                auto face = mesh->mFaces[j];
                for (int k = 0; k < face.mNumIndices; ++k) {
                    auto uv = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][face.mIndices[k]] : aiVector3D(0.0f,
                                                                                                                 0.0f,
                                                                                                                 0.0f);
                    texCoords.push_back(uv.x);
                    texCoords.push_back(uv.y);

                    auto vertex = mesh->mVertices[face.mIndices[k]];

                    vertices.push_back(vertex.x);
                    vertices.push_back(vertex.y);
                    vertices.push_back(vertex.z);

                    auto normal = mesh->mNormals[face.mIndices[k]];

                    normals.push_back(normal.x);
                    normals.push_back(normal.y);
                    normals.push_back(normal.z);
                }
            }

            LOG_S(INFO) << "vertices: " << vertices.size();
            LOG_S(INFO) << "texCoords: " << texCoords.size();
            //LOG_S(INFO) << "indices: " << indices.size();
            LOG_S(INFO) << "normals: " << normals.size();
            LOG_S(INFO) << "material: " << materials[mesh->mMaterialIndex].name;

            LOG_S(INFO) << "---------------";
            loadedMeshes.push_back({indices, vertices, texCoords, normals,materials[mesh->mMaterialIndex]});
        }

        return loadedMeshes;
    }

    // C++ importer interface
    // Output data structure
    // Post processing flags
    std::vector<loadedOBJ> doTheImportThing(const std::string &pFile) {
        // Create an instance of the Importer class
        Assimp::Importer importer;
        // And have it read the given file with some example postprocessing
        // Usually - if speed is not the most important aspect for you - you'll
        // probably to request more postprocessing than we do in this example.
        const aiScene *scene = importer.ReadFile(pFile,
                                                 aiProcess_GenSmoothNormals |
                                                 aiProcess_CalcTangentSpace |
                                                 aiProcess_Triangulate |
                                                 aiProcess_JoinIdenticalVertices |
                                                 aiProcess_SortByPType);
        // If the import failed, report it
        if (!scene) {
            LOG_S(FATAL) << "Failed to load file: " << importer.GetErrorString();
        }
        // Now we can access the file's contents.
        return doTheSceneProcessing(scene);
    }

    static MaterialInfo processMaterial(aiMaterial *material) {
        MaterialInfo mat;
        mat.name = material->GetName().C_Str();
        int shadingModel;
        material->Get(AI_MATKEY_SHADING_MODEL, shadingModel);

        if (shadingModel != aiShadingMode_Phong && shadingModel != aiShadingMode_Gouraud) {
            LOG_S(WARNING)
            << "This mesh's shading model is not implemented in this loader, setting to default material";
            mat.name = "DefaultMaterial";
        } else {
            aiColor3D dif(0.f, 0.f, 0.f);
            aiColor3D amb(0.f, 0.f, 0.f);
            aiColor3D spec(0.f, 0.f, 0.f);
            float shine = 0.0;

            material->Get(AI_MATKEY_COLOR_AMBIENT, amb);
            material->Get(AI_MATKEY_COLOR_DIFFUSE, dif);
            material->Get(AI_MATKEY_COLOR_SPECULAR, spec);
            material->Get(AI_MATKEY_SHININESS, shine);
            mat.ambient = {amb.r, amb.g, amb.b};
            mat.diffuse = {dif.r, dif.g, dif.b};
            mat.specular = {spec.r, spec.g, spec.b};
            mat.shininess = shine;

            mat.ambient *= 0.2;
            if (mat.shininess == 0.0)
                mat.shininess = 30;
        }
        return mat;
    }

public:
    std::vector<loadedOBJ> loadObj(const std::string &filename) {
        return doTheImportThing(filename);
    }

};