#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>

#include "glm/glm.hpp"
#include <GL/glew.h>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#endif

class Skybox {
public:

	Skybox();
	GLuint loadCubeTex(std::vector<const GLchar*> faces);
	GLuint CreateVao();
	
	GLuint skyVaoId;
	GLuint skyTexId;
};

#endif
