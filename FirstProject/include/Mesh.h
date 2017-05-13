#pragma once
/*
	Description:
		Combines code from tinyobjloader's example file (viewer.cc) and openGlTutorial's Mesh loader.
	This process is not kind to memory, it will not check to confirm if an object has already been loaded (thus we cannot save buffer space)
	DO NOT USE IN LARGE PRODUCTION! We need to track textures currently in use, as well as objects currently in use
*/

#include <algorithm>
#include <string>
#include <iostream>
#include <limits>
// GL Includes
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../include/tiny_obj_loader.h"
#include "../include/File_IO.h"

typedef struct {
	GLuint vb;  // vertex buffer
	int numTriangles;
	size_t material_id;
} DrawObject;

// TODO: replace all float arrays with vec3s. Fujita is obsessed with STL even when writing code for GLM!
void calculate_surface_normal(float Normal[3], float const vertex_1[3], float const vertex_2[3], float const vertex_3[3]);

class Mesh {
public:
	Mesh(std::string filename, std::map<std::string, GLuint>& scene_textures, std::string base_mat_location = "./Materials/");
	void draw(GLuint shader_program);

private:
	std::map<std::string, GLuint>* loaded_textures;
	std::vector<tinyobj::material_t>* materials;
	std::vector<tinyobj::shape_t>* shapes;

	std::vector<DrawObject>* objects;
	
	tinyobj::attrib_t* attrib;

	void setupMesh();
	void setupTextures(std::string base_dir);
	// Smallest value for each axis (unnormalised)
	glm::vec3 bounding_minimum;
	// Largest value for each axis (unnormalised)
	glm::vec3 bounding_maximum;
	// Store our scale to normalise the size of this object to 1. Value is 1 / LargestAxisLength
	GLfloat normalise_scale;	

};