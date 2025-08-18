#include "physics/forceGen.h"
#include "physics/core.h"
#include <bits/stdc++.h>

void physics::ParticleForceRegistry::updateForce(physics::real duration)
{
    for(auto it = registrations.begin(); it!=registrations.end();++it)
    {
        it->fg->updateForce(it->particle, duration);
    }
    // for(auto &it : registrations)
    // {
    //     it.fg->updateForce(it.particle, duration);
    // }
}

void physics::ParticleForceRegistry::add(Particle *particle, ParticleForceGenerator *fg)
{
    ParticleForceRegistration registration;
    registration.fg = fg;
    registration.particle = particle;
    registrations.push_back(registration);
}

void physics::ParticleForceRegistry::remove(Particle *particle, ParticleForceGenerator *fg)
{
    for(auto it = registrations.begin(); it!=registrations.end(); ++it)
    {
        if(it->particle == particle && it->fg == fg)
        {
            registrations.erase(it);
            break;
        }
    }
}

void physics::ParticleForceRegistry::clear()
{
    registrations.clear();
}

void particleGravity::updateForce(Particle *particle, physics::real duration)
{
    if(!particle->hasFiniteMass()) return;
    particle->addForce(gravity * particle->getMass());
    
}


