#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <cstdlib>
#include <regex>
#include <string>
#include <iostream>
#include <glm\glm.hpp>

#include "ShaderLoader.h"
#include "ObjLoader.h"

class ComplexMesh;

class StaticMesh {
public:
	StaticMesh(	std::string static_details, ComplexMesh* parent);
	~StaticMesh();
	void draw();

private:
	ComplexMesh* parent;
	std::string name;
	GLuint* VAO; // Single Value
	GLuint* VBO; // Verts, Normals, UVs
	GLuint vertices; // Num of Verts
	std::vector<GLuint> texture; // Look at making this a vector for layered textures
	GLuint shader_program;
	glm::vec3* rotation;
	glm::vec3* location;
	glm::vec3* scale;

	void build_texture(std::string texture_file);
};