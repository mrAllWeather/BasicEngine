#pragma once
// Original Design learnt from http://www.mbsoftworks.sk/index.php?page=tutorials&series=1&tutorial=24
/****************************************************************************************

Class:		Heightmap

Purpose:	Wraps FreeType heightmap loading and rendering, also allowing
to use multiple layers of textures with transitions between them.

****************************************************************************************/

#define NUMTERRAINSHADERS 3

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
	GLuint vb[4];  // vertex buffer
	GLuint idx;
} DrawMap;

class Heightmap
{
public:
	Heightmap(std::string name, std::string height_map_file, loadedComponents* scene_tracker);

	bool LoadHeightMapFromImage(std::string sImagePath);

	void ReleaseHeightmap();

	void Draw(GLuint shader);

	void SetRenderSize(float fQuadSize, float fHeight);
	void SetRenderSize(float fRenderX, float fHeight, float fRenderZ);

	uint32_t GetNumHeightmapRows();
	uint32_t GetNumHeightmapCols();
	
	void setupTextures(std::string);
	void loadTexture(std::string, std::string);



private:
	std::string m_name;
	std::string m_height_file;
	loadedComponents* scene_tracker;

	DrawMap m_map;

	bool bLoaded;
	bool bShaderProgramLoaded;

	uint32_t iRows;
	uint32_t iCols;

	glm::vec3 m_mesh_scale;
	glm::vec2 m_texture_scale;
	glm::vec3 m_location;

	std::vector<tinyobj::material_t>* materials;
};
