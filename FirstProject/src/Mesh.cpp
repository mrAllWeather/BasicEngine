#include "../include/Mesh.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

Mesh::Mesh(std::string filename, loadedComponents* scene_tracker, std::string base_dir)
{
	this->name = filename;
	this->scene_tracker = scene_tracker;

	std::string err;

	tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str(), base_dir.c_str());

	if (!err.empty())
	{
		std::cerr << "Encountered error loading file: " << filename << "\n" << err << std::endl;
		std::cerr << "Skipping.\n";
		loaded_successfully = false;
		return;
	}

	bounding_maximum = glm::vec3(-std::numeric_limits<float>::max());
	bounding_minimum = glm::vec3(std::numeric_limits<float>::max());

	// Add a default material. Set default texture
	materials.push_back(tinyobj::material_t());
	materials.at(materials.size() - 1).diffuse_texname = "_default.png";
	materials.push_back(tinyobj::material_t());
	materials.at(materials.size() - 1).diffuse_texname = "_default_black.png";

	setupMesh();
	setupTextures(base_dir);
	generateTransform();
	compute_bounds();
	std::cerr << "Mesh Loaded: " << filename << "\n" << std::endl;
	std::cerr << "Mesh Bounds: (" << m_lower_bounds.x << ", " << m_lower_bounds.y << ", " << m_lower_bounds.z << ") - (" << m_upper_bounds.x << ", " << m_upper_bounds.y << ", " << m_upper_bounds.z << ")\n" << std::endl;
}

Mesh::~Mesh()
{
}

void Mesh::draw(GLuint shader)
{

	for (const auto object : objects)
	{
		if ((object.material_id < materials.size())) {
			// -- Texture Uniforms --
			std::string loc_str = "material[0]";

			// Load diffuse texture
			glActiveTexture(GL_TEXTURE0);
			std::string diffuse_texname = materials.at(object.material_id).diffuse_texname;
			if (scene_tracker->Textures->find(diffuse_texname) != scene_tracker->Textures->end()) {
				glBindTexture(GL_TEXTURE_2D, scene_tracker->Textures->at(diffuse_texname).first);
				glUniform1i(glGetUniformLocation(shader, (loc_str + ".diffuse").c_str()), 0);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, scene_tracker->Textures->at("_default.png").first);
				glUniform1i(glGetUniformLocation(shader, (loc_str + ".diffuse").c_str()), 0);
			}

			// Load specular texture
			glActiveTexture(GL_TEXTURE1);
			std::string specular_texname = materials.at(object.material_id).specular_texname;
			if (scene_tracker->Textures->find(specular_texname) != scene_tracker->Textures->end()) {
				glBindTexture(GL_TEXTURE_2D, scene_tracker->Textures->at(specular_texname).first);
				glUniform1i(glGetUniformLocation(shader, (loc_str + ".specular").c_str()), 1);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, scene_tracker->Textures->at("_default.png").first);
				glUniform1i(glGetUniformLocation(shader, (loc_str + ".specular").c_str()), 1);
			}

			// Load normal texture
			glActiveTexture(GL_TEXTURE2);
			std::string normal_texname = materials.at(object.material_id).normal_texname;
			if (scene_tracker->Textures->find(normal_texname) != scene_tracker->Textures->end()) {
				glBindTexture(GL_TEXTURE_2D, scene_tracker->Textures->at(normal_texname).first);
				glUniform1i(glGetUniformLocation(shader, (loc_str + ".normal").c_str()), 2);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, scene_tracker->Textures->at("_default_black.png").first);
				glUniform1i(glGetUniformLocation(shader, (loc_str + ".normal").c_str()), 2);
			}

			// -- Material Uniforms --

			GLuint diffuseColor = glGetUniformLocation(shader, "material[0].diffuse_color");
			glUniform3fv(diffuseColor, 1, materials.at(object.material_id).diffuse);

			GLuint shininess = glGetUniformLocation(shader, "material[0].shininess");
			glUniform1f(shininess, materials.at(object.material_id).shininess);

			GLuint loaded = glGetUniformLocation(shader, "material[0].loaded");
			glUniform1f(loaded, true);
		}


		// Mesh Uniforms
		GLuint modelLoc = glGetUniformLocation(shader, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m_transform));

		glBindVertexArray(object.va);
		glDrawArrays(GL_TRIANGLES, 0, object.numTriangles * 3);

		// Clean up
		GLuint loaded = glGetUniformLocation(shader, "material[0].loaded");
		glUniform1f(loaded, false);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

}

glm::vec3 Mesh::get_lower_bounds()
{
	return m_lower_bounds;
}

glm::vec3 Mesh::get_upper_bounds()
{
	return m_upper_bounds;
}

std::string Mesh::get_scale()
{
	return std::to_string(scale);
}

void Mesh::remove_instance()
{
	for (size_t m = 0; m < materials.size(); m++) {
		tinyobj::material_t* mp = &materials.at(m);

		if (mp->ambient_texname.length() > 0)
			scene_tracker->Textures->at(mp->ambient_texname).second--;

		if (mp->diffuse_texname.length() > 0)
			scene_tracker->Textures->at(mp->diffuse_texname).second--;

		if (mp->specular_texname.length() > 0)
			--scene_tracker->Textures->at(mp->specular_texname).second;

		if (mp->specular_highlight_texname.length() > 0)
			--scene_tracker->Textures->at(mp->specular_highlight_texname).second;

		if (mp->bump_texname.length() > 0)
			--scene_tracker->Textures->at(mp->bump_texname).second;

		if (mp->displacement_texname.length() > 0)
			--scene_tracker->Textures->at(mp->displacement_texname).second;

		if (mp->alpha_texname.length() > 0)
			--scene_tracker->Textures->at(mp->alpha_texname).second;


		if (mp->roughness_texname.length() > 0)
			--scene_tracker->Textures->at(mp->roughness_texname).second;

		if (mp->metallic_texname.length() > 0)
			--scene_tracker->Textures->at(mp->metallic_texname).second;
		if (mp->sheen_texname.length() > 0)
			--scene_tracker->Textures->at(mp->sheen_texname).second;
		if (mp->emissive_texname.length() > 0)
			--scene_tracker->Textures->at(mp->emissive_texname).second;
		if (mp->normal_texname.length() > 0)
			--scene_tracker->Textures->at(mp->normal_texname).second;
	}
	--scene_tracker->Meshes->at(name).second;
}

void Mesh::setupMesh()
{
	for (size_t s = 0; s < shapes.size(); s++) {
		DrawObject o;
		std::vector<glm::vec3> vb_pos;  // Buffer for Position
		std::vector<glm::vec3> vb_norm;  // Buffer for Normal
		std::vector<glm::vec3> vb_col;  // Buffer for Color
		std::vector<glm::vec2> vb_tex;	// Buffer for Texture Coords

		std::vector<glm::vec3> vb_tan;  // Buffer for Tangent
		std::vector<glm::vec3> vb_bitan;  // Buffer for Bitangent

		for (size_t f = 0; f < shapes.at(s).mesh.indices.size() / 3; f++) {
			tinyobj::index_t idx0 = shapes.at(s).mesh.indices[3 * f + 0];
			tinyobj::index_t idx1 = shapes.at(s).mesh.indices[3 * f + 1];
			tinyobj::index_t idx2 = shapes.at(s).mesh.indices[3 * f + 2];

			int current_material_id = shapes.at(s).mesh.material_ids[f];

			if ((current_material_id < 0) || (current_material_id >= static_cast<int>(materials.size()))) {
				// Invaid material ID. Use default material.
				current_material_id = materials.size() - 1; // Default material is added to the last item in `materials`.
			}

			float diffuse[3];
			for (size_t i = 0; i < 3; i++) {
				diffuse[i] = materials.at(current_material_id).diffuse[i];
			}
			float tc[3][2];
			if (attrib.texcoords.size() > 0) {
				assert(attrib.texcoords.size() > static_cast<size_t>(2 * idx0.texcoord_index + 1));
				assert(attrib.texcoords.size() > static_cast<size_t>(2 * idx1.texcoord_index + 1));
				assert(attrib.texcoords.size() > static_cast<size_t>(2 * idx2.texcoord_index + 1));
				tc[0][0] = attrib.texcoords[2 * idx0.texcoord_index];
				tc[0][1] = 1.0f - attrib.texcoords[2 * idx0.texcoord_index + 1];
				tc[1][0] = attrib.texcoords[2 * idx1.texcoord_index];
				tc[1][1] = 1.0f - attrib.texcoords[2 * idx1.texcoord_index + 1];
				tc[2][0] = attrib.texcoords[2 * idx2.texcoord_index];
				tc[2][1] = 1.0f - attrib.texcoords[2 * idx2.texcoord_index + 1];
			}
			else {
				tc[0][0] = 0.0f;
				tc[0][1] = 0.0f;
				tc[1][0] = 0.0f;
				tc[1][1] = 0.0f;
				tc[2][0] = 0.0f;
				tc[2][1] = 0.0f;
			}

			float v[3][3];
			for (int k = 0; k < 3; k++) {
				int f0 = idx0.vertex_index;
				int f1 = idx1.vertex_index;
				int f2 = idx2.vertex_index;
				assert(f0 >= 0);
				assert(f1 >= 0);
				assert(f2 >= 0);

				v[0][k] = attrib.vertices[3 * f0 + k];
				v[1][k] = attrib.vertices[3 * f1 + k];
				v[2][k] = attrib.vertices[3 * f2 + k];
				bounding_minimum[k] = std::min(v[0][k], bounding_minimum[k]);
				bounding_minimum[k] = std::min(v[1][k], bounding_minimum[k]);
				bounding_minimum[k] = std::min(v[2][k], bounding_minimum[k]);
				bounding_maximum[k] = std::max(v[0][k], bounding_maximum[k]);
				bounding_maximum[k] = std::max(v[1][k], bounding_maximum[k]);
				bounding_maximum[k] = std::max(v[2][k], bounding_maximum[k]);
			}

			float n[3][3];
			if (attrib.normals.size() > 0) {
				int f0 = idx0.normal_index;
				int f1 = idx1.normal_index;
				int f2 = idx2.normal_index;
				assert(f0 >= 0);
				assert(f1 >= 0);
				assert(f2 >= 0);
				for (int k = 0; k < 3; k++) {
					n[0][k] = attrib.normals[3 * f0 + k];
					n[1][k] = attrib.normals[3 * f1 + k];
					n[2][k] = attrib.normals[3 * f2 + k];
				}
			}
			else {
				// compute geometric normal
				calculate_surface_normal(n[0], v[0], v[1], v[2]);
				n[1][0] = n[0][0];
				n[1][1] = n[0][1];
				n[1][2] = n[0][2];
				n[2][0] = n[0][0];
				n[2][1] = n[0][1];
				n[2][2] = n[0][2];
			}

			// Calculate tangents and bitangents
			float tan[3][3], bitan[3][3];
			calculate_tangent_and_bitangent(tan[0], bitan[0], v[0], v[1], v[2], tc[0], tc[1], tc[2]);
			tan[1][0] = tan[0][0];
			tan[1][1] = tan[0][1];
			tan[1][2] = tan[0][2];
			tan[2][0] = tan[0][0];
			tan[2][1] = tan[0][1];
			tan[2][2] = tan[0][2];

			bitan[1][0] = bitan[0][0];
			bitan[1][1] = bitan[0][1];
			bitan[1][2] = bitan[0][2];
			bitan[2][0] = bitan[0][0];
			bitan[2][1] = bitan[0][1];
			bitan[2][2] = bitan[0][2];

			for (int k = 0; k < 3; k++) {
				// Combine normal and diffuse to get color.
				float normal_factor = 0.2;
				float diffuse_factor = 1 - normal_factor;
				glm::vec3 color = {
					n[k][0] * normal_factor + diffuse[0] * diffuse_factor,
					n[k][1] * normal_factor + diffuse[1] * diffuse_factor,
					n[k][2] * normal_factor + diffuse[2] * diffuse_factor
				};
				color = glm::normalize(color);
				color = {
					color.x * 0.5 + 0.5,
					color.y * 0.5 + 0.5,
					color.z * 0.5 + 0.5
				};

				vb_col.push_back(color);
				vb_tex.push_back(glm::vec2(tc[k][0], tc[k][1]));
				vb_pos.push_back(glm::vec3(v[k][0], v[k][1], v[k][2]));
				vb_norm.push_back(glm::vec3(n[k][0], n[k][1], n[k][2]));
				vb_tan.push_back(glm::vec3(tan[k][0], tan[k][1], tan[k][2]));
				vb_bitan.push_back(glm::vec3(bitan[k][0], bitan[k][1], bitan[k][2]));
			}
		}

		// Report on Position Date
		std::cout << "Vertices: " << vb_pos.size() << std::endl;

		o.vb[3] = 0;
		o.numTriangles = 0;

		// OpenGL viewer does not support texturing with per-face material.
		if (shapes.at(s).mesh.material_ids.size() > 0 && shapes.at(s).mesh.material_ids.size() > s) {
			// Base case
			o.material_id = shapes.at(s).mesh.material_ids[s];
		}
		else {
			o.material_id = materials.size() - 1; // = ID for default material.
		}

		// Generate and Bind our VAO
		glGenVertexArrays(1, &o.va);
		glBindVertexArray(o.va);

		// Generate our VBOs
		glGenBuffers(6, o.vb);

		// Bind Vertex Buffer Object
		glBindBuffer(GL_ARRAY_BUFFER, o.vb[0]);
		glBufferData(GL_ARRAY_BUFFER, vb_pos.size() * sizeof(glm::vec3), vb_pos.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		// Bind Normal Buffer Object
		glBindBuffer(GL_ARRAY_BUFFER, o.vb[1]);
		glBufferData(GL_ARRAY_BUFFER, vb_norm.size() * sizeof(glm::vec3), vb_norm.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		// Bind Color Buffer Object
		glBindBuffer(GL_ARRAY_BUFFER, o.vb[2]);
		glBufferData(GL_ARRAY_BUFFER, vb_col.size() * sizeof(glm::vec3), vb_col.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		// Bind UV Buffer Object
		glBindBuffer(GL_ARRAY_BUFFER, o.vb[3]);
		glBufferData(GL_ARRAY_BUFFER, vb_tex.size() * sizeof(glm::vec2), vb_tex.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		// Bind Tangent Buffer Object
		glBindBuffer(GL_ARRAY_BUFFER, o.vb[4]);
		glBufferData(GL_ARRAY_BUFFER, vb_tan.size() * sizeof(glm::vec3), vb_tan.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		// Bind Bitangent Buffer Object
		glBindBuffer(GL_ARRAY_BUFFER, o.vb[5]);
		glBufferData(GL_ARRAY_BUFFER, vb_bitan.size() * sizeof(glm::vec3), vb_bitan.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		// Clean up
		glBindVertexArray(0); // Unbind VAO
		glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind Buffer Object

		o.numTriangles = vb_pos.size() / 3;
		printf("shape[%d] # of triangles = %d\n", static_cast<int>(s),
				o.numTriangles);
		objects.push_back(o);
	}

	for (unsigned int axis = 0; axis < 3; axis++)
	{
		float diff = fabs(bounding_maximum[axis] - bounding_minimum[axis]);
		if (diff > scale)
		{
			scale = diff;
		}
	}

	bounding_center = glm::vec3(
		(bounding_maximum.x + bounding_minimum.x) / 2,
		(bounding_maximum.y + bounding_minimum.y) / 2,
		(bounding_maximum.z + bounding_minimum.z) / 2
	);
}

void Mesh::setupTextures(std::string base_dir)
{
	for (size_t m = 0; m < materials.size(); m++) {
		tinyobj::material_t* mp = &materials.at(m);

		if (mp->ambient_texname.length() > 0)
			loadTexture(base_dir, mp->ambient_texname);
		if (mp->diffuse_texname.length() > 0)
		{
			std::cout << "Loading Diffuse Map (map_Kd): " << mp->diffuse_texname << "\n";
			loadTexture(base_dir, mp->diffuse_texname);
		}
		if (mp->specular_texname.length() > 0)
		{
			std::cout << "Loading Specular (map_Ks): " << mp->specular_texname << "\n";
			loadTexture(base_dir, mp->specular_texname);
		}
		if (mp->specular_highlight_texname.length() > 0)
			loadTexture(base_dir, mp->specular_highlight_texname);
		if (mp->bump_texname.length() > 0)
		{
			std::cout << "Loading Bump Map (map_bump): " << mp->bump_texname << "\n";
			loadTexture(base_dir, mp->bump_texname);
		}
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
		{
			std::cout << "Loading Normal Map (norm): " << mp->normal_texname << "\n";
			loadTexture(base_dir, mp->normal_texname);
		}
	}
}

void Mesh::loadTexture(std::string base_dir, std::string texture_name)
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

void Mesh::generateTransform()
{
	m_transform = glm::mat4();
	m_transform = glm::translate(m_transform, -(bounding_center * (1/scale)));
	m_transform = glm::scale(m_transform, glm::vec3(1 / scale));
}

void Mesh::compute_bounds()
{
	// Shift our upper and lower bounds by transform, then reasses each vertex for the new max and min positions
	glm::vec4 tmp_vec_L = glm::vec4(bounding_minimum, 1.0);
	tmp_vec_L = m_transform * tmp_vec_L;

	glm::vec4 tmp_vec_H = glm::vec4(bounding_maximum, 1.0);
	tmp_vec_H = m_transform * tmp_vec_H;

	m_lower_bounds = glm::vec3(glm::min(tmp_vec_L.x, tmp_vec_H.x), glm::min(tmp_vec_L.y, tmp_vec_H.y), glm::min(tmp_vec_L.z, tmp_vec_H.z));

	m_upper_bounds = glm::vec3(glm::max(tmp_vec_L.x, tmp_vec_H.x), glm::max(tmp_vec_L.y, tmp_vec_H.y), glm::max(tmp_vec_L.z, tmp_vec_H.z));
}


void calculate_surface_normal(float Normal[3], float const vertex_1[3], float const vertex_2[3], float const vertex_3[3])
{
	// https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
	glm::vec3 vector_1 = glm::vec3(vertex_2[0] - vertex_1[0], vertex_2[1] - vertex_1[1], vertex_2[2] - vertex_1[2]);
	glm::vec3 vector_2 = glm::vec3(vertex_3[0] - vertex_1[0], vertex_3[1] - vertex_1[1], vertex_3[2] - vertex_1[2]);

	glm::vec3 cross = glm::cross(vector_1, vector_2);
	cross = glm::normalize(cross);

	Normal[0] = cross[0];
	Normal[1] = cross[1];
	Normal[2] = cross[2];
}

void calculate_tangent_and_bitangent(float Tangent[3], float Bitangent[3], float const vertex_1[3], float const vertex_2[3], float const vertex_3[3], float uv_1[2], float uv_2[2], float uv_3[2])
{
	glm::vec3 tan, bitan;
	//https://learnopengl.com/#!Advanced-Lighting/Normal-Mapping
	glm::vec3 vector_1 = glm::vec3(vertex_2[0] - vertex_1[0], vertex_2[1] - vertex_1[1], vertex_2[2] - vertex_1[2]);
	glm::vec3 vector_2 = glm::vec3(vertex_3[0] - vertex_1[0], vertex_3[1] - vertex_1[1], vertex_3[2] - vertex_1[2]);

	glm::vec2 deltaUV1 = glm::vec2(uv_2[0] - uv_1[0], uv_2[1] - uv_1[1]);
	glm::vec2 deltaUV2 = glm::vec2(uv_3[0] - uv_1[0], uv_3[1] - uv_1[1]);

	float ratio = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	tan = (vector_1 * deltaUV2.y - vector_2 * deltaUV1.y)*ratio;

	bitan = (vector_2 * deltaUV1.x - vector_1 * deltaUV2.x)*ratio;

	Tangent[0] = tan[0];
	Tangent[1] = tan[1];
	Tangent[2] = tan[2];

	Bitangent[0] = bitan[0];
	Bitangent[1] = bitan[1];
	Bitangent[2] = bitan[2];
}
