#pragma once
/* Author: Ben Weatherall (a1617712)
 * Description: Point light class to be used within scene. 
 * Provides details of the location, strength and color of light.
*/

#define _CRT_SECURE_NO_WARNINGS
#include <cstdlib>
#include <regex>
#include <string>
#include <iostream>
#include <sstream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// GLM
#include <glm/glm.hpp>

#define LIGHT_REGEX "\\t.*"

class Light {
public:
	Light(	std::string light_details );
	~Light();

	std::string get_name();
	void tick();

	std::string name;
	GLfloat ambient_strength;
	glm::vec3* location;
	glm::vec3* color;

private:
};
