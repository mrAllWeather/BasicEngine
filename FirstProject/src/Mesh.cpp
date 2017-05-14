#include "../include/Mesh.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

Mesh::Mesh(std::string filename, std::map<std::string, GLuint>& scene_textures, std::string base_dir)
{
	this->attrib = new tinyobj::attrib_t;
	this->objects = new std::vector<DrawObject>;
	this->loaded_textures = &scene_textures;
	this->materials = new std::vector<tinyobj::material_t>;
	this->shapes = new std::vector<tinyobj::shape_t>;
	std::string err;

	tinyobj::LoadObj(attrib, shapes, materials, &err, filename.c_str(), base_dir.c_str());

	if (!err.empty())
	{
		std::cout << "Encountered error loading file: " << filename << "\n" << err << std::endl;
	}

	bounding_maximum = glm::vec3(std::numeric_limits<float>::lowest());
	bounding_minimum = glm::vec3(std::numeric_limits<float>::max());

	// Add a default material TODO Confirm this doesn't mess with anything
	materials->push_back(tinyobj::material_t());

	setupMesh();
	setupTextures(base_dir);
}

void Mesh::draw(GLuint shader)
{
	for (const auto object : *objects)
	{
		// materials->at(object.material_id).
		// Get component Specular Value
		// glUniform1f(specularStrength, component.second->specular);
		// GLuint modelLoc = glGetUniformLocation(shader_program.first, "model");
		// glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(component.first->static_transform));

		if ((object.material_id < materials->size())) {
			std::string diffuse_texname = materials->at(object.material_id).diffuse_texname;
			if (loaded_textures->find(diffuse_texname) != loaded_textures->end()) {
				glBindTexture(GL_TEXTURE_2D, loaded_textures->at(diffuse_texname));
			}
		}

		//GLuint componentLoc = glGetUniformLocation(shader_program.first, "component");
		// glUniformMatrix4fv(componentLoc, 1, GL_FALSE, glm::value_ptr(component.second->component_transform));

		glBindVertexArray(object.va);
		glDrawArrays(GL_TRIANGLES, 0, object.numTriangles);
		glBindVertexArray(0);

	}

}

void Mesh::setupMesh()
{
	
	for (size_t s = 0; s < shapes->size(); s++) {
		DrawObject o;
		std::vector<glm::vec3> vb_pos;  // Buffer for Position
		std::vector<glm::vec3> vb_norm;  // Buffer for Normal
		std::vector<glm::vec3> vb_col;  // Buffer for Color
		std::vector<glm::vec2> vb_tex;	// Buffer for Texture Coords

		for (size_t f = 0; f < shapes->at(s).mesh.indices.size() / 3; f++) {
			tinyobj::index_t idx0 = shapes->at(s).mesh.indices[3 * f + 0];
			tinyobj::index_t idx1 = shapes->at(s).mesh.indices[3 * f + 1];
			tinyobj::index_t idx2 = shapes->at(s).mesh.indices[3 * f + 2];

			int current_material_id = shapes->at(s).mesh.material_ids[f];

			if ((current_material_id < 0) || (current_material_id >= static_cast<int>(materials->size()))) {
				// Invaid material ID. Use default material.
				current_material_id = materials->size() - 1; // Default material is added to the last item in `materials`.
			}

			float diffuse[3];
			for (size_t i = 0; i < 3; i++) {
				diffuse[i] = materials->at(current_material_id).diffuse[i];
			}
			float tc[3][2];
			if (attrib->texcoords.size() > 0) {
				assert(attrib->texcoords.size() > 2 * idx0.texcoord_index + 1);
				assert(attrib->texcoords.size() > 2 * idx1.texcoord_index + 1);
				assert(attrib->texcoords.size() > 2 * idx2.texcoord_index + 1);
				tc[0][0] = attrib->texcoords[2 * idx0.texcoord_index];
				tc[0][1] = 1.0f - attrib->texcoords[2 * idx0.texcoord_index + 1];
				tc[1][0] = attrib->texcoords[2 * idx1.texcoord_index];
				tc[1][1] = 1.0f - attrib->texcoords[2 * idx1.texcoord_index + 1];
				tc[2][0] = attrib->texcoords[2 * idx2.texcoord_index];
				tc[2][1] = 1.0f - attrib->texcoords[2 * idx2.texcoord_index + 1];
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

				v[0][k] = attrib->vertices[3 * f0 + k];
				v[1][k] = attrib->vertices[3 * f1 + k];
				v[2][k] = attrib->vertices[3 * f2 + k];
				bounding_minimum[k] = std::min(v[0][k], bounding_minimum[k]);
				bounding_minimum[k] = std::min(v[1][k], bounding_minimum[k]);
				bounding_minimum[k] = std::min(v[2][k], bounding_minimum[k]);
				bounding_maximum[k] = std::max(v[0][k], bounding_maximum[k]);
				bounding_maximum[k] = std::max(v[1][k], bounding_maximum[k]);
				bounding_maximum[k] = std::max(v[2][k], bounding_maximum[k]);
			}

			float n[3][3];
			if (attrib->normals.size() > 0) {
				int f0 = idx0.normal_index;
				int f1 = idx1.normal_index;
				int f2 = idx2.normal_index;
				assert(f0 >= 0);
				assert(f1 >= 0);
				assert(f2 >= 0);
				for (int k = 0; k < 3; k++) {
					n[0][k] = attrib->normals[3 * f0 + k];
					n[1][k] = attrib->normals[3 * f1 + k];
					n[2][k] = attrib->normals[3 * f2 + k];
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

			for (int k = 0; k < 3; k++) {
				vb_pos.push_back(glm::vec3(v[k][0], v[k][1], v[k][2]));
				vb_norm.push_back(glm::vec3(n[k][0], n[k][1], n[k][2]));

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
			}
		}

		o.va, o.vb[0], o.vb[1], o.vb[2], o.vb[3] = 0;
		o.numTriangles = 0;

		// OpenGL viewer does not support texturing with per-face material.
		if (shapes->at(s).mesh.material_ids.size() > 0 && shapes->at(s).mesh.material_ids.size() > s) {
			// Base case
			o.material_id = shapes->at(s).mesh.material_ids[s];
		}
		else {
			o.material_id = materials->size() - 1; // = ID for default material.
		}

		// Generate and Bind our VAO
		glGenVertexArrays(1, &o.va);
		glBindVertexArray(o.va);

		// Generate our VBOs
		glGenBuffers(3, o.vb);

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
		glBufferData(GL_ARRAY_BUFFER, vb_norm.size() * sizeof(glm::vec3), vb_norm.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		// Bind UV Buffer Object
		glBindBuffer(GL_ARRAY_BUFFER, o.vb[3]);
		glBufferData(GL_ARRAY_BUFFER, vb_tex.size() * sizeof(glm::vec2), vb_tex.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		// Clean up
		glBindVertexArray(0); // Unbind VAO
		glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind Buffer Object

		o.numTriangles = vb_pos.size() / 3;
		printf("shape[%d] # of triangles = %d\n", static_cast<int>(s),
				o.numTriangles);
		
		objects->push_back(o);
	}

	float scale = 0;
	for (unsigned int axis = 0; axis < 3; axis++)
	{
		float diff = fabs(bounding_maximum[axis] - bounding_minimum[axis]);
		std::cout << diff << std::endl;
		if (diff > scale)
		{
			scale = diff;
		}
	}
	normalise_scale = 1 / scale;
}

void Mesh::setupTextures(std::string base_dir)
{
	for (size_t m = 0; m < materials->size(); m++) {
		tinyobj::material_t* mp = &materials->at(m);

		if (mp->diffuse_texname.length() > 0) {
			// Only load the texture if it is not already loaded
			if (loaded_textures->find(mp->diffuse_texname) == loaded_textures->end()) {
				GLuint texture_id;
				int w, h;
				int comp;

				std::string texture_filename = mp->diffuse_texname;

				// TODO fix this
				if (!FileExists(texture_filename)) {
					// Append base dir.
					texture_filename = base_dir + mp->diffuse_texname;
					if (!FileExists(texture_filename)) {
						std::cerr << "Unable to find file: " << mp->diffuse_texname << std::endl;
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
				loaded_textures->insert(std::make_pair(mp->diffuse_texname, texture_id));
			}
		}
	}
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