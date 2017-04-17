
/**
    This class controls the billiard balls and simulates collisions and friction.
    It makes a few assumptions:
    - the balls are all the same size
    - the balls are moving around in an axis aligned volume centred at the origin
    If you're not sure about anything feel free to ask on the Q&A forum
    
    Anthony Dick
*/

#include <vector>
#include <glm/glm.hpp>
#include "Scene.h"

class Bouncer
{
public:        
    /**
     Constructor. 
     @param nBalls the number of balls in the simulation
     @param radius the radius of each ball
     @param limits the boundary of the volume in which the balls bounce around.
         The volume is defined as:  -limits[0] < x < limits[0]
                                    -limits[1] < y < limits[1]        
                                    -limits[2] < z < limits[2] 
         Collisions are only tested on axes where limits[i] > radius.       
     @param friction a coefficient of friction. The fraction of speed that is lost 
         each second. Higher values denote more friction, minimum value is 0.0.
    */
    Bouncer( Scene* current_scene, float radius, glm::vec3 limits, float friction );    
    ~Bouncer();
    
    /**
     Pass initial position of each ball to the collision system.
     Assumes initial positions are all within volume boundary.
     @param initData the initial positions. 
    */
    void initialise( const std::vector<glm::vec3> &initData );
    
    /**
     Move the simulation forward by dt seconds.
     @param dt amount of time elapsed since last update (in seconds)
    */
    void update( float dt );
    
    /**
     Add the velocity vel to the ball. Can be used to simulate hitting the ball.
     @param ball the index of the ball to strike
     @param vel the velocity added to the ball by the strike
    */ 
    void strike( int ball, glm::vec3 vel );
    
    /**
     Get the current position and velocity of all balls.
     @return a vector containing current position/velocity of all balls.
    */
    std::vector<glm::vec3> getCurrentPos();
    std::vector<glm::vec3> getCurrentVel();

private:    
	std::vector<glm::vec3*> ball_pos;
    int nBalls;
    float radius;
    glm::vec3 limits;
    float friction;
    bool xyzActive[3];
    std::vector<glm::vec3> pos;
    std::vector<glm::vec3> vel;
    
    void bounceOffWalls( int i );
    void bounceOffOtherBalls( int i );
    void applyFriction( int i, float dt );    
};
