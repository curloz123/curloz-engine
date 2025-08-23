#pragma once
#include "physics/core.h"
#include "renderer/gltLoader.h"
#include "renderer/shaders.h"
#include "renderer/camera.h"
#include "renderer/lighting.h"
#include "renderer/renderCube.h"
#include "renderer/general.h"
#include "physics/particle.h"
#include <bits/stdc++.h>
#include <map>
#include <vector>


class Entity
{
    public:
        static Camera *camera;
        static glm::mat4 viewMatrix;
        static glm::mat4 projectionMatrix;
        static DirectionalLight directionLight;
        static std::vector<PointLight> PointLights;
        static std::vector<Entity*> entities;
        static float distanceEntityAndLight(physics::Vector3 entityPos, glm::vec3 lightPos);
        static int numInfluencingLights;
        static int numEntities;


    private:

        int ID;
        float scale;
        glm::vec3 rotation;
        glm::vec3 position;
        glm::mat4 modelMatrix;
        GLTloader ourModel;
        Shader &shader;
        Particle particle = Particle(physics::Vector3(0.0f,5.0f,0.0f));
        std::map<std::string, float> rotationMap;

        std::vector<PointLight> nearestLights;
        bool callSorting = false;
        void sortNearestLights(std::vector<PointLight> &PointLights);

    public:
        std::string name;
        Entity(std::string modelPath, Shader &shader);
        void setPosition(glm::vec3 newPosition);
        glm::vec3 getPosition();
        void setRotation(glm::vec3 axis, float angle);
        glm::vec3 getRotation();
        void setScale(float scale);
        float getScale();


        void update(float deltaTime);
};
