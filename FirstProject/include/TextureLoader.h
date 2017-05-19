#pragma once
/* Author: Ben Weatherall (a1617712)
 * Description: Class to build Textures from provided files
 * Tracks previously loaded textures to avoid duplication
*/

#include <fstream>
#include <string>
#include <cerrno>
#include <regex>
#include <iostream>
#include <vector>
#include <map>

#include "../include/File_IO.h"

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>

class TextureLoader {
public:
	TextureLoader();
	~TextureLoader();
	void add_texture(std::string filename);
	void remove_texture(std::string filename);
	GLuint get_texture(std::string filename);
private:
	std::map<std::string, std::pair<GLuint, GLuint> >* loaded_textures;

	void load_texture(std::string filename);
	bool is_texture_built(std::string);
};
