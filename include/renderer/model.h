#pragma once

#include "shaders.h"
#include "mesh.h"
#include "glm/glm.hpp"
#include "imageLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <iostream>
#include <map>

struct BoneInfo
{
    int id;
    glm::mat4 offset;
};

class Model 
{
    public:
        Model(std::string path)
        {
            loadModel(path);
        }
        void Draw(Shader &shader);	
    private:
        std::vector<assimpMesh> meshes;
        std::string directory;
        std::vector<meshTexture> textures_loaded;

        void loadModel(std::string path);
        void processNode(aiNode *node, const aiScene *scene);
        assimpMesh processMesh(aiMesh *mesh, const aiScene *scene);
        std::vector<meshTexture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
        unsigned int TextureFromFile(const char *path);

        //Animation stuff
        std::map<std::string, BoneInfo> m_BoneInfoMap;
        int m_BoneCounter = 0;
        // auto &getBoneInfoMap();
        std::map<std::string, BoneInfo> getBoneInfoMap();
        int getBoneCount();
        void setVertexBoneData2Default(meshVertex& vertex);
        void setVertexBoneData(meshVertex &vertex, int BoneID, float weight);
        void ExtractBoneWeightForVertices(std::vector<meshVertex> vertices, aiMesh* mesh, const aiScene* scene);
};


