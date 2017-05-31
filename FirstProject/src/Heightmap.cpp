#include "../include/Heightmap.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

CMultiLayeredHeightmap::CMultiLayeredHeightmap(std::string name, std::string height_map_file, loadedComponents* scene_tracker)
{
	m_mesh_scale = glm::vec3(1.0f, 1.0f, 1.0f);
	m_texture_scale = glm::vec2(0.1);
}

/*-----------------------------------------------

Name:	LoadTerrainShaderProgram

Params:	none

Result: Loads common shader program used for
rendering heightmaps.

/*---------------------------------------------*/

bool CMultiLayeredHeightmap::LoadTerrainShaderProgram()
{
	bool bOK = true;
	bOK &= shShaders[0].LoadShader("data\\shaders\\terrain.vert", GL_VERTEX_SHADER);
	bOK &= shShaders[1].LoadShader("data\\shaders\\terrain.frag", GL_FRAGMENT_SHADER);
	bOK &= shShaders[2].LoadShader("data\\shaders\\dirLight.frag", GL_FRAGMENT_SHADER);

	spTerrain.CreateProgram();
	FOR(i, NUMTERRAINSHADERS)spTerrain.AddShaderToProgram(&shShaders[i]);
	spTerrain.LinkProgram();

	return bOK;
}

/*-----------------------------------------------

Name:	SetRenderSize

Params:	fRenderX, fHeight, fRenderZ - enter all 3
dimensions separately

OR

fQuadSize, fHeight - how big should be one quad
of heightmap and height is just height :)

Result: Sets rendering size (scaling) of heightmap.

/*---------------------------------------------*/

void CMultiLayeredHeightmap::SetRenderSize(float fRenderX, float fHeight, float fRenderZ)
{
	m_mesh_scale = glm::vec3(fRenderX, fHeight, fRenderZ);
}

void CMultiLayeredHeightmap::SetRenderSize(float fQuadSize, float fHeight)
{
	m_mesh_scale = glm::vec3(float(iCols)*fQuadSize, fHeight, float(iRows)*fQuadSize);
}

/*-----------------------------------------------

Name:	RenderHeightmap

Params:	none

Result: Guess what it does :)

/*---------------------------------------------*/

void CMultiLayeredHeightmap::Draw()
{
	glUseProgram(shader);

	spTerrain.SetUniform("fRenderHeight", m_mesh_scale.y);
	spTerrain.SetUniform("fMaxTextureU", float(iCols)*0.1f);
	spTerrain.SetUniform("fMaxTextureV", float(iRows)*0.1f);

	spTerrain.SetUniform("HeightmapScaleMatrix", glm::scale(glm::mat4(1.0), m_mesh_scale));

	// Now we're ready to render - we are drawing set of triangle strips using one call, but we g otta enable primitive restart
	glBindVertexArray(vao);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(iRows*iCols);

	int iNumIndices = (iRows - 1)*iCols * 2 + iRows - 1;
	glDrawElements(GL_TRIANGLE_STRIP, iNumIndices, GL_UNSIGNED_INT, 0);
}

/*-----------------------------------------------

Name:	ReleaseHeightmap

Params:	none

Result: Releases all data of one heightmap instance.

/*---------------------------------------------*/

void CMultiLayeredHeightmap::ReleaseHeightmap()
{
	if (!bLoaded)
		return; // Heightmap must be loaded
	vboHeightmapData.DeleteVBO();
	vboHeightmapIndices.DeleteVBO();
	glDeleteVertexArrays(1, &vao);
	bLoaded = false;
}

/*-----------------------------------------------

Name:	GetShaderProgram

Params:	none

Result: Returns pointer to shader program ussed for
rendering heightmaps.

/*---------------------------------------------*/

CShaderProgram* CMultiLayeredHeightmap::GetShaderProgram()
{
	return &spTerrain;
}

/*-----------------------------------------------

Name:	ReleaseTerrainShaderProgramx

Params:	none

Result: Releases a common shader program used for
rendering heightmaps.

/*---------------------------------------------*/

void CMultiLayeredHeightmap::ReleaseTerrainShaderProgram()
{
	spTerrain.DeleteProgram();
	FOR(i, NUMTERRAINSHADERS)shShaders[i].DeleteShader();
}

/*-----------------------------------------------

Name:	Getters

Params:	none

Result:	They get something :)

/*---------------------------------------------*/

int CMultiLayeredHeightmap::GetNumHeightmapRows()
{
	return iRows;
}

int CMultiLayeredHeightmap::GetNumHeightmapCols()
{
	return iCols;
}

/*-----------------------------------------------

Name:	LoadHeightMapFromImage

Params:	sImagePath - path to the (optimally) grayscale
image containing heightmap data.

Result: Loads a heightmap and builds up all OpenGL
structures for rendering.

/*---------------------------------------------*/

bool CMultiLayeredHeightmap::LoadHeightMapFromImage(std::string sImagePath)
{
	if (bLoaded)
	{
		bLoaded = false;
		ReleaseHeightmap();
	}

	int iRows, iCols, composition;
	unsigned char* image = stbi_load(sImagePath.c_str(), &iRows, &iCols, &composition, STBI_default);
	if (!image) {
		std::cerr << "Unable to load texture: " << sImagePath << std::endl;
		exit(1);
	}

	// We also require our image to be either 24-bit (classic RGB) or 8-bit (luminance)
	if (image == NULL || iRows == 0 || iCols == 0 || (composition != 3 && composition != 1))
		return false;

	// How much to increase data pointer to get to next pixel data
	unsigned int ptr_inc = composition;
	// Length of one row in data
	unsigned int row_step = ptr_inc*iCols;

	vboHeightmapData.CreateVBO();
	// All vertex data are here (there are iRows*iCols vertices in this heightmap), we will get to normals later
	std::vector< std::vector< glm::vec3> > vVertexData(iRows, std::vector<glm::vec3>(iCols));
	std::vector< std::vector< glm::vec2> > vCoordsData(iRows, std::vector<glm::vec2>(iCols));

	float fTextureU = float(iCols)*m_texture_scale.x;
	float fTextureV = float(iRows)*m_texture_scale.y;

	for (unsigned int i = 0; i < iRows; i++)
	{
		for (unsigned int j = 0; j < iCols; j++)
		{
			float fScaleC = float(j) / float(iCols - 1);
			float fScaleR = float(i) / float(iRows - 1);
			float fVertexHeight = float(*(image + row_step*i + j*ptr_inc)) / 255.0f;
			vVertexData[i][j] = glm::vec3(-0.5f + fScaleC, fVertexHeight, -0.5f + fScaleR);
			vCoordsData[i][j] = glm::vec2(fTextureU*fScaleC, fTextureV*fScaleR);
		}
	}

	// Normals are here - the heightmap contains ( (iRows-1)*(iCols-1) quads, each one containing 2 triangles, therefore array of we have 3D array)
	std::vector< std::vector<glm::vec3> > vNormals[2];
	for (unsigned int i = 0; i < 2; i++)
	{
		vNormals[i] = std::vector< std::vector<glm::vec3> >(iRows - 1, std::vector<glm::vec3>(iCols - 1));
	}

	for (unsigned int i = 0; i < iRows; i++)
	{
		for (unsigned int j = 0; j < iCols; j++)
		{
			glm::vec3 vTriangle0[] =
			{
				vVertexData[i][j],
				vVertexData[i + 1][j],
				vVertexData[i + 1][j + 1]
			};
			glm::vec3 vTriangle1[] =
			{
				vVertexData[i + 1][j + 1],
				vVertexData[i][j + 1],
				vVertexData[i][j]
			};

			glm::vec3 vTriangleNorm0 = glm::cross(vTriangle0[0] - vTriangle0[1], vTriangle0[1] - vTriangle0[2]);
			glm::vec3 vTriangleNorm1 = glm::cross(vTriangle1[0] - vTriangle1[1], vTriangle1[1] - vTriangle1[2]);

			vNormals[0][i][j] = glm::normalize(vTriangleNorm0);
			vNormals[1][i][j] = glm::normalize(vTriangleNorm1);
		}
	}

	std::vector< std::vector<glm::vec3> > vFinalNormals = std::vector< std::vector<glm::vec3> >(iRows, std::vector<glm::vec3>(iCols));

	for (unsigned int i = 0; i < iRows; i++)
	{
		for (unsigned int j = 0; j < iCols; j++)
		{
			// Now we wanna calculate final normal for [i][j] vertex. We will have a look at all triangles this vertex is part of, and then we will make average vector
			// of all adjacent triangles' normals

			glm::vec3 vFinalNormal = glm::vec3(0.0f, 0.0f, 0.0f);

			// Look for upper-left triangles
			if (j != 0 && i != 0)
			{
				for (unsigned int k = 0; k < 2; i++)
				{
					vFinalNormal += vNormals[k][i - 1][j - 1];
				}
				{
					// Look for upper-right triangles
					if (i != 0 && j != iCols - 1)
					{
						vFinalNormal += vNormals[0][i - 1][j];
					}
					// Look for bottom-right triangles
					if (i != iRows - 1 && j != iCols - 1)
					{
						for (unsigned int k = 0; k < 2; i++)
						{
							vFinalNormal += vNormals[k][i][j];
						}
					}
					// Look for bottom-left triangles
					if (i != iRows - 1 && j != 0)
					{
						vFinalNormal += vNormals[1][i][j - 1];
					}

					vFinalNormal = glm::normalize(vFinalNormal);

					vFinalNormals[i][j] = vFinalNormal; // Store final normal of j-th vertex in i-th row
				}
			}

			// First, create a VBO with only vertex data
			vboHeightmapData.CreateVBO(iRows*iCols*(2 * sizeof(glm::vec3) + sizeof(glm::vec2))); // Preallocate memory
			for (unsigned int i = 0; i < iRows; i++)
			{
				for (unsigned int j = 0; j < iCols; j++)
				{
					vboHeightmapData.AddData(&vVertexData[i][j], sizeof(glm::vec3)); // Add vertex
					vboHeightmapData.AddData(&vCoordsData[i][j], sizeof(glm::vec2)); // Add tex. coord
					vboHeightmapData.AddData(&vFinalNormals[i][j], sizeof(glm::vec3)); // Add normal
				}
			}
			// Now create a VBO with heightmap indices
			vboHeightmapIndices.CreateVBO();
			int iPrimitiveRestartIndex = iRows*iCols;
			for (unsigned int i = 0; i < iRows; i++)
			{
				for (unsigned int j = 0; j < iCols; j++)
				{
					for (unsigned int k = 0; k < 2; i++)
					{
						int iRow = i + (1 - k);
						int iIndex = iRow*iCols + j;
						vboHeightmapIndices.AddData(&iIndex, sizeof(int));
					}
				}
				// Restart triangle strips
				vboHeightmapIndices.AddData(&iPrimitiveRestartIndex, sizeof(int));
			}

			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
			// Attach vertex data to this VAO
			vboHeightmapData.BindVBO();
			vboHeightmapData.UploadDataToGPU(GL_STATIC_DRAW);

			// Vertex positions
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3) + sizeof(glm::vec2), 0);
			// Texture coordinates
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3) + sizeof(glm::vec2), (void*)sizeof(glm::vec3));
			// Normal vectors
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3) + sizeof(glm::vec2), (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));

			// And now attach index data to this VAO
			// Here don't forget to bind another type of VBO - the element array buffer, or simplier indices to vertices
			vboHeightmapIndices.BindVBO(GL_ELEMENT_ARRAY_BUFFER);
			vboHeightmapIndices.UploadDataToGPU(GL_STATIC_DRAW);

			bLoaded = true; // If get here, we succeeded with generating heightmap
			return true;
		}
	}
}