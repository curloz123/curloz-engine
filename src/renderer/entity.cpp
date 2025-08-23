#include "renderer/entity.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/trigonometric.hpp"
#include "physics/core.h"
#include <algorithm>
#include <cmath>

Camera *Entity::camera;
glm::mat4 Entity::viewMatrix = generalInfo.viewMatrix;
glm::mat4 Entity::projectionMatrix = generalInfo.projectionMatrix;
DirectionalLight Entity::directionLight;
std::vector<PointLight> Entity::PointLights;
std::vector<Entity*> Entity::entities;
int Entity::numInfluencingLights = 8;
int Entity::numEntities = 0;

Entity::Entity(std::string modelPath, Shader &shader):
    ourModel(GLTloader(modelPath) ), shader(shader)
{
    //general attributes

    ID = numEntities;
    ++numEntities;

    int pos = modelPath.find(".");
    name = modelPath.substr(0,pos);
    scale = 1.0f;

    rotationMap["x"] = 0.0f;
    rotationMap["y"] = 0.0f;
    rotationMap["z"] = 0.0f;
    rotation = glm::vec3(rotationMap["x"], rotationMap["y"], rotationMap["z"]);

    particle.position = physics::Vector3(0.0f,5.0f,0.0f);

    //Finally push back to static vector
    entities.push_back(this); 
}

float Entity::distanceEntityAndLight(physics::Vector3 entityPos, glm::vec3 lightPos)
{
    return std::sqrt( 
                        pow(
                                (entityPos.x - lightPos.x), 2
                           ) 
                        + 
                        pow(
                                (entityPos.y - lightPos.y), 2
                           ) 
                        + 
                        pow(
                                (entityPos.z - lightPos.z), 2
                           ) 
                    );
}



void Entity::sortNearestLights(std::vector<PointLight> &PointLights)
{
    std::sort(PointLights.begin(), PointLights.end(), [this](PointLight &pre, PointLight &post){
            return distanceEntityAndLight(particle.position, pre.position) < distanceEntityAndLight(particle.position, post.position);
    });
    nearestLights.clear();
    for(int i=0; i<numInfluencingLights; ++i)
    {
        if(i<PointLights.size()) this->nearestLights.push_back(PointLights[i]);
        else break;
    }
}

void Entity::setPosition(glm::vec3 newPosition)
{
    // setting the position finally
    particle.position = physics::Vector3(newPosition.x, newPosition.y, newPosition.z);

    //Call sorting function here
    sortNearestLights(PointLights);
}
glm::vec3 Entity::getPosition()
{
    return glm::vec3(particle.position.x , particle.position.y, particle.position.z);
}

void Entity::setScale(float scale)
{
    this->scale = scale;
}
float Entity::getScale()
{
    return scale;
}

void Entity::setRotation(glm::vec3 axis, float angle)
{
    // setting the rotation finally
    if(axis.x == 1.0f) rotationMap["x"] = angle;
    if(axis.y == 1.0f) rotationMap["y"] = angle;    
    if(axis.z == 1.0f) rotationMap["z"] = angle;    

    rotation = glm::vec3(rotationMap["x"], rotationMap["y"], rotationMap["z"]);
}

glm::vec3 Entity::getRotation()
{
    return rotation; 
}
    
void Entity::update(float deltaTime)
{
    shader.use();
    viewMatrix = generalInfo.viewMatrix;
    projectionMatrix = generalInfo.projectionMatrix;

    viewMatrix = generalInfo.viewMatrix;
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(particle.position.x, particle.position.y, particle.position.z));
    for(auto it : rotationMap)
    {
        if(it.first == "x") modelMatrix = glm::rotate(modelMatrix, glm::radians(it.second), glm::vec3(1.0f,0.0f,0.0f));
        if(it.first == "y") modelMatrix = glm::rotate(modelMatrix, glm::radians(it.second), glm::vec3(0.0f,1.0f,0.0f));
        if(it.first == "z") modelMatrix = glm::rotate(modelMatrix, glm::radians(it.second), glm::vec3(0.0f,0.0f,1.0f));
    }
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
    shader.setMat3("normalMatrix", glm::mat3(glm::transpose(glm::inverse(modelMatrix) ) ) );
    shader.setMat4("model", modelMatrix);
    shader.setMat4("transform", projectionMatrix * viewMatrix * modelMatrix);
    


    shader.setVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
    shader.setVec3("dirLight.ambient", glm::vec3(0.1f));
    shader.setVec3("dirLight.diffuse", glm::vec3(0.1f));
    shader.setVec3("dirLight.specular", glm::vec3(0.2f));
    shader.setInt("numInfluencingLights", numInfluencingLights);

    int i=0;
    std::cout<<"global vector lights size: "<<PointLights.size()<<std::endl;
    std::cout<<"nearest lights size: "<<nearestLights.size()<<std::endl;
    for(auto it = nearestLights.begin(); it!= nearestLights.end(); ++it)
    {
        std::cout<<"x: "<<it->position.x<<" z: "<<it->position.z<<std::endl;     
        shader.setVec3("pointLights["+ std::to_string(i)+ "].position", it->position);
        shader.setVec3("pointLights["+ std::to_string(i)+ "].ambient", it->ambient);
        std::cout<<"r: "<<it->ambient.x<<"y: "<<it->ambient.y<<" z: "<<it->ambient.z<<std::endl;     
        shader.setVec3("pointLights["+ std::to_string(i)+ "].diffuse", it->diffuse);
        std::cout<<"r: "<<it->diffuse.x<<"y: "<<it->diffuse.y<<" z: "<<it->diffuse.z<<std::endl;     
        shader.setVec3("pointLights["+ std::to_string(i)+ "].specular", it->specular);
        std::cout<<"r: "<<it->specular.x<<"y: "<<it->specular.y<<" z: "<<it->specular.z<<std::endl;     
        shader.setFloat("pointLights["+ std::to_string(i)+ "].constant", 1.0f);
        shader.setFloat("pointLights["+ std::to_string(i)+ "].linear", 0.09f);
        shader.setFloat("pointLights["+ std::to_string(i)+ "].quadratic", 0.032f);
        renderCube(0.2f, it->diffuse, it->position, shader);
        ++i; 
    }
    
    shader.setInt("numLights",i);
    shader.setVec3("viewPos", camera->Position);

    shader.setFloat("material.shininess", 128.0f);
    ourModel.Draw(shader);
}
