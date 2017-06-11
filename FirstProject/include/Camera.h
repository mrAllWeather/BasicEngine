#pragma once
/*  Author: Ben Weatherall (a1617712)
 *  Description: Camera class for use within Scenes.
 *  Heavily based upon 'Camera' class from LearnOpenGL tutorials (https://learnopengl.com/#!Getting-started/Camera)
 *  Some (now) redundant function calls remain. Will save for the code.
 *  Features: 
 *  	Manual Rotation and Movement
 *	Automatic LookAt (Pass Vec3* of location)
 *	Automatic Rotation Around Object
*/
// Std. Includes
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values (we actually replace Yaw and Pitch straight away, but they may come in useful later on).
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 1.0f;
const GLfloat SENSITIVTY = 0.25f;
// While we have removed Zoom functionality, we still use this for perspective
const GLfloat ZOOM = 1.0f;


// An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// Constructor with vectors
	Camera(glm::vec3 position, glm::vec3 up, GLfloat yaw, GLfloat pitch);

	// Constructor with scalar values
	Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch);

	// Returns the view matrix calculated using Eular Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix();

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime);

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch);

	// Start focusing on this object
	void SetLookFocus(glm::vec3* focus);

	// Save position (if not already circling) and begin rotating at fixed radius and offset from the object
	void SetCircleFocus(glm::vec3* focus, float radius, glm::vec3 offset);

	// Stop circling and return to original position
	void RemoveCircleFocus();

	// Update pitch and roll 
	void LookAtObject(GLboolean constrainPitch);
	
	// Move around object we are circling
	void CircleObject(GLfloat xoffset, GLfloat yoffset);

	void tick();

	std::string report();


	GLfloat Zoom;
	glm::vec3 Position;
	glm::vec3 SavedPosition; // Consider adding to main branch camera
	glm::vec3 WorldPositionOffset;
	glm::vec3 Front;
private:
	// Objects in Scene we are concerned with
	glm::vec3* LookAtFocus;  // What we are looking at
	glm::vec3* CircleFocus;  // What we are circling
	float CircleRadius = 0;
	float Theta = M_PI / 2;
	float Phi = 0.0f;

	// Camera Attributes
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// Eular Angles
	GLfloat Yaw;
	GLfloat Pitch;
	// Camera options
	GLfloat MovementSpeed;
	GLfloat MouseSensitivity;

	// Calculates the front vector from the Camera's (updated) Eular Angles
	void updateCameraVectors();

};
