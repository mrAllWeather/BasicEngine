#pragma once

#include <fstream>
#include <string>
#include <cerrno>
#include <regex>
#include <iostream>
#include <vector>
#include <map>

#include "File_IO.h"

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>

// STRUCTS
/* Basic Shape Data*/
struct shaders {
	std::vector<GLuint> fragmentShader;
	std::vector<GLuint> vertexShader;
};

// Constants
const std::regex VERT_EXT(".*[.]vert");
const std::regex FRAG_EXT(".*[.]frag");

class ShaderLoader {
public:
	ShaderLoader();
	~ShaderLoader();
	void add_shaders(std::vector<std::string> filenames);
	// Expect Fragment and Vertex filenames
	GLuint build_program(std::pair<std::string, std::string> shaders);
private:

	std::map<std::string, GLuint>* built_shaders;
	std::map<std::pair<std::string, std::string>, GLuint>* built_programs;

	void load_shader(std::string filename);

	bool is_shader_built(std::string);
	bool is_program_built(std::pair<std::string, std::string>);

	GLuint build_shader(GLchar** SourceCode, GLuint type);
	void build_shader_program(std::pair<std::string, std::string> shaders);
};
