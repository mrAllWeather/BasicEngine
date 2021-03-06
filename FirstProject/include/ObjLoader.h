#pragma once
/* Author: Ben Weatherall (a1617712)
 * Description: System to load obj files, including verts, normals and uvs.
 * Tracks built meshes to ensure each file is only loaded once.
 * Strongly influenced by http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
 * Issues: Only handles Triangulated objs; cannot handle non-textured surfaces
*/

#define _CRT_SECURE_NO_WARNINGS
#include <cctype>
#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <sstream>
#include <map>

#include "../include/File_IO.h"

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// GLM
#include <glm/glm.hpp>

struct VertexObjects {
	GLuint* VAO;
	GLuint* VBO;
	GLuint vertices;
};

class ObjLoader{
public:
	ObjLoader();
	~ObjLoader();

	void build_static_mesh(std::string filename, GLuint** VAO, GLuint** VBO, GLuint* vertices);

private:
	std::map<std::string, VertexObjects>* built_meshes;

	bool is_static_mesh_built(std::string);

	void load_mesh(std::string);
	void build_vertex_buffer(	std::string filename,
								std::vector< glm::vec3 >* vertices,
								std::vector< glm::vec3 >* normals,
								std::vector< glm::vec2 >* uvs);
};
