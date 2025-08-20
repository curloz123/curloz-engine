#pragma once
#include "physics/core.h"
#include "renderer/gltLoader.h"
#include "renderer/shaders.h"
#include "renderer/camera.h"
#include "renderer/lighting.h"
#include "physics/particle.h"

class Entity
{
    public:
        static Camera *camera;
        static glm::mat4 viewMatrix;
        static glm::mat4 projectionMatrix;
        static DirectionalLight directionLight;
        static std::vector<PointLight> PointLights;
        static std::vector<Entity*> entities;

    private:

        glm::mat4 modelMatrix;
        GLTloader ourModel;
        Shader &shader;
        Particle particle = Particle(physics::Vector3(0.0f,5.0f,-5.0f));
        bool callSorting = false;
        float scale = 1.0f;

    public:
        Entity(GLTloader ourModel, Shader &shader);
        void setPosition(physics::Vector3 newPosition);

        void update(float deltaTime);
};
