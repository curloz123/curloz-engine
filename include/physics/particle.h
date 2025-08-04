#pragma once
#include "core.h"
#define real_pow powf

class Particle
{
    public:
        physics::Vector3 position;
        physics::Vector3 velocity;
        // physics::Vector3 acceleration ;
        physics::Vector3 forceAccum;
        physics::real damping;
        physics::real inverseMass;
        bool hasFiniteMass();
        physics::real getMass();
        void addForce(const physics::Vector3& force);
        void clearAccumulator();
        void integrate(physics::real duration);
        glm::vec3 to_glmVec3(const physics::Vector3& vector) const
        {
            return glm::vec3(vector.x , vector.y, vector.z);
        }


};

