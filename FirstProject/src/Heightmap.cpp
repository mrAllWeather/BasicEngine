#include "../include/Heightmap.h"

#include <fstream>
#include <sstream>
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

CMultiLayeredHeightmap::CMultiLayeredHeightmap(std::string name, std::string height_map_file, loadedComponents* scene_tracker)
{
	m_name = name;
	materials = new std::vector<tinyobj::material_t>;
	std::ifstream fb; // FileBuffer
	fb.open((height_map_file), std::ios::in);
	std::string LineBuf, component_name;
	std::stringstream ss;

	if (fb.is_open()) {
		std::getline(fb, LineBuf);
		ss.str(LineBuf);
		ss >> m_height_file >>
			m_mesh_scale.x >> m_mesh_scale.y >> m_mesh_scale.z >>
			m_texture_scale.x >> m_texture_scale.y;

		for (uint32_t mat = 0; mat < 3; mat++)
		{
			// Get component name
			std::getline(fb, LineBuf);
			ss.clear();
			ss.str(LineBuf);
			
			// Create component TODO: Completely replace with Material Load
			tinyobj::material_t load_mat;
			ss >> load_mat.ambient_texname >>
				load_mat.diffuse_texname >>
				load_mat.specular_texname >>
				load_mat.specular_highlight_texname >>
				load_mat.bump_texname >>
				load_mat.displacement_texname >>
				load_mat.alpha_texname >>
				load_mat.roughness_texname >>
				load_mat.metallic_texname >>
				load_mat.sheen_texname >>
				load_mat.emissive_texname >>
				load_mat.normal_texname;

			materials->push_back(load_mat);
		}
	}
	else
	{
		std::cerr << "ERROR: " << height_map_file << " Failed to open.\n";
	}

	fb.close();

	LoadHeightMapFromImage(m_height_file);
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

void CMultiLayeredHeightmap::Draw(GLuint shader)
{
	glUniform1f(glGetUniformLocation(shader, "Heightmap.render_height"), m_mesh_scale.y);
	glUniform1f(glGetUniformLocation(shader, "Heightmap.max_texture_u"), float(iCols)*m_texture_scale.x);
	glUniform1f(glGetUniformLocation(shader, "Heightmap.max_texture_v"), float(iRows)*m_texture_scale.y);
	glUniform3fv(glGetUniformLocation(shader, "Heightmap.scale_matrix"), 1, glm::value_ptr(m_mesh_scale));

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
	glDeleteVertexArrays(1, &vao);
	bLoaded = false;
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

					vFinalNormals[i][j] = glm::normalize(vFinalNormal); // Store final normal of j-th vertex in i-th row
				}
			}

			std::vector<glm::vec3> vb_pos, vb_norm, vb_col;
			std::vector<glm::vec2> vb_tex;

			for (unsigned int i = 0; i < iRows; i++)
			{
				for (unsigned int j = 0; j < iCols; j++)
				{
					vb_pos.push_back(vVertexData[i][j]); // Add vertex
					vb_tex.push_back(vCoordsData[i][j]); // Add tex. coord
					vb_tex.push_back(vFinalNormals[i][j]);// Add normal
					vb_tex.push_back(glm::vec3(1.0, 0.07, 0.57)); // Add Bright Pink
				}
			}

			// Now create a VBO with heightmap indices
			std::vector<int> indices; 
			int iPrimitiveRestartIndex = iRows*iCols;
			for (unsigned int i = 0; i < iRows; i++)
			{
				for (unsigned int j = 0; j < iCols; j++)
				{
					for (unsigned int k = 0; k < 2; i++)
					{
						int iRow = i + (1 - k);
						int iIndex = iRow*iCols + j;
						indices.push_back(iIndex);
					}
				}
				// Restart triangle strips
				indices.push_back(iPrimitiveRestartIndex);
			}

			// Gen and Bind our VAO
			glGenVertexArrays(1, &m_map.va);
			glBindVertexArray(m_map.va);

			// Generate our VBO's
			glGenBuffers(4, m_map.vb);

			// Bind Vertex Buffer Object
			glBindBuffer(GL_ARRAY_BUFFER, m_map.vb[0]);
			glBufferData(GL_ARRAY_BUFFER, vb_pos.size() * sizeof(glm::vec3), vb_pos.data(), GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

			// Bind Normal Buffer Object
			glBindBuffer(GL_ARRAY_BUFFER, m_map.vb[1]);
			glBufferData(GL_ARRAY_BUFFER, vb_norm.size() * sizeof(glm::vec3), vb_norm.data(), GL_STATIC_DRAW);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

			// Bind Color Buffer Object
			glBindBuffer(GL_ARRAY_BUFFER, m_map.vb[2]);
			glBufferData(GL_ARRAY_BUFFER, vb_col.size() * sizeof(glm::vec3), vb_col.data(), GL_STATIC_DRAW);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

			// Bind UV Buffer Object
			glBindBuffer(GL_ARRAY_BUFFER, m_map.vb[3]);
			glBufferData(GL_ARRAY_BUFFER, vb_tex.size() * sizeof(glm::vec2), vb_tex.data(), GL_STATIC_DRAW);
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

			// And now attach index data to this VAO
			// Here don't forget to bind another type of VBO - the element array buffer, or simplier indices to vertices
			glGenBuffers(1, &m_map.idx);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_map.idx);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

			// Unbind Buffers
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			bLoaded = true; // If get here, we succeeded with generating heightmap
			return true;
		}
	}
}