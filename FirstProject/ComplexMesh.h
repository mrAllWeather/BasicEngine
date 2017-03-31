#pragma once
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


#include "StaticMesh.h"

class Scene;

class ComplexMesh {
public:
	friend class StaticMesh;
	ComplexMesh(std::string cmesh_details, ShaderLoader* scene_shader_loader, ObjLoader* scene_object_loader);
	void tick(GLfloat delta);
	void draw();

	std::string name;
	std::map<std::string, StaticMesh*>* components;
	Scene* parent;
	glm::vec3* location;
	glm::vec3* rotation;
	glm::vec3* scale;
	glm::mat4 static_transform;
private:
	void build_static_transform();
};