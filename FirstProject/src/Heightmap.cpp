#include "../include/Heightmap.h"

#include <fstream>
#include <sstream>
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
}

Heightmap::~Heightmap()
{
	ReleaseHeightmap();

	delete materials;
	stbi_image_free(map_image);
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

void Heightmap::draw(GLuint shader)
{
	// glUniform1i(glGetUniformLocation(shader, "heightmap_enabled"), 1);
	// glUniform1f(glGetUniformLocation(shader, "Heightmap.render_height"), m_mesh_scale.y);
	// glUniform1f(glGetUniformLocation(shader, "Heightmap.max_texture_u"), float(iCols)*m_texture_scale.x);
	// glUniform1f(glGetUniformLocation(shader, "Heightmap.max_texture_v"), float(iRows)*m_texture_scale.y);
	// glUniform3fv(glGetUniformLocation(shader, "Heightmap.scale_matrix"), 1, glm::value_ptr(m_mesh_scale));

	// Mesh Uniforms
	GLuint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m_transform));

	GLuint componentLoc = glGetUniformLocation(shader, "component");
	glUniformMatrix4fv(componentLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4()));

	GLuint objectLoc = glGetUniformLocation(shader, "object");
	glUniformMatrix4fv(objectLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4()));

	
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

	glDrawElements(GL_TRIANGLE_STRIP, indices_count, GL_UNSIGNED_INT, 0);

	glDisable(GL_PRIMITIVE_RESTART);
	glBindVertexArray(0);
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

uint32_t Heightmap::GetNumHeightmapRows()
{
	return iRows;
}

uint32_t Heightmap::GetNumHeightmapCols()
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

	int x, y, c;
	map_image = stbi_load(sImagePath.c_str(), &x, &y, &c, STBI_default);
	if (!map_image) {
		std::cerr << "Unable to load texture: " << sImagePath << std::endl;
		exit(1);
	}

	iRows = x;
	iCols = y;
	composition = c;
	uint32_t img_size = x * y;

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

	for (unsigned int i = 0; i < iRows; i++)
	{
		for (unsigned int j = 0; j < iCols; j++)
		{	
			uint32_t row = row_step * i;
			uint32_t col = ptr_inc * j;

			// all points will be between -1 and 1, we will then rescale based on m_mesh_scale
			float x = -1 + 2*(float(j) / (iCols-1));
			float z = -1 + 2 * (float(i) / (iRows - 1));

			// We work with Y being up
			float y = -1 + 2 * (*(map_image + row + col) / 255.0f); // Normalise our colour, then offset
			// std::cout << "(" << x << ", " << y << ", " << z << ") ";
			// Mesh scaling is applied via transform matrix
			vb_pos->push_back(glm::vec3(x, y, z));

			// We make texture_scale account for what % of the map should be covered by a single texture map_image. TODO Check that is right!
			vb_tex->push_back(glm::vec2(x*m_texture_scale.x, y*m_texture_scale.y));

			// Set up Normals
			// Using very interesting code http://www.flipcode.com/archives/Calculating_Vertex_Normals_for_Height_Maps.shtml
			uint32_t col_next = j < iCols - 1 ? j + 1 : j;
			uint32_t col_last = j > 0 ? j - 1 : j;

			col_next *= ptr_inc;
			col_last *= ptr_inc;

			float sx = *(map_image + row + col_next) - *(map_image + row + col_last);
			if (j == 0 || j == iCols - 1)
				sx *= 2;

			uint32_t row_next = i < iRows - 1 ? i + 1 : i;
			uint32_t row_last = i > 0 ? i - 1 : i;

			row_next *= row_step;
			row_last *= row_step;

			float sy = *(map_image + row_next + col) - *(map_image + row_last + col);
			if (i == 0 || i == iRows - 1)
				sy *= 2;

			vb_norm->push_back(glm::normalize(glm::vec3(-sx*m_mesh_scale.y, 2*m_mesh_scale.x, sy*m_mesh_scale.x*m_mesh_scale.y/m_mesh_scale.z)));

			// Set up Base Colour (gonna default to bright pink)
			vb_col->push_back(glm::vec3(1.0, 0.07, 0.57));
		}
		// std::cout << std::endl;
	}

	std::cout << vb_pos->size() << " points generated\n" << std::endl;

	std::cout << "Generate Heightmap indices\n" << std::endl;
	// Now create a VBO with heightmap indices
	std::vector<GLuint>* indices = new std::vector<GLuint>;
	int iPrimitiveRestartIndex = iRows*iCols;
	for (unsigned int i = 0; i < iRows-1; i++)
	{
		for (unsigned int j = 0; j < iCols; j++)
		{
			unsigned int tris[2] = { 
				(j + i* iCols),			// Current Point
				(j + (i+1)*iCols)		// Point on line below
			};
			indices->insert(indices->end(), tris, std::end(tris));
			// std::cout << tris[0] << " " << tris[1] << " ";
		}
		// Restart triangle strips
		if (i < iRows - 2)
		{
			indices->push_back(iPrimitiveRestartIndex);
			// std::cout << iPrimitiveRestartIndex;
		}
		// std::cout << std::endl;
	}

	indices_count = indices->size();

	std::cout << indices->size() << " indices generated\n" << std::endl;

	std::cout << "Bind Array Objects\n" << std::endl;

	// Gen and Bind our VAO
	glGenVertexArrays(1, &m_map.va);
	glBindVertexArray(m_map.va);

	// Generate our VBO's
	glGenBuffers(4, m_map.vb);

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

char Heightmap::get_image_value(uint32_t x, uint32_t y, uint8_t channel)
{
	if(x >= iRows || y >= iCols || channel >= composition)
		return 0;

	return *(map_image + channel + x*(iCols * composition) + y * composition);
}

void Heightmap::build_transform()
{
	m_transform = glm::mat4();
	m_transform = glm::scale(m_transform, m_mesh_scale);
}
