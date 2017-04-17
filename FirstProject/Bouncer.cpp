
// See description of each function in the header file
#include "Bouncer.h"

#define EPSILON 0.000001

Bouncer::Bouncer( int nBalls, float radius, glm::vec3 limits, float friction )
{
    this->nBalls = nBalls;
    this->radius = radius;
    this->limits = limits;
    this->friction = friction;
    
    // If limits[i] is too tight to fit the ball, simply fix its
    // pos and vel to zero in that direction.
    for ( int i = 0; i < 3; i++ ) {
        xyzActive[i] = (limits[i] > radius);
    }
    
    pos.resize( nBalls );
    vel.resize( nBalls );
}

Bouncer::~Bouncer()
{}

void Bouncer::initialise( const std::vector<glm::vec3> &initData )
{
    for ( int i = 0; i < nBalls; i++ ) {
        pos[i] = initData[i];
        vel[i] = glm::vec3(0.0);
        for ( int k = 0; k < 3; k++ ) {
            if ( !xyzActive[k] ) {
                pos[i][k] = 0.0;
            }
        }
    }   
}

void Bouncer::strike( int ball, glm::vec3 v )
{
    for ( int k = 0; k < 3; k++ ) {
        if ( xyzActive[k] ) {
            vel[ball][k] += v[k];
        }
    }
}

void Bouncer::update( float dt )
{
    for ( int i = 0; i < nBalls; i++ ) 
    {
        pos[i] += vel[i] * dt;            
    
        bounceOffWalls( i );
        bounceOffOtherBalls( i );
            
        applyFriction( i, dt );
    }            
}

std::vector<glm::vec3> Bouncer::getCurrentPos()
{
    return pos;
}

std::vector<glm::vec3> Bouncer::getCurrentVel()
{
    return vel;
}

// Private functions used for simulation

void Bouncer::bounceOffWalls( int i )
{
    // Elastic collision:
    // Reverse component of velocity normal to wall we've collided with
    for ( int k = 0; k < 3; k++ ) 
    {
        if ( xyzActive[k] ) 
        {
            if ( pos[i][k] > limits[k] - radius ) {
                pos[i][k] = limits[k] - radius;
                vel[i][k] = -vel[i][k];
            }                
            else if ( pos[i][k] < -limits[k] + radius ) {
                pos[i][k] = -limits[k] + radius;
                vel[i][k] = -vel[i][k];
            }                    
        }
    }
}

void Bouncer::bounceOffOtherBalls( int i )
{
    for ( int j = i+1; j < nBalls; j++ ) 
    {
        float dist = 0.0;
        for ( int k = 0; k < 3; k++ ) {
            if ( xyzActive[k] ) {
                dist += (pos[i][k] - pos[j][k])*(pos[i][k] - pos[j][k]);
            }
        }
        
        // If the distance between balls is less than 2 radii, they have collided.
        // Collision response for elastic collision:
        //   Motion along direction joining sphere centres is swapped
        //   Motion perpendicular to this is preserved
        float diff = sqrt(dist) - 2*radius;
        if ( diff < 0.0 ) 
        {
            glm::vec3 inter( pos[j] - pos[i] );
            inter = glm::normalize( inter );
            glm::vec3 ui( inter * glm::dot( inter, vel[i] ) );
            glm::vec3 uj( -inter * glm::dot( -inter, vel[j] ) );
            glm::vec3 vi( vel[i] - ui );
            glm::vec3 vj( vel[j] - uj );
                            
            vel[i] = uj + vi;
            vel[j] = ui + vj;
            
            pos[i] += diff * 0.5f * inter;
            pos[j] -= diff * 0.5f * inter;                
        }
    }           
}    

void Bouncer::applyFriction( int i, float dt )
{
    glm::vec3 zero( 0.0, 0.0, 0.0 );
    
    if ( glm::length( vel[i] ) > EPSILON  ) {   
        float acc = (1.0 - friction*dt);
        if ( acc > 0 )  {
            vel[i] *= acc;
        }
        else {
            vel[i] = zero;
        }
    }
    else if ( vel[i] != zero ) {
        vel[i] = zero;
    }            

}
