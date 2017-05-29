#pragma once
/* Author: Ben Weatherall (a1617712)
 * Description: Container class used to handle collection of component meshes into singular object
 * Handles world coordination and placement. Allows component meshes to be referenced by name
 * opening the way for basic animations.
*/

#define _CRT_SECURE_NO_WARNINGS
#define CMESH_REGEX "\\t\\w* .* ([\\d\\.f]*( )?){9}"

#include <map>
#include <string>
#include <fstream>
#include <sstream>

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

#include "../include/Component.h"

// https://stackoverflow.com/questions/2659248/finding-minimum-value-in-a-map

// This does need to be heavily reviewed for rights
// Everything is currently public as we give up our details constantly to pretty much everyone
class Object {
public:
	Object(std::string name, std::string cmesh_details, loadedComponents* scene_tracker);
	Object(std::string name, glm::quat rot, glm::vec3 loc, glm::vec3 scale, loadedComponents* scene_tracker);
	void addComponent(std::string name, std::string mesh_name, glm::quat rot, glm::vec3 loc, glm::vec3 scale);
	void remComponent(std::string name);
	void draw(GLuint shader);

	glm::vec3* getLocation();

	glm::vec3 get_lower_bounds();
	glm::vec3 get_upper_bounds();
	std::string report_bounds();

private:
	void build_static_transform();
	void computer_bounds();

	std::string m_name;
	// List of all out components (by name so we can call them for local transforms if needed)
	std::map<std::string, Component*>* components;
	
	glm::vec3* m_location;
	glm::vec3* m_scale;
	glm::quat* m_rotation;
	glm::mat4 m_transform;

	glm::vec3 m_lower_bounds;
	glm::vec3 m_upper_bounds;
	
	loadedComponents* scene_tracker;

	

};
