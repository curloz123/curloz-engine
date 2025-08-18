#pragma once
#define MAX_BONE_INFLUENCE 4
#include <string>
#include <vector>
#include <iostream>
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "shaders.h"


struct meshVertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct meshTexture {
    unsigned int id;
    std::string type;
    std::string path;

};

class assimpMesh {
    public:
        std::vector<meshVertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<meshTexture>      textures;
        unsigned int VAO;

        assimpMesh(std::vector<meshVertex> vertices, std::vector<unsigned int> indices, std::vector<meshTexture> textures)
        {
            this->vertices = vertices;
            this->indices = indices;
            this->textures = textures;

            setupMesh();
        }

        void Draw(Shader &shader) 
        {
            unsigned int diffuseNr  = 1;
            unsigned int specularNr = 1;
            // std::cout<<"Num textures: "<<textures.size()<<std::endl;
            for(unsigned int i = 0; i < textures.size(); ++i)
            {
                glActiveTexture(GL_TEXTURE0 + i); 
                std::string number;
                std::string name = textures[i].type;

                if(name == "texture_diffuse")
                {
                    number = std::to_string(diffuseNr++);
                }
                else if(name == "texture_specular")
                {
                    number = std::to_string(specularNr++); 
                }
                
                glUniform1i(glGetUniformLocation(shader.ID, ("material." + name + number).c_str()), i);
                glBindTexture(GL_TEXTURE_2D, textures[i].id);
            }

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            glActiveTexture(GL_TEXTURE0);
        }

    private:
        unsigned int VBO, EBO;


        void setupMesh()
        {
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(meshVertex), &vertices[0], GL_STATIC_DRAW);  

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);	
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(meshVertex), (void*)0);

            glEnableVertexAttribArray(1);	
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(meshVertex), (void*)offsetof(meshVertex, Normal));

            glEnableVertexAttribArray(2);	
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(meshVertex), (void*)offsetof(meshVertex, TexCoords));

            glEnableVertexAttribArray(3);	
            glVertexAttribIPointer(3, MAX_BONE_INFLUENCE, GL_FALSE, sizeof(meshVertex), (void*)offsetof(meshVertex, m_BoneIDs));

            glEnableVertexAttribArray(4);	
            glVertexAttribPointer(4, MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, sizeof(meshVertex), (void*)offsetof(meshVertex, m_Weights));
           
            glBindVertexArray(0);
        }
};
