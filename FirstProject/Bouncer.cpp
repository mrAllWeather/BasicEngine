
// See description of each function in the header file
#include "Bouncer.h"

#define EPSILON 0.000001

Bouncer::Bouncer(Scene* current_scene, float radius, glm::vec3 limits, float friction )
{
	int nBalls = 0;

	std::map<std::string, ComplexMesh*>::iterator it;
	auto it_end = current_scene->statics->end();

	it = current_scene->statics->find("CueBall");
	if (it != it_end)
	{
		ball_pos.push_back(it->second->location);
		ball_rot.push_back(it->second->rot);
		++nBalls;
	}
	else
	{
		// We really should die here, No cueball really means no camera attach and no game
	}

	// http://stackoverflow.com/questions/17253690/finding-in-a-std-map-using-regex
	std::string key = "Ball_";
	auto lower = current_scene->statics->lower_bound(key);
	auto current = lower;

	while (current != it_end && current->first.compare(0, key.size(), key) == 0)
	{
		std::cout << ".";
		ball_pos.push_back(current->second->location);
		ball_rot.push_back(current->second->rot);
		++nBalls;
		++current;
	}

	std::cout << "\nFound " << nBalls << " Balls\n";

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

//void Bouncer::initialise( const std::vector<glm::vec3> &initData )
void Bouncer::initialise()
{
    for ( int i = 0; i < nBalls; i++ ) {
        vel[i] = glm::vec3(0.0);
        for ( int k = 0; k < 3; k++ ) {
            if ( !xyzActive[k] ) {
                ball_pos[i]->operator[](k) = 0.0;
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
		// Test for Ball Rot
		//TODO Apply rotation here
		if (glm::length(vel[i]) > 0.001)
		{
			// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
			float angle = (glm::length(vel[i]) * dt) / radius;

			glm::vec3 axis = glm::normalize(glm::cross(glm::vec3(0.0, 1.0, 0.0), glm::normalize(vel[i])));

			glm::quat new_rot = glm::angleAxis(angle, axis);
			std::cout << "Ball: " << i << "\tAngle: " << angle << "\tAxis: " << axis.x << ":" << axis.y << ":" << axis.z << std::endl;
			std::cout << "\tAdd Rot:" << new_rot.x << ":" << new_rot.y << ":" << new_rot.z << std::endl;

			*ball_rot[i] = new_rot * *ball_rot[i];
			std::cout << "New Rot: " << ball_rot[i]->x << ":" << ball_rot[i]->y << ":" << ball_rot[i]->z << std::endl;
		}

		*ball_pos[i] += vel[i] * dt;
        // pos[i] += vel[i] * dt;            
    
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
		/*
            if ( pos[i][k] > limits[k] - radius ) {
                pos[i][k] = limits[k] - radius;
                vel[i][k] = -vel[i][k];
            }                
            else if ( pos[i][k] < -limits[k] + radius ) {
                pos[i][k] = -limits[k] + radius;
                vel[i][k] = -vel[i][k];
            }                    
		*/
			
			if (ball_pos[i]->operator[](k) > limits[k] - radius) {
				ball_pos[i]->operator[](k) = limits[k] - radius;
				vel[i][k] = -vel[i][k];
			}
			else if (ball_pos[i]->operator[](k) < -limits[k] + radius) {
				ball_pos[i]->operator[](k) = -limits[k] + radius;
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
			/*
            if ( xyzActive[k] ) {
                dist += (pos[i][k] - pos[j][k])*(pos[i][k] - pos[j][k]);
            }
			*/
			if (xyzActive[k]) {
				dist += (ball_pos[i]->operator[](k) - ball_pos[j]->operator[](k))*(ball_pos[i]->operator[](k) - ball_pos[j]->operator[](k));
			}
        }
        
        // If the distance between balls is less than 2 radii, they have collided.
        // Collision response for elastic collision:
        //   Motion along direction joining sphere centres is swapped
        //   Motion perpendicular to this is preserved
        float diff = sqrt(dist) - 2*radius;
        if ( diff < 0.0 ) 
        {
            // glm::vec3 inter(pos[j] - pos[i] );
			glm::vec3 inter(*ball_pos[j] - *ball_pos[i]);
            inter = glm::normalize( inter );
            glm::vec3 ui( inter * glm::dot( inter, vel[i] ) );
            glm::vec3 uj( -inter * glm::dot( -inter, vel[j] ) );
            glm::vec3 vi( vel[i] - ui );
            glm::vec3 vj( vel[j] - uj );
                            
            vel[i] = uj + vi;
            vel[j] = ui + vj;
            
            // pos[i] += diff * 0.5f * inter;
            // pos[j] -= diff * 0.5f * inter;                
			*ball_pos[i] += diff * 0.5f * inter;
			*ball_pos[j] -= diff * 0.5f * inter;


			//TODO Apply rotation here
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
