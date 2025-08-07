#include "renderer/model.h"
#include "renderer/assimp_glm_helpers.h"
#include <algorithm>

void Model::Draw(Shader &shader)
{
    for(unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);
}  

void Model::loadModel(std::string path)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_ConvertToLeftHanded);	
	
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cout << "ERROR::ASSIMP-> " << import.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}  

void Model::processNode(aiNode *node, const aiScene *scene)
{

    std::cout << "Processing node: " << node->mName.C_Str()<< " with " << node->mNumMeshes << " meshes" << std::endl;
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

assimpMesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::cout << "Processing mesh with " << mesh->mNumVertices << " vertices" << std::endl;
    std::vector<meshVertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<meshTexture> textures;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        meshVertex vertex;
        setVertexBoneData2Default(vertex);

        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal = vector;

        if(mesh->mTextureCoords[0]) 
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
        {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }
    // process indices
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    
    // process material
    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        std::cout << "Checking all texture types:\n";
        for (int i = 1; i < AI_TEXTURE_TYPE_MAX; ++i) {
            aiTextureType type = static_cast<aiTextureType>(i);
            unsigned int count = material->GetTextureCount(type);
            if (count > 0) {
                std::cout << "Type " << i << " (" << type << ") has " << count << " textures\n";
                for (unsigned int t = 0; t < count; ++t) {
                    aiString str;
                    material->GetTexture(type, t, &str);
                    std::cout << "  Texture " << t << ": " << str.C_Str() << std::endl;
              }
            }
        }
        std::vector<meshTexture> diffuseMaps = loadMaterialTextures(material, 
                                            aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<meshTexture> specularMaps = loadMaterialTextures(material, 
                                            aiTextureType_SPECULAR, "texture_specular");
        if(!specularMaps.empty()) std::cout<<"Found models specular maps"<<std::endl;
        else std::cout<<"didnt find any specular maps for model"<<std::endl;
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }
    std::cout << "Mesh processed successfully!" << std::endl;


    //Processsing Bone data
    ExtractBoneWeightForVertices(vertices,mesh,scene);

    return assimpMesh(vertices, indices, textures);
}

std::vector<meshTexture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
{
    std::vector<meshTexture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for(unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true; 
                break;
            }
        }
        if(!skip)
        {   
            meshTexture texture;
            texture.id = TextureFromFile(str.C_Str());
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture); 
        }
    }
    return textures;
}  

unsigned int Model::TextureFromFile(const char *path)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;
    std::replace(filename.begin(), filename.end(),'\\','/');
    std::cout<<"Loading obj/fbx texture-> "<<filename<<std::endl;

    GLuint textureID = loadTextureFromFile(filename.c_str(),false);
    return textureID;
}

//Animation stuff
std::map<std::string, BoneInfo> &Model::getBoneInfoMap()
{
    return m_BoneInfoMap;
}

int &Model::getBoneCount()
{
    return m_BoneCounter;
}

void Model::setVertexBoneData2Default(meshVertex &vertex)
{
    for(int i=0; i<MAX_BONE_INFLUENCE; ++i)
    {
        vertex.m_Weights[i] = 0.0f;
        vertex.m_BoneIDs[i] = -1;
    }
}

void Model::setVertexBoneData(meshVertex &vertex, int BoneID, float weight)
{
    for(int i=0; i<MAX_BONE_INFLUENCE; ++i)
    {
        if(vertex.m_BoneIDs[i]<0)
        {
            vertex.m_BoneIDs[i] = BoneID;
            vertex.m_Weights[i] = weight;
            break;
        }
    }
}

void Model::ExtractBoneWeightForVertices(std::vector<meshVertex> &vertices, aiMesh* mesh, const aiScene* scene)
{
    for(int boneIndex=0; boneIndex<mesh->mNumBones; ++boneIndex)
    {
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        if(m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
        {
            BoneInfo newBoneInfo;
            newBoneInfo.id = m_BoneCounter;
            newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
            m_BoneInfoMap[boneName] = newBoneInfo;            
            boneID = m_BoneCounter;
            ++m_BoneCounter;
        }
        else
        {
            boneID = m_BoneInfoMap[boneName].id;
        }
        if(boneID == -1)
        {
            std::cout<<"Could not load bone"<<std::endl;
            assert(boneID != -1);
        }            
        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;
        
        for(int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
        {
            int vertexID = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;
            assert(vertexID <= vertices.size());
            setVertexBoneData(vertices[vertexID], boneID, weight);
        }
    }
}
