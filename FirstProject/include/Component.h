#pragma once
/* Author: Ben Weatherall (a1617712)
 * Description: Low level component class
 * Contains component VAO, VBO, Shader program and textures.
 * Handles local transformations of the component within it's parent object.
*/


#define _CRT_SECURE_NO_WARNINGS
#include <cstdlib>
#include <regex>
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <sstream>

#include "../include/Mesh.h"

class Component {
public:
    Component();
	Component(std::string name, std::string static_details, loadedComponents* scene_tracker);
	Component(std::string name, std::string mesh_name, glm::quat rot, glm::vec3 loc, glm::vec3 scale, loadedComponents* scene_tracker);
	~Component();

	void draw(GLuint shader);

	glm::vec3 get_lower_bounds();
	glm::vec3 get_upper_bounds();
	bool is_collision(glm::vec3 lower_bound, glm::vec3 upper_bound, glm::mat4 &scene_transform);

private:
	void build_component_transform();

	void compute_bounds();
	void compute_scene_bounds(glm::mat4 &scene_transform);

	std::string m_name;
	std::string m_mesh_name;
	Mesh* m_Mesh;

	glm::quat* m_rotation;
	glm::vec3* m_location;
	glm::vec3* m_scale;
	glm::mat4 m_transform;

	glm::vec3 m_lower_bounds;
	glm::vec3 m_upper_bounds;

	glm::vec3 m_scene_lower_bounds = glm::vec3(0);
	glm::vec3 m_scene_upper_bounds = glm::vec3(0);

	std::vector<std::pair<std::string, GLuint>> textures;

	// Carry so we can update and remove components as needed
	loadedComponents* scene_tracker;
};
