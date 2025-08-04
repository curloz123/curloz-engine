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
#include <vector>
#include <iostream>
#include "glm/glm.hpp"

struct gltVertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};




struct gltMesh
{
    GLuint VAO,VBO,EBO;
    GLsizei indexCount;
    std::vector<GLuint> texture;

    void Draw(Shader &shader)
    {
        shader.use();
        glBindVertexArray(VAO);
        if(!texture.empty())
        {
            glActiveTexture(GL_TEXTURE0); 
            glBindTexture(GL_TEXTURE_2D,texture[0]);
        }
        glDrawElements(GL_TRIANGLES,indexCount, GL_UNSIGNED_INT,0);

    }   
};

class GLTloader
{
    public:
        std::string directory;

        //constructor that learns the file location
        GLTloader(const std::string &path, const std::string &type);

        //A list of all loaded meshes
        std::vector<gltMesh> meshes;

        //load the .glt* file Returns true if successful
        bool loadModel(const std::string &path , const std::string &type);

        //Draws the model
        void Draw(Shader &shader);

};


