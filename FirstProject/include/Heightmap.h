#pragma once
// Original Design learnt from http://www.mbsoftworks.sk/index.php?page=tutorials&series=1&tutorial=24
/****************************************************************************************

Class:		Heightmap

Purpose:	Wraps FreeType heightmap loading and rendering, also allowing
to use multiple layers of textures with transitions between them.

****************************************************************************************/

#define IMAGE_DEPTH 255.0
#define NORMAL_UP_DEPTH 64.0

#include <string>
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// GLM
#include <glm/glm.hpp>
#include <vector>

#include "../include/Mesh.h"

typedef struct {
	GLuint va;
	GLuint vb[6];  // vertex buffer
	GLuint idx;
} DrawMap;

glm::vec3 calculate_surface_normal(glm::vec3 const vertex_1, glm::vec3 const vertex_2, glm::vec3 const vertex_3);

class Heightmap
{
public:
	Heightmap(std::string name, std::string height_map_file, loadedComponents* scene_tracker);
	~Heightmap();

	void ReleaseHeightmap();

	void draw(GLuint shader);

	/*-----------------------------------------------

	Name:	SetRenderSize

	Params:	fRenderX, fHeight, fRenderZ - enter all 3
	dimensions separately

	OR

	fQuadSize, fHeight - how big should be one quad
	of heightmap and height is just height :)

	Result: Sets rendering size (scaling) of heightmap.

	---------------------------------------------*/
	void SetRenderSize(float fQuadSize, float fHeight);
	void SetRenderSize(float fRenderX, float fHeight, float fRenderZ);

	float GetFloor(glm::vec3);

	uint32_t GetNumHeightmapRows();
	uint32_t GetNumHeightmapCols();
	
	unsigned char get_image_value(GLfloat, GLfloat, uint8_t);

private:
	void build_transform();

	void setupTextures(std::string);
	void loadTexture(std::string, std::string);
	bool LoadHeightMapFromImage(std::string sImagePath);

	std::string m_name;
	std::string m_height_file;

	glm::mat4 m_transform;
	int indices_count;

	loadedComponents* scene_tracker;

	DrawMap m_map;

	bool bLoaded;
	bool bShaderProgramLoaded;

	uint32_t iRows;
	uint32_t iCols;
	uint32_t composition;

	unsigned char* map_image;

	glm::vec3 m_mesh_scale;
	glm::vec2 m_texture_scale;
	glm::vec3 m_location;

	std::vector<tinyobj::material_t>* materials;
};
