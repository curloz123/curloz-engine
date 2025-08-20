#include "renderer/entity.h"

Camera *Entity::camera;
glm::mat4 Entity::viewMatrix;
glm::mat4 Entity::projectionMatrix;
DirectionalLight Entity::directionLight;
std::vector<PointLight> Entity::PointLights;

Entity::Entity(GLTloader ourModel, Shader &shader):
    ourModel(ourModel), shader(shader)
{
   entities.push_back(this); 
}

void Entity::setPosition(physics::Vector3 newPosition)
{
    particle.position = newPosition; 
    //Call sorting function here
}

void Entity::update(float deltaTime)
{
    shader.use();
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(particle.position.x, particle.position.y, particle.position.z));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
    shader.setMat3("normalMatrix", glm::mat3(glm::transpose(glm::inverse(modelMatrix) ) ) );
    shader.setMat4("model", modelMatrix);
    shader.setMat4("transform", projectionMatrix * viewMatrix * modelMatrix);
    

    shader.setVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
    shader.setVec3("dirLight.ambient", glm::vec3(0.1f));
    shader.setVec3("dirLight.diffuse", glm::vec3(0.f));
    shader.setVec3("dirLight.specular", glm::vec3(0.2f));


    int i=0;
    for(auto it = PointLights.begin(); it!= PointLights.end(); ++it)
    {
        
        shader.setVec3("pointLights["+ std::to_string(i)+ "].position", it->position);
        shader.setVec3("pointLights["+ std::to_string(i)+ "].ambient", it->ambient);
        shader.setVec3("pointLights["+ std::to_string(i)+ "].diffuse", it->diffuse);
        shader.setVec3("pointLights["+ std::to_string(i)+ "].specular", it->specular);
        shader.setFloat("pointLights["+ std::to_string(i)+ "].constant", 1.0f);
        shader.setFloat("pointLights["+ std::to_string(i)+ "].linear", 0.09f);
        shader.setFloat("pointLights["+ std::to_string(i)+ "].quadratic", 0.032f);
        ++i; 
    }
    shader.setInt("numLights",i);
    shader.setVec3("viewPos", camera->Position);

    shader.setFloat("material.shininess", 128.0f);
    ourModel.Draw(shader);
}


