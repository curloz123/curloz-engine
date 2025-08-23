#pragma once
#include "glm/glm.hpp"
#include "renderer/shaders.h"
#include "glad/glad.h"
#include "general.h"
#include "shaders.h"
    
struct generalAttributes
{
    Shader shaderCube; 
    GLuint VAO;
    GLuint VBO;
    float vertices[108] = 
    {
            -0.5f, -0.5f, -0.5f, 
             0.5f, -0.5f, -0.5f, 
             0.5f,  0.5f, -0.5f, 
             0.5f,  0.5f, -0.5f, 
            -0.5f,  0.5f, -0.5f, 
            -0.5f, -0.5f, -0.5f, 

            -0.5f, -0.5f,  0.5f, 
             0.5f, -0.5f,  0.5f, 
             0.5f,  0.5f,  0.5f, 
             0.5f,  0.5f,  0.5f, 
            -0.5f,  0.5f,  0.5f, 
            -0.5f, -0.5f,  0.5f, 

            -0.5f,  0.5f,  0.5f, 
             -0.5f,  0.5f, -0.5f, 
            -0.5f, -0.5f, -0.5f, 
            -0.5f, -0.5f, -0.5f, 
            -0.5f, -0.5f,  0.5f, 
            -0.5f,  0.5f,  0.5f, 
            
             0.5f,  0.5f,  0.5f, 
             0.5f,  0.5f, -0.5f, 
             0.5f, -0.5f, -0.5f, 
             0.5f, -0.5f, -0.5f, 
             0.5f, -0.5f,  0.5f, 
             0.5f,  0.5f,  0.5f, 

            -0.5f, -0.5f, -0.5f, 
             0.5f, -0.5f, -0.5f, 
             0.5f, -0.5f,  0.5f, 
             0.5f, -0.5f,  0.5f, 
            -0.5f, -0.5f,  0.5f, 
            -0.5f, -0.5f, -0.5f, 

            -0.5f,  0.5f, -0.5f, 
             0.5f,  0.5f, -0.5f, 
             0.5f,  0.5f,  0.5f, 
             0.5f,  0.5f,  0.5f, 
            -0.5f,  0.5f,  0.5f, 
            -0.5f,  0.5f, -0.5f, 

    };

    void setup()
    {
        shaderCube =  Shader("assets/shaders/renderCube.vert", "assets/shaders/renderCube.frag"); 
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER,VBO);
        glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
        glEnableVertexAttribArray(0);

    }
};
class renderCube
{
    private:

        static generalAttributes attributes;
        glm::mat4 projectionMatrix;
        glm::mat4 viewMatrix;
    public:
        static void bookInAdvance();        
        // void Draw(const float &scale, glm::vec3 &color, glm::vec3 &position, Shader &cubeShader);
        renderCube(const float &scale, glm::vec3 &color, glm::vec3 &position, Shader &shader);

};
