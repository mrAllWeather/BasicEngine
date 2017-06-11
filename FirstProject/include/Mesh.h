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
#include <glm/gtc/type_ptr.hpp>

#include "../include/tiny_obj_loader.h"
#include "../include/File_IO.h"

class Mesh;

typedef struct {
	GLuint va;
	GLuint vb[6];  // vertex buffer
	int numTriangles;
	size_t material_id;
} DrawObject;

struct loadedComponents {
	std::map<std::string, std::pair<Mesh*, int>>* Meshes;
	std::map<std::string, std::pair<GLuint, int>>* Textures;
	std::map<std::string, GLuint>* Shaders;
	loadedComponents()
	{
		Meshes = new std::map<std::string, std::pair<Mesh*, int>>;
		Textures = new std::map<std::string, std::pair<GLuint, int>>;
		Shaders = new std::map<std::string, GLuint>;
	};
	~loadedComponents()
	{
		for (auto mesh : *Meshes)
		{
			mesh.second.second = 0;
			delete mesh.second.first;
		}
		delete Meshes;
		delete Textures;
		delete Shaders;
	}
};

void calculate_surface_normal(float Normal[3], float const vertex_1[3], float const vertex_2[3], float const vertex_3[3]);
void calculate_tangent_and_bitangent(float Tangent[3], float Bitangent[3], float const vertex_1[3], float const vertex_2[3], float const vertex_3[3], float uv_1[2], float uv_2[2], float uv_3[2]);

class Mesh {
public:
	Mesh(std::string filename, loadedComponents* scene_tracker, std::string base_mat_location = "./Materials/");
	~Mesh();
	void draw(GLuint shader);
	glm::vec3 get_lower_bounds();
	glm::vec3 get_upper_bounds();
	std::string get_scale();
	bool loaded_successfully = true;
	void remove_instance();

private:
	std::string name;
	glm::mat4 m_transform;
	loadedComponents* scene_tracker;

	std::vector<tinyobj::material_t> materials;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<DrawObject> objects;
	
	tinyobj::attrib_t attrib;

	void setupMesh();
	void setupTextures(std::string base_dir);
	void loadTexture(std::string base_dir, std::string texture_name);
	void generateTransform();
	void compute_bounds();

	// Smallest / Largest value for each axis (unnormalised)
	glm::vec3 bounding_minimum;
	glm::vec3 bounding_maximum;

	// Normalised Bounds (Since everything is normalised to -1 to 1, this is a static value)
	glm::vec3 m_lower_bounds;
	glm::vec3 m_upper_bounds;

	// Center of Bounding
	glm::vec3 bounding_center;
	// Store our scale to normalise the size of this object to 1. Value is 1 / LargestAxisLength
	glm::vec3 offset;

	GLfloat scale;
};