#pragma once
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

#include "../include/Component.h"

enum Character_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

class Player_Controller
{
public:
	// Constructor
	Player_Controller(Component* component_pointer, bool* keyboard_input, bool* mouse_buttons, glm::vec3 position, glm::vec3 rotation, loadedComponents* scene_tracker);
	Player_Controller(std::string component_file_name, bool* keyboard_input, bool* mouse_buttons, glm::vec3 position, glm::vec3 rotation, loadedComponents* scene_tracker);

	// Appearance
	void set_model(Component* object_pointer);
	void set_create_model(std::string object_file_name);
	void draw(GLuint shader);

	// Motion
		// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(GLfloat deltaTime);
		// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch);
		// Actions over time
	void tick(double delta);

private:
	void build_static_transform();
	void computer_bounds();

	// Appearance
	Component* player_model;
	glm::vec3 m_scale;
	glm::mat4 m_transform;
	
	// Collision
	glm::vec3 m_lower_bounds;
	glm::vec3 m_upper_bounds;

	// Movement
	glm::vec3 m_location;
	glm::vec3 m_up;
	glm::vec3 m_right;
	glm::vec3 m_front;
	
	// Rotation Vectors
	glm::quat m_rotation;
	float Theta = M_PI / 2;
	float Phi = 0.0f;
	GLfloat Yaw;

	// Controller Options
	bool* keyboard_input;
	bool* mouse_buttons;
	GLfloat MovementSpeed;
	GLfloat MouseSensitivity;

	loadedComponents* scene_tracker;
};