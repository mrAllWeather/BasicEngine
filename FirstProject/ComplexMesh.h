#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define CMESH_REGEX "\t/w*, \w*, ([\d\.f](, )?){9}"
#include <map>
#include <string>
#include <fstream>
#include <sstream>

#include <GLFW\glfw3.h>
#include <glm\glm.hpp>

//#include "StaticMesh.h"
//#include "ComplexMesh.h"

class Scene;
class StaticMesh;

class ComplexMesh {
public:
	friend class StaticMesh;
	ComplexMesh(std::string cmesh_details, Scene* parent);
	void tick(GLfloat delta);
	void draw();

private:
	std::string name;
	std::map<std::string, StaticMesh*>* components;
	Scene* parent;
	glm::vec3* location;
	glm::vec3* rotation;
	glm::vec3* scale;
};