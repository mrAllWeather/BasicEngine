#include "StaticMesh.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

StaticMesh::StaticMesh(	std::string static_details, ShaderLoader* scene_shader_loader, ObjLoader* scene_object_loader)
{
	this->rotation = new glm::vec3;
	this->location = new glm::vec3;
	this->scale = new glm::vec3;

	std::string static_file_name;

	// Load Component Details
	sscanf(static_details.c_str(), "\t%s, %s, %f, %f, %f, %f, %f, %f, %f, %f, %f\n",
		&name, &static_file_name,
		&scale->x, &scale->y, &scale->z,
		&location->x, &location->y, &location->z,
		&rotation->x, &rotation->y, &rotation->z);

	std::ifstream fb; // FileBuffer
	fb.open((static_file_name), std::ios::in);
	std::string LineBuf;
	std::stringstream ss;
	std::vector<std::string> shader_files;
	std::vector<std::string> texture_files;

	if(fb.is_open()){
		// Load Obj
		std::getline(fb, LineBuf);
		scene_object_loader->build_static_mesh(LineBuf, VAO, VBO);

		// Load Textures
		std::getline(fb, LineBuf);
		ss.str(LineBuf);
		for (std::string each; std::getline(ss, each, ','); build_texture(each));

		std::getline(fb, LineBuf);
		ss.str(LineBuf);
		for (std::string each; std::getline(ss, each, ','); texture_files.push_back(each));
		shader_program = scene_shader_loader->build_program(shader_files);
	}

	fb.close();
}

StaticMesh::~StaticMesh()
{
	glDeleteProgram(this->shader_program);
}

void StaticMesh::draw()
{
	// Only 32 GL_TEXTURE# are defined (0x84C0 -> 0x84DF)
	for (unsigned int tex_num = 0; tex_num < texture.size() && tex_num < 32; tex_num++)
	{
		std::string texture_name = "texture_" + std::to_string(tex_num);
		glActiveTexture(GL_TEXTURE0+tex_num);
		glBindTexture(GL_TEXTURE_2D, texture.at(tex_num));
		glUniform1i(glGetUniformLocation(this->shader_program, texture_name.c_str()), 0);
	}
	
	glUseProgram(this->shader_program);
	glBindVertexArray(*this->VAO);
	glDrawArrays(GL_TRIANGLES, 0, this->vertices);
	glBindVertexArray(0);
}

// TODO Replace with Scene function
void StaticMesh::build_texture(std::string texture_file)
{
	GLuint load_texture;
	glGenTextures(1, &load_texture);
	glBindTexture(GL_TEXTURE_2D, load_texture); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load, create texture and generate mipmaps
	int width, height, bpp;

	unsigned char* image = stbi_load(texture_file.c_str(), &width, &height, &bpp, STBI_rgb);
	/*
	unsigned char* image = NULL;
	width = 0;
	height = 0;
	*/
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	glGenerateMipmap(GL_TEXTURE_2D);

	// stbi_image_free(image);

	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.

	this->texture.push_back(load_texture);
}
