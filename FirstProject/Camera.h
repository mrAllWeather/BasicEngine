#pragma once
/*  Camera Class from LearnOpenGL tutorials (https://learnopengl.com/#!Getting-started/Camera)
	TODO Refine based on my needs (Assess values, and see if can improve for my projects)

*/
// Std. Includes
#include <vector>

// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ComplexMesh.h"



// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 1.0f;
const GLfloat SENSITIVTY = 0.25f;
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

	void SetLookFocus(glm::vec3* focus);
	void SetCircleFocus(glm::vec3* focus, float radius, glm::vec3 offset);
	void RemoveCircleFocus();
	void LookAtObject(GLboolean constrainPitch);
	void CircleObject(GLfloat xoffset, GLfloat yoffset);

	void tick();


	GLfloat Zoom;
	glm::vec3 Position;
	glm::vec3 SavedPosition; // Consider turning this into vector for main system
	glm::vec3 WorldPositionOffset;
	glm::vec3 Front;
private:
	// Objects in Scene we are concerned with
	glm::vec3* LookAtFocus;
	glm::vec3* CircleFocus;
	float CircleRadius = 0;
	float Theta = 7.0f;
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
