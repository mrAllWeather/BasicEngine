#pragma once
// Original Design learnt from http://www.mbsoftworks.sk/index.php?page=tutorials&series=1&tutorial=24
/****************************************************************************************

Class:		CMultiLayeredHeightmap

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

class CMultiLayeredHeightmap
{
public:
	CMultiLayeredHeightmap(std::string name, std::string height_map_file, loadedComponents* scene_tracker);

	static bool LoadTerrainShaderProgram();
	static void ReleaseTerrainShaderProgram();

	bool LoadHeightMapFromImage(std::string sImagePath);

	void ReleaseHeightmap();

	void Draw();

	void SetRenderSize(float fQuadSize, float fHeight);
	void SetRenderSize(float fRenderX, float fHeight, float fRenderZ);

	int GetNumHeightmapRows();
	int GetNumHeightmapCols();



private:
	std::string name;

	GLuint vao;
	GLuint shader;

	bool bLoaded;
	bool bShaderProgramLoaded;

	int iRows;
	int iCols;

	glm::vec3 m_mesh_scale;
	glm::vec2 m_texture_scale;
	glm::vec3 m_location;

	std::vector<tinyobj::material_t>* materials;

	// CVertexBufferObject vboHeightmapData;
	// CVertexBufferObject vboHeightmapIndices;

	// static CShaderProgram spTerrain;
	// static CShader shTerrainShaders[NUMTERRAINSHADERS];
};