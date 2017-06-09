#include "../include/Heightmap.h"

#include <fstream>
#include <sstream>
#include <cmath>
#include "../include/stb_image.h"

Heightmap::Heightmap(std::string name, std::string height_map_file, loadedComponents* scene_tracker)
{
	std::cerr << "\tName: " << name;
	std::cerr << " (" << height_map_file << ")\n";

	this->scene_tracker = scene_tracker;
	m_name = name;

	materials = new std::vector<tinyobj::material_t>;


	std::ifstream fb; // FileBuffer
	fb.open(height_map_file.c_str(), std::ios::in);
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
		std::cerr << "Error: " << strerror(errno) << std::endl;
		fb.close();
		return;
	}
	fb.close();
	std::cout << "\tHeight Map Image: " << m_height_file << std::endl;
	std::cout << "\tMaterial File: " << material_file << std::endl;
	std::cout << "\tMesh Scale: " << m_mesh_scale.x  << ":" << m_mesh_scale.y << ":" << m_mesh_scale.z << std::endl;
	std::cout << "\tTexture Scale: " << m_texture_scale.x << ":" << m_texture_scale.y << std::endl;

	std::cerr << "\tLoading Materials" << std::endl;

	// Load Heightmap Materials
	std::map<std::string, int>* material_map = new std::map<std::string, int>;
	std::ifstream mat_fb;
	mat_fb.open((material_file), std::ios::in);
	std::string warn;
	if(mat_fb.is_open())
	{
		tinyobj::LoadMtl(material_map, materials, static_cast<std::istream*>(&mat_fb), &warn);

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

	delete material_map;


	// Add a default material. Set default texture
	materials->push_back(tinyobj::material_t());
	materials->at(materials->size() - 1).diffuse_texname = "_default.png";

	std::cerr << "\tLoading Map from Image" << std::endl;
	LoadHeightMapFromImage(m_height_file);

	std::cerr << "\tSetting up textures" << std::endl;
	setupTextures("./Materials/");

	std::cerr << "\tBuilding transform" << std::endl;
	build_transform();

	std::cerr << "\tLoad Interpolation texture" << std::endl;
	loadTexture("./Materials/", m_height_file);
}

Heightmap::~Heightmap()
{
	ReleaseHeightmap();

	delete materials;
	stbi_image_free(map_image);
}

void Heightmap::SetRenderSize(float fRenderX, float fHeight, float fRenderZ)
{
	m_mesh_scale = glm::vec3(fRenderX, fHeight, fRenderZ);
}

void Heightmap::SetRenderSize(float fQuadSize, float fHeight)
{
	m_mesh_scale = glm::vec3(float(iCols)*fQuadSize, fHeight, float(iRows)*fQuadSize);
}

float Heightmap::GetFloor(glm::vec3 location)
{
	/* I've reverse engineered the x and y coords (I think!)
	 * Currently we just go to the nearest point and use that values y
	 * this will probably lead to very choppy movement over low rez terrain
	 * ALSO for some reason our x and z values are reversed :S
	*/

	uint32_t base_height = get_image_value(location.z, location.x, 0);

	return (-1 + 2 * (base_height / IMAGE_DEPTH)) * m_mesh_scale.y;
}

void Heightmap::draw(GLuint shader)
{
	glUniform1i(glGetUniformLocation(shader, "is_heightmap"), 1);

	// Mesh Uniforms
	GLuint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m_transform));

	GLuint componentLoc = glGetUniformLocation(shader, "component");
	glUniformMatrix4fv(componentLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4()));

	GLuint objectLoc = glGetUniformLocation(shader, "object");
	glUniformMatrix4fv(objectLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4()));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scene_tracker->Textures->at(m_height_file).first);
	glUniform1i(glGetUniformLocation(shader, "heightmap"), 0);

	glm::vec2 shifted_scale = glm::vec2(m_mesh_scale.x, m_mesh_scale.z);
	glUniform2fv(glGetUniformLocation(shader, "heightmap_scale"), 1, glm::value_ptr(shifted_scale));

	for(uint32_t mat_idx = 0; mat_idx < materials->size(); mat_idx++)
	{
		// -- Texture Uniforms --
		std::string loc_str = "material[" + std::to_string(mat_idx) + "]";

		// std::cout << "Loading " << loc_str << " : " << materials->at(mat_idx).name << std::endl;
		// Load diffuse texture
		glActiveTexture(GL_TEXTURE0+(mat_idx * TEXTURE_MAPS)+1);

		std::string diffuse_texname = materials->at(mat_idx).diffuse_texname;

		if (materials->at(mat_idx).diffuse_texname.length() > 0 && scene_tracker->Textures->find(diffuse_texname) != scene_tracker->Textures->end()) {
			glBindTexture(GL_TEXTURE_2D, scene_tracker->Textures->at(diffuse_texname).first);
			GLuint dif = glGetUniformLocation(shader, (loc_str + ".diffuse").c_str());

			glUniform1i(dif, (mat_idx * TEXTURE_MAPS)+1);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, scene_tracker->Textures->at("_default.png").first);
			GLuint dif = glGetUniformLocation(shader, (loc_str + ".diffuse").c_str());

			glUniform1i(dif, (mat_idx * TEXTURE_MAPS)+1);
		}

		// Load specular texture
		glActiveTexture(GL_TEXTURE0+ (mat_idx * TEXTURE_MAPS) + 2);
		std::string specular_texname = materials->at(mat_idx).specular_texname;

		if (materials->at(mat_idx).specular_texname.length() > 0 && scene_tracker->Textures->find(specular_texname) != scene_tracker->Textures->end()) {
			glBindTexture(GL_TEXTURE_2D, scene_tracker->Textures->at(specular_texname).first);
			GLuint spec = glGetUniformLocation(shader, (loc_str + ".specular").c_str());

			glUniform1i(spec,  (mat_idx * TEXTURE_MAPS) + 2);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, scene_tracker->Textures->at("_default.png").first);
			GLuint spec = glGetUniformLocation(shader, (loc_str + ".specular").c_str());

			glUniform1i(spec, (mat_idx * TEXTURE_MAPS) + 2);
		}

		// Load specular texture
		glActiveTexture(GL_TEXTURE0 + (mat_idx * TEXTURE_MAPS) + 3);
		std::string normal_texname = materials->at(mat_idx).normal_texname;

		if (materials->at(mat_idx).normal_texname.length() > 0 && scene_tracker->Textures->find(normal_texname) != scene_tracker->Textures->end()) {
			glBindTexture(GL_TEXTURE_2D, scene_tracker->Textures->at(normal_texname).first);
			GLuint spec = glGetUniformLocation(shader, (loc_str + ".normal").c_str());

			glUniform1i(spec, (mat_idx * TEXTURE_MAPS) + 3);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, scene_tracker->Textures->at("_default.png").first);
			GLuint spec = glGetUniformLocation(shader, (loc_str + ".normal").c_str());

			glUniform1i(spec, (mat_idx * TEXTURE_MAPS) + 3);
		}

		// -- Material Uniforms --
		;
		GLuint diffuseColor = glGetUniformLocation(shader, (loc_str+ ".diffuse_color").c_str());
		glUniform3fv(diffuseColor, 1, materials->at(mat_idx).diffuse);

		GLuint shininess = glGetUniformLocation(shader, (loc_str + ".shininess").c_str());
		glUniform1f(shininess, materials->at(mat_idx).shininess);

		GLuint loaded = glGetUniformLocation(shader, (loc_str + ".loaded").c_str());
		glUniform1f(loaded, true);
	}

	// Now we're ready to render - we are drawing set of triangle strips using one call, but we g otta enable primitive restart
	glBindVertexArray(m_map.va);

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(iRows*iCols);

	glDrawElements(GL_TRIANGLE_STRIP, indices_count, GL_UNSIGNED_INT, 0);


	glDisable(GL_PRIMITIVE_RESTART);
	// Unload our textures;
	for (uint32_t mat_idx = 0; mat_idx < materials->size(); mat_idx++)
	{
		std::string loc_str = "material[" + std::to_string(mat_idx) + "]";
		GLuint loaded = glGetUniformLocation(shader, (loc_str + ".loaded").c_str());
		glUniform1f(loaded, false);

	}

	glUniform1i(glGetUniformLocation(shader, "is_heightmap"), 0);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Heightmap::ReleaseHeightmap()
{
	if (!bLoaded)
		return; // Heightmap must be loaded
	glDeleteVertexArrays(1, &m_map.va);
	bLoaded = false;
}

uint32_t Heightmap::GetNumHeightmapRows()
{
	return iRows;
}

uint32_t Heightmap::GetNumHeightmapCols()
{
	return iCols;
}

bool Heightmap::LoadHeightMapFromImage(std::string sImagePath)
{
	if (bLoaded)
	{
		bLoaded = false;
		ReleaseHeightmap();
	}

	int ix, iz, ic;
	map_image = stbi_load(sImagePath.c_str(), &ix, &iz, &ic, STBI_default);
	if (!map_image) {
		std::cerr << "Unable to load texture: " << sImagePath << std::endl;
		exit(1);
	}

	iCols = ix;
	iRows = iz;
	composition = ic;
	uint32_t img_size = ix * iz;

	std::cout << "\tMap Parameters\tX: " << iCols << "\tY: " << iRows << "\tDepth: " << composition << std::endl;
	// We also require our image to be either 24-bit (classic RGB) or 8-bit (luminance)
	//if (image == NULL || iRows == 0 || iCols == 0 || (composition != 3 && composition != 1))
	//	return false;

	// How much to increase data pointer to get to next pixel data
	unsigned int ptr_inc = composition;
	// Length of one row in data
	unsigned int row_step = ptr_inc*iCols;

	// All vertex data are here (there are iRows*iCols vertices in this heightmap), we will get to normals later
	std::vector< glm::vec3>* vb_pos = new std::vector< glm::vec3>;
	std::vector<glm::vec3>* vb_norm = new std::vector<glm::vec3>;
	std::vector<glm::vec3>* vb_col = new std::vector<glm::vec3>;

	std::vector< glm::vec2>* vb_tex = new std::vector< glm::vec2>;

	std::cerr << "\tBuild Heightmap Vertices" << std::endl;

	for (unsigned int row_idx = 0; row_idx < iRows; row_idx++)
	{
		for (unsigned int col_idx = 0; col_idx < iCols; col_idx++)
		{
			uint32_t row = row_step * row_idx;
			uint32_t col = ptr_inc * col_idx;

			// all points will be between -1 and 1, we will then rescale based on m_mesh_scale
			float x = -1 + 2*  (float(col_idx) / (iCols-1));
			float z = -1 + 2 * (float(row_idx) / (iRows - 1));

			// We work with Y being up
			float y = -1 + 2 * (*(map_image + row + col) / IMAGE_DEPTH); // Normalise our colour, then offset
			// std::cout << "(" << x << ", " << y << ", " << z << ") ";
			// Mesh scaling is applied via transform matrix
			vb_pos->push_back(glm::vec3(x, y, z));

			// Set up Base Colour (gonna default to bright pink)
			vb_col->push_back(glm::vec3(1.0, 0.07, 0.57));

			// We make texture_scale account for what % of the map should be covered by a single texture map_image. TODO Check that is right!
			vb_tex->push_back(glm::vec2((x+1)/2*m_texture_scale.x, (z+1)/2*m_texture_scale.y));

			// Set up Normals
			// Using very interesting code http://www.flipcode.com/archives/Calculating_Vertex_Normals_for_Height_Maps.shtml
			// Explanation of process https://stackoverflow.com/a/34644939/7880704
			uint32_t col_next = (col_idx < iCols - 1	? col_idx + 1 : col_idx) * ptr_inc;
			uint32_t col_last = (col_idx > 0			? col_idx - 1 : col_idx) * ptr_inc;

			float dxdz = (*(map_image + row + col_next) - *(map_image + row + col_last));
			if (col_idx == 0 || col_idx == iCols - 1)
				dxdz *= 2;


			uint32_t row_next = (row_idx < iRows - 1	? row_idx + 1 : row_idx) * row_step;
			uint32_t row_last = (row_idx > 0			? row_idx - 1 : row_idx) * row_step;

			float dydz = *(map_image + row_next + col) - *(map_image + row_last + col);
			if (row_idx == 0 || row_idx == iRows - 1)
				dydz *= 2;
			
			vb_norm->push_back(glm::normalize(glm::vec3(-dxdz*m_mesh_scale.x, NORMAL_UP_DEPTH*m_mesh_scale.y, -dydz*m_mesh_scale.z)));
		}
		// std::cout << std::endl;
	}
	std::cout << vb_pos->size() << " points generated\n" << std::endl;

	std::cout << "Generate Heightmap indices\n" << std::endl;
	// Now create a VBO with heightmap indices
	std::vector<GLuint>* indices = new std::vector<GLuint>;
	int iPrimitiveRestartIndex = iRows*iCols;
	for (unsigned int row_idx = 0; row_idx < iRows-1; row_idx++)
	{
		for (unsigned int col_idx = 0; col_idx < iCols; col_idx++)
		{
			unsigned int tris[2] = {
				(col_idx + row_idx* iCols),			// Current Point
				(col_idx + (row_idx+1)*iCols)		// Point on line below
			};
			indices->insert(indices->end(), tris, std::end(tris));
			// std::cout << tris[0] << " " << tris[1] << " ";
		}
		// Restart triangle strips
		if (row_idx < iRows - 2)
		{
			indices->push_back(iPrimitiveRestartIndex);
			// std::cout << iPrimitiveRestartIndex;
		}
		// std::cout << std::endl;
	}

	indices_count = indices->size();

	std::cout << indices->size() << " indices generated\n" << std::endl;

	std::cout << "Generate Tangents and Bitangents" << std::endl;

	std::vector<glm::vec3>* vb_tan = new std::vector<glm::vec3>;
	std::vector<glm::vec3>* vb_bitan = new std::vector<glm::vec3>;
	vb_tan->resize(vb_pos->size());
	vb_bitan->resize(vb_pos->size());

	for (uint32_t idx = 0; idx < indices->size()-(iCols); idx += 2)
	{
		GLuint current = indices->at(idx);
		GLuint below = indices->at(idx+1);
		GLuint across = indices->at(idx+2);
		GLuint across_below = indices->at(idx+3);

		if (current == iPrimitiveRestartIndex || across == iPrimitiveRestartIndex)
		{
			idx += 1; // Shift back one to compensate for restart value
			continue;
		}

		// std::cout << current << " : " << below << " : " << across << " : " << across_below << "\n";

		// Prep our values
		glm::vec3 tangent, bitangent, edge1, edge2;
		glm::vec2 deltaUV1, deltaUV2;
		GLfloat scalar;

		// Triangle 1 (Upper)
		edge1 = vb_pos->at(below) - vb_pos->at(current);
		edge2 = vb_pos->at(across) - vb_pos->at(current);
		deltaUV1 = vb_tex->at(below) - vb_tex->at(current);
		deltaUV2 = vb_tex->at(across) - vb_tex->at(current);

		scalar = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		tangent.x = scalar * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = scalar * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = scalar * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent = glm::normalize(tangent);

		bitangent.x = scalar * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent.y = scalar * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent.z = scalar * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent = glm::normalize(bitangent);

		vb_tan->at(current) = tangent;
		vb_bitan->at(current) = bitangent;
		vb_tan->at(below) = tangent;
		vb_bitan->at(below) = bitangent;

		// Triangle 2 (Lower)
		edge1 = vb_pos->at(below) - vb_pos->at(across_below);
		edge2 = vb_pos->at(across) - vb_pos->at(across_below);
		deltaUV1 = vb_tex->at(below) - vb_tex->at(across_below);
		deltaUV2 = vb_tex->at(across) - vb_tex->at(across_below);

		scalar = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		tangent.x = scalar * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = scalar * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = scalar * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent = glm::normalize(tangent);

		bitangent.x = scalar * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent.y = scalar * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent.z = scalar * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent = glm::normalize(bitangent);

		vb_tan->at(across) = tangent;
		vb_bitan->at(across) = bitangent;
		vb_tan->at(across_below) = tangent;
		vb_bitan->at(across_below) = bitangent;
	}


	std::cout << "Bind Array Objects\n" << std::endl;

	// Gen and Bind our VAO
	glGenVertexArrays(1, &m_map.va);
	glBindVertexArray(m_map.va);

	// Generate our VBO's
	glGenBuffers(6, m_map.vb);

	// Bind Vertex Buffer Object
	glBindBuffer(GL_ARRAY_BUFFER, m_map.vb[0]);
	glBufferData(GL_ARRAY_BUFFER, vb_pos->size() * sizeof(glm::vec3), vb_pos->data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Bind Normal Buffer Object
	glBindBuffer(GL_ARRAY_BUFFER, m_map.vb[1]);
	glBufferData(GL_ARRAY_BUFFER, vb_norm->size() * sizeof(glm::vec3), vb_norm->data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Bind Color Buffer Object
	glBindBuffer(GL_ARRAY_BUFFER, m_map.vb[2]);
	glBufferData(GL_ARRAY_BUFFER, vb_col->size() * sizeof(glm::vec3), vb_col->data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Bind UV Buffer Object
	glBindBuffer(GL_ARRAY_BUFFER, m_map.vb[3]);
	glBufferData(GL_ARRAY_BUFFER, vb_tex->size() * sizeof(glm::vec2), vb_tex->data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Bind Tangent Buffer Object
	glBindBuffer(GL_ARRAY_BUFFER, m_map.vb[4]);
	glBufferData(GL_ARRAY_BUFFER, vb_tan->size() * sizeof(glm::vec3), vb_tan->data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Bind Bitangent Buffer Object
	glBindBuffer(GL_ARRAY_BUFFER, m_map.vb[5]);
	glBufferData(GL_ARRAY_BUFFER, vb_bitan->size() * sizeof(glm::vec3), vb_bitan->data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// And now attach index data to this VAO
	// Here don't forget to bind another type of VBO - the element array buffer, or simplier indices to vertices
	glGenBuffers(1, &m_map.idx);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_map.idx);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(GLuint), indices->data(), GL_STATIC_DRAW);

	// Unbind Buffers
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	bLoaded = true; // If get here, we succeeded with generating heightmap

	delete vb_pos;
	delete vb_col;
	delete vb_norm;
	delete vb_tex;
	delete vb_tan;
	delete vb_bitan;

	return true;

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
	std::cerr << "\tLoading Texture: " << texture_name << std::endl;
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

		// Unbind
		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(image);

		scene_tracker->Textures->insert(std::make_pair(texture_name, std::make_pair(texture_id, 1)));
	}
	else {
		scene_tracker->Textures->at(texture_name).second++;
	}
}

glm::vec3 calculate_surface_normal(glm::vec3 const vertex_1, glm::vec3 const vertex_2, glm::vec3 const vertex_3)
{
	glm::vec3 normal;
	// https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
	glm::vec3 vector_1 = glm::vec3(vertex_2.x - vertex_1.x, vertex_2.y - vertex_1.y, vertex_2.z - vertex_1.z);
	glm::vec3 vector_2 = glm::vec3(vertex_3.x - vertex_1.x, vertex_3.y - vertex_1.y, vertex_3.z - vertex_1.z);

	glm::vec3 cross = glm::cross(vector_1, vector_2);
	cross = glm::normalize(cross);

	normal.x = cross.x;
	normal.y = cross.y;
	normal.z = cross.z;

	return normal;
}

unsigned char Heightmap::get_image_value(GLfloat x, GLfloat z, uint8_t channel)
{
	int local_x = (int) roundf((((x / m_mesh_scale.x) + 1) / 2) * (iCols - 1));

	int local_z = (int) roundf((((z / m_mesh_scale.z) + 1) / 2) * (iRows - 1));

	if ((local_x < static_cast<int>(iRows) && local_x >= 0)
        && (local_z < static_cast<int>(iCols) && local_z >= 0)
        && (channel < composition && channel >= 0))
	{
		// Value = *(MP + ROW: r_idx * comp * cols + COL: c_idx * comp + channel)
		int value = *(map_image + (local_x * iCols * composition) + (local_z * composition) + channel);
		// std::cout << "(" << local_x << ", " << local_z << ") = " << value << std::endl;
		return value;
	}

	return 0;
}

void Heightmap::build_transform()
{
	m_transform = glm::mat4();
	m_transform = glm::scale(m_transform, m_mesh_scale);
}
