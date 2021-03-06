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

#define EPSILON 0.01
#define RESISTANCE	1.0
#define ACCELERATION 2.3
// Below are the Average run speed, terminal velocity and average run speed.
#define LIMIT_X 12.5
#define LIMIT_Y 55.0
#define LIMIT_Z 12.5

class Player_Controller
{
public:
	// Constructor
	Player_Controller();
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

	void setForwardVector(glm::vec3);

	// Collision
	glm::vec3 get_lower_bounds();
	glm::vec3 get_upper_bounds();

	glm::vec3* get_location();
	glm::quat get_rotation();

	float timer;
	
	void clip(bool is_clipping);

private:
	bool clipping;
	GLfloat no_clip_empty = 0;
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
	float m_height;

	// Collision
	glm::vec3 m_lower_bounds;
	glm::vec3 m_upper_bounds;

	// Location
	glm::vec3 m_location;
	const glm::vec3 m_up = glm::vec3(0,1,0);
	glm::vec3 m_right;
	glm::vec3 m_forward;

	// Rotation Vectors
	glm::quat m_rotation;

	// Controller Options
	bool* keyboard_input;
	bool* mouse_buttons;

	// Scene Components
	loadedComponents* scene_tracker;
	std::map<std::string, Object*>* objects;
	Heightmap* heightmap;
};
