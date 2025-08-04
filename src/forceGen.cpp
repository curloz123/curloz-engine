#include "physics/forceGen.h"
#include "physics/core.h"

void physics::ParticleForceRegistry::updateForce(physics::real duration)
{
    for(auto it = registrations.begin(); it!=registrations.end();++it)
    {
        it->fg->updateForce(it->particle, duration);
    }
}

void physics::ParticleForceRegistry::add(Particle *particle, ParticleForceGenerator *fg)
{
    ParticleForceRegistration registration;
    registration.fg = fg;
    registration.particle = particle;
    registrations.push_back(registration);
}

void particleGravity::updateForce(Particle *particle, physics::real duration)
{
    if(!particle->hasFiniteMass()) return;
    particle->addForce(gravity * particle->getMass());
    
}


