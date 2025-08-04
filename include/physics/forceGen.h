/*
 * This file initializes the force generator for our engine.
 * We basically use polymorphusm here
*/
#pragma once
#include <vector>
#include "particle.h"

class ParticleForceGenerator
{
    public:
        
        virtual void updateForce(Particle* particle, physics::real duration) = 0;
};

namespace physics
{
    class ParticleForceRegistry
    {
        protected:
            struct ParticleForceRegistration
            {
                Particle *particle;
                ParticleForceGenerator *fg;
            };

            typedef std::vector<ParticleForceRegistration> Registry;
            Registry registrations;

        public:
            void add(Particle *particle, ParticleForceGenerator *fg);
            void remove(Particle *particle, ParticleForceGenerator *fg);
            void clear();
            void updateForce(physics::real duration);
    };
};

class particleGravity : public ParticleForceGenerator
{
    private:
        physics::Vector3 gravity;

    public:
        particleGravity(const physics::Vector3 &gravity):gravity(gravity){}
        void updateForce(Particle* particle, physics::real duration) override;
    
};
