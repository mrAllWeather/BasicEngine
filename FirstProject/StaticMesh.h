#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <cstdlib>
#include <regex>
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ShaderLoader.h"
#include "ObjLoader.h"
#include "TextureLoader.h"

class StaticMesh {
public:
	StaticMesh(	std::string static_details, 
			ShaderLoader* scene_shader_loader, 
			ObjLoader* scene_object_loader,
			TextureLoader* scene_texture_loader
			);
	~StaticMesh();

	std::string name;
	GLuint* VAO; // Single Value
	GLuint* VBO; // Verts, Normals, UVs
	GLuint vertices; // Num of Verts
	std::vector<std::pair<std::string, GLuint>> textures; // Look at making this a vector for layered textures
	GLuint shader_program;
	glm::vec3* rotation;
	glm::vec3* location;
	glm::vec3* scale;
	glm::mat4 component_transform;

private:
	void build_component_transform();
};
