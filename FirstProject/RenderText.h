#pragma once
#include <iostream>
#include <string>
#include <map>
//GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H

#include "ShaderLoader.h"

struct Character {
	GLuint TextureID;
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	GLuint Advance;
};

class RenderText
{
public:
	RenderText(int screen_width, int screen_height);
	void DrawString(std::string, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
private:
	std::map<GLchar, Character> Characters;
	GLuint VAO;
	GLuint VBO;
	GLuint program;
	glm::mat4 projection_transform;

};

