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
#include "../include/Object.h"
#include "../include/Heightmap.h"

#define EPSILON 1.0
#define RESISTANCE	1.0
#define ACCELERATION 2.3
#define LIMIT_X 12.5	// Average run speed
#define LIMIT_Y 55.0	// Terminal Velocity
#define LIMIT_Z 12.5	// Average run speed

class Player_Controller
{
public:
	// Constructor
	Player_Controller(Component* component_pointer, bool* keyboard_input, bool* mouse_buttons, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, loadedComponents* scene_tracker, std::map<std::string, Object*>* objects, Heightmap* heightmap);
	Player_Controller(std::string component_file_name, bool* keyboard_input, bool* mouse_buttons, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, loadedComponents* scene_tracker, std::map<std::string, Object*>* objects, Heightmap* heightmap);

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
	void tick(GLfloat delta);

	// Collision
	glm::vec3 get_lower_bounds();
	glm::vec3 get_upper_bounds();

	glm::vec3* get_location();

private:
	// Build internal values
	void build_static_transform();
	void computer_bounds();

	// Movement
	const GLfloat m_resistance = RESISTANCE;
	const GLfloat m_acceleration = ACCELERATION;
	glm::vec3 m_velocity;
	glm::vec3 m_limits;

	// Appearance
	Component* player_model;
	glm::vec3 m_scale;
	glm::mat4 m_transform;
	
	// Collision
	glm::vec3 m_lower_bounds;
	glm::vec3 m_upper_bounds;

	// Location
	glm::vec3 m_location;
	glm::vec3 m_up;
	glm::vec3 m_right;
	glm::vec3 m_forward;
	
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

	// Scene Components
	loadedComponents* scene_tracker;
	std::map<std::string, Object*>* objects;
	Heightmap* heightmap;
};