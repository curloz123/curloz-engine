#include "physics/particle.h"
#include "physics/core.h"
#include <assert.h>

void Particle::integrate(physics::real duration)
{
    assert(duration > 0);

    //Update our position, using velocity.
    position.addScaledVector(velocity,duration);

    //Update our velocity using acceleration.
    //Also acceleration is assumed constant g, 
    //we update each frame about external forces
    //acting on it.
    physics::Vector3 resultingAcc = physics::Vector3(0.0f,0.0f,0.0f);
    resultingAcc.addScaledVector(forceAccum, inverseMass);

    velocity.addScaledVector(resultingAcc,duration);
    velocity *= real_pow(damping,duration);
    
    //Clear all the forces
    clearAccumulator();

}

void Particle::clearAccumulator()
{
    forceAccum.clear();
}

void Particle::addForce(const physics::Vector3 &force)
{
    forceAccum += force;
}

bool Particle::hasFiniteMass()
{
    if(inverseMass == (physics::real)0)
    {
        return false;
    }
    return true;
}

physics::real Particle::getMass()
{
    return 1/(physics::real)inverseMass;
}
