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

#include "../include/ShaderLoader.h"
#include "../include/ObjLoader.h"
#include "../include/TextureLoader.h"

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
	std::vector<std::pair<std::string, GLuint>> textures; // We may have many textures (we don't, but it's a possibility)
	GLuint shader_program;
	glm::vec3* rotation; // We set this but don't actually use it as we have quats now. 
	glm::quat* rot;
	glm::vec3* location;
	glm::vec3* scale;
	glm::mat4 component_transform;
	void build_component_transform();
	GLfloat specular;
private:
	
};
