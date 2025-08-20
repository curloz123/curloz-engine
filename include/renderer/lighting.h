#pragma once 
#include "glm/ext/vector_float3.hpp"
#include <glm/glm.hpp>
class Lighting
{
    public:
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
        Lighting(){}
        Lighting(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular):
            ambient(ambient), diffuse(diffuse), specular(specular){}
};

class DirectionalLight : public Lighting
{
    public:
        glm::vec3 direction;
        DirectionalLight(){}
        DirectionalLight
        (
            glm::vec3 direction,
            glm::vec3 ambient,
            glm::vec3 diffuse,
            glm::vec3 specular
        ): 
            Lighting(ambient, diffuse, specular), direction(direction){}
};

class PointLight : public Lighting 
{
    public:
        glm::vec3 position;
        float constant;
        float linear;
        float quadratic;
        
        PointLight
        (
            glm::vec3 position,
            glm::vec3 ambient,
            glm::vec3 diffuse,
            glm::vec3 specular
        ): 
            Lighting(ambient, diffuse, specular), position(position)
        {
            constant = 1.0f;
            linear = 0.009f;
            quadratic = 0.0032f;
        }
        
        void setPosition(glm::vec3 newPosition)
        {
            position = newPosition;
        }
};
