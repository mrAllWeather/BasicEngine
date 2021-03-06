#include "../include/Skybox.h"
#include <iostream>
#include <fstream>
#ifndef STB_IMAGE_IMPLEMENTATION

#include "../include/stb_image.h"
#endif

Skybox::Skybox(){
	std::vector<const GLchar*> faces;

	faces.push_back("Materials/posx.tga");
	faces.push_back("Materials/negx.tga");
	faces.push_back("Materials/posy.png");
	faces.push_back("Materials/negy.tga");
	faces.push_back("Materials/posz.tga");
	faces.push_back("Materials/negz.tga");
	skyVaoId = CreateVao();
	skyTexId = loadCubeTex(faces);
}


GLuint Skybox::CreateVao(){
	//vertex positions
	float cubeVertices[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

  // Create VAO
	GLuint vaoHandle;
	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);
	GLuint buffer[1];
	glGenBuffers(1, buffer);

	// Set vertex position
	glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
	glBufferData(GL_ARRAY_BUFFER,
                 sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Un-bind vao
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return vaoHandle;

}


GLuint Skybox::loadCubeTex(std::vector<const GLchar*> faces){

	GLuint text;
	glGenTextures(1, &text);
	glActiveTexture(GL_TEXTURE0);

	int x,y,n;
	unsigned char* data;

	glBindTexture(GL_TEXTURE_CUBE_MAP, text);
	for(GLuint i = 0; i < faces.size(); i++)
	{
        data = stbi_load( faces[i], &x, &y, &n, 0);
        glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return text;
}
