#include "../include/Heightmap.h"

#include <fstream>
#include <sstream>
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

Heightmap::Heightmap(std::string name, std::string height_map_file, loadedComponents* scene_tracker)
{
	m_name = name;
	this->scene_tracker = scene_tracker;
	materials = new std::vector<tinyobj::material_t>;
	std::ifstream fb; // FileBuffer
	fb.open((height_map_file), std::ios::in);
	std::string LineBuf, material_file;
	std::stringstream ss;

	if (fb.is_open()) {
		std::getline(fb, LineBuf);
		ss.str(LineBuf);

		ss >> m_height_file >> material_file >>
			m_mesh_scale.x >> m_mesh_scale.y >> m_mesh_scale.z >>
			m_texture_scale.x >> m_texture_scale.y;

	}
	else
	{
		std::cerr << "ERROR: " << height_map_file << " Failed to open.\n";
	}
	fb.close();

	// Load Heightmap Materials	
	std::map<std::string, int> material_map;
	std::ifstream mat_fb;
	mat_fb.open((material_file), std::ios::in);	
	std::string warn;
	if(mat_fb.is_open()) 
	{
		tinyobj::LoadMtl(&material_map, materials, static_cast<std::istream*>(&mat_fb), warn);
	
		if(!warn.empty())
		{
			std::cout << "ERROR: Problem loading: " << LineBuf <<
				"\n" << warn << std::endl;
		}
	}
	else
	{
		std::cerr << "ERROR: " << material_file << " Failed to open.\n";
	}

	mat_fb.close();

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

---------------------------------------------*/

void Heightmap::SetRenderSize(float fRenderX, float fHeight, float fRenderZ)
{
	m_mesh_scale = glm::vec3(fRenderX, fHeight, fRenderZ);
}

void Heightmap::SetRenderSize(float fQuadSize, float fHeight)
{
	m_mesh_scale = glm::vec3(float(iCols)*fQuadSize, fHeight, float(iRows)*fQuadSize);
}

/*-----------------------------------------------

Name:	RenderHeightmap

Params:	none

Result: Guess what it does :)

---------------------------------------------*/

void Heightmap::Draw(GLuint shader)
{
	glUniform1f(glGetUniformLocation(shader, "Heightmap.render_height"), m_mesh_scale.y);
	glUniform1f(glGetUniformLocation(shader, "Heightmap.max_texture_u"), float(iCols)*m_texture_scale.x);
	glUniform1f(glGetUniformLocation(shader, "Heightmap.max_texture_v"), float(iRows)*m_texture_scale.y);
	glUniform3fv(glGetUniformLocation(shader, "Heightmap.scale_matrix"), 1, glm::value_ptr(m_mesh_scale));

	glUniform1i(glGetUniformLocation(shader, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(shader, "material.specular"), 1);

	for(uint32_t mat_idx = 0; mat_idx < materials->size(); mat_idx++)
	{
		// -- Texture Uniforms --
	
		// Load diffuse texture
		glActiveTexture(GL_TEXTURE0+mat_idx);
		std::string diffuse_texname = materials->at(mat_idx).diffuse_texname;
		if (scene_tracker->Textures->find(diffuse_texname) != scene_tracker->Textures->end()) {
			glBindTexture(GL_TEXTURE_2D, scene_tracker->Textures->at(diffuse_texname).first);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, scene_tracker->Textures->at("_default.png").first);
		}

		// Load specular texture
		glActiveTexture(GL_TEXTURE1+mat_idx);
		std::string specular_texname = materials->at(mat_idx).specular_texname;
		if (scene_tracker->Textures->find(specular_texname) != scene_tracker->Textures->end()) {
			glBindTexture(GL_TEXTURE_2D, scene_tracker->Textures->at(specular_texname).first);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, scene_tracker->Textures->at("_default.png").first);
		}

		// -- Material Uniforms -- 
		std::string loc_str = "material["+ std::to_string(mat_idx) +"].diffuse_color";
		GLuint diffuseColor = glGetUniformLocation(shader, loc_str.c_str());
		glUniform3fv(diffuseColor, 1, materials->at(mat_idx).diffuse);

		loc_str = "material["+ std::to_string(mat_idx) + "].shininess";
		GLuint shininess = glGetUniformLocation(shader, loc_str.c_str());
		glUniform1f(shininess, materials->at(mat_idx).shininess);
	}
	// Now we're ready to render - we are drawing set of triangle strips using one call, but we g otta enable primitive restart
	glBindVertexArray(m_map.va);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(iRows*iCols);

	int iNumIndices = (iRows - 1)*iCols * 2 + iRows - 1;
	glDrawElements(GL_TRIANGLE_STRIP, iNumIndices, GL_UNSIGNED_INT, 0);
}

/*-----------------------------------------------

Name:	ReleaseHeightmap

Params:	none

Result: Releases all data of one heightmap instance.

---------------------------------------------*/

void Heightmap::ReleaseHeightmap()
{
	if (!bLoaded)
		return; // Heightmap must be loaded
	glDeleteVertexArrays(1, &m_map.va);
	bLoaded = false;
}

/*-----------------------------------------------

Name:	Getters

Params:	none

Result:	They get something :)

---------------------------------------------*/

int Heightmap::GetNumHeightmapRows()
{
	return iRows;
}

int Heightmap::GetNumHeightmapCols()
{
	return iCols;
}

/*-----------------------------------------------

Name:	LoadHeightMapFromImage

Params:	sImagePath - path to the (optimally) grayscale
image containing heightmap data.

Result: Loads a heightmap and builds up all OpenGL
structures for rendering.

---------------------------------------------*/

bool Heightmap::LoadHeightMapFromImage(std::string sImagePath)
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
					vb_norm.push_back(vFinalNormals[i][j]);// Add normal
					vb_col.push_back(glm::vec3(1.0, 0.07, 0.57)); // Add Bright Pink
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
	return false;
}

void Heightmap::setupTextures(std::string base_dir)
{
	for (size_t m = 0; m < materials->size(); m++) {
		tinyobj::material_t* mp = &materials->at(m);
					                                
		if (mp->ambient_texname.length() > 0)
			loadTexture(base_dir, mp->ambient_texname);
		if (mp->diffuse_texname.length() > 0)
		        loadTexture(base_dir, mp->diffuse_texname);
		if (mp->specular_texname.length() > 0)
		        loadTexture(base_dir, mp->specular_texname);
		if (mp->specular_highlight_texname.length() > 0)
			loadTexture(base_dir, mp->specular_highlight_texname);
		if (mp->bump_texname.length() > 0)
		        loadTexture(base_dir, mp->bump_texname);
		if (mp->displacement_texname.length() > 0)
			loadTexture(base_dir, mp->displacement_texname);
		if (mp->alpha_texname.length() > 0)
			loadTexture(base_dir, mp->alpha_texname);
		if (mp->roughness_texname.length() > 0)
			loadTexture(base_dir, mp->roughness_texname);
		if (mp->metallic_texname.length() > 0)
			loadTexture(base_dir, mp->metallic_texname);
		if (mp->sheen_texname.length() > 0)
			loadTexture(base_dir, mp->sheen_texname);
		if (mp->emissive_texname.length() > 0)
			loadTexture(base_dir, mp->emissive_texname);
		if (mp->normal_texname.length() > 0)
			loadTexture(base_dir, mp->normal_texname);
	}
}

void Heightmap::loadTexture(std::string base_dir, std::string texture_name)
{
	// Only load the texture if it is not already loaded
	if (scene_tracker->Textures->find(texture_name) == scene_tracker->Textures->end()) {
		GLuint texture_id;
		int w, h;
		int comp;

		std::string texture_filename = texture_name;

		if (!FileExists(texture_filename)) {

			// If desired, grab the default material (from it's default location)
			if (texture_filename == "_default.png")
				texture_filename = "./Materials/" + texture_name;
			else // Append base dir.
				texture_filename = base_dir + texture_name;

			if (!FileExists(texture_filename)) {
				std::cerr << "Unable to find file: " << texture_name << std::endl;
				exit(1);
			}
		}

		unsigned char* image = stbi_load(texture_filename.c_str(), &w, &h, &comp, STBI_default);
		if (!image) {
			std::cerr << "Unable to load texture: " << texture_filename << std::endl;
			exit(1);
		}

		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		if (comp == 3) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		}
		else if (comp == 4) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(image);

		scene_tracker->Textures->insert(std::make_pair(texture_name, std::make_pair(texture_id, 1)));
	}
	else {
		scene_tracker->Textures->at(texture_name).second++;
	}
}
