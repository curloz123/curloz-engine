/*********************************************/
/*********************************************/
/**This file is important for loading models**/
/************of type gltf and glb*************/
/*********************************************/
/*********************************************/



#pragma once


#include "tiny_gltf.h"
#include "glad/glad.h"
#include "shaders.h"
#include "glm/glm.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <map>


struct gltVertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;

    glm::ivec4 BoneIDs = glm::ivec4(0);
    glm::vec4 weights = glm::vec4(0.0);
};

struct glTexture 
{
    GLuint ID;
    std::string type;
    std::string path;
};




struct gltMesh
{
    GLuint VAO,VBO,EBO;
    GLsizei indexCount;
    std::vector<glTexture> texture;
    GLuint metallicRoughnessTexture = 0.0f;
    float metallicFactor ;
    float roughnessFactor ;
    std::string path;
    // std::vector<const unsigned char *> binaryTexturesLoaded;

    void Draw(Shader &shader)
    {
        shader.use();
        glBindVertexArray(VAO);
        GLuint nrDiffuse = 0;
        GLuint nrSpecular = 0;
        for(GLuint i=0; i<texture.size(); ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            if(texture[i].type == "diffuse")
            {
                shader.setInt("material.diffuse" + std::to_string(i), i);
                glBindTexture(GL_TEXTURE_2D, texture[i].ID);
            }
            else if(texture[i].type == "specular")
            {
                shader.setInt("material.specular" + std::to_string(i), i);
                glBindTexture(GL_TEXTURE_2D, texture[i].ID);
            }
        }
        shader.setFloat("material.metallicFactor", metallicFactor);
        shader.setFloat("material.roughnessFactor", roughnessFactor);


        // if(!texture.empty())
        // {
        //     glActiveTexture(GL_TEXTURE0); 
        //     glBindTexture(GL_TEXTURE_2D,texture[0].ID);
        //     shader.setInt("material.diffuse0",0);
        // }
        // if(metallicRoughnessTexture != 0)
        // {
        //     glActiveTexture(GL_TEXTURE1);
        //     glBindTexture(GL_TEXTURE1, metallicRoughnessTexture);
        //     shader.setInt("material.specular0", 1);
        //     shader.setFloat("material.metallicFactor", metallicFactor);
        //     shader.setFloat("material.roughnessFactor", roughnessFactor);
        // }
        glDrawElements(GL_TRIANGLES,indexCount, GL_UNSIGNED_INT,0);

    }   
};

class GLTloader
{
    private:
        static std::map<std::string, glTexture> texturesLoaded;
        std::vector<const unsigned char*> binaryTexturesLoaded;

    public:
        std::string directory;

        //constructor that learns the file location
        GLTloader(const std::string &path);

        //A list of all loaded meshes
        std::vector<gltMesh> meshes;

        //load the .glt* file Returns true if successful
        bool loadModel(const std::string &path , const std::string &type);

        //Draws the model
        void Draw(Shader &shader);

};


