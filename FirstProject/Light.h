#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <cstdlib>
#include <regex>
#include <string>
#include <iostream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// GLM
#include <glm/glm.hpp>


class Light {
public:
	Light(	std::string light_details );
	~Light();

	std::string name;
	GLfloat ambient_strength;
	glm::vec3* location;
	glm::vec3* color;

private:
};
