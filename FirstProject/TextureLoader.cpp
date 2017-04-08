#include "TextureLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

TextureLoader::TextureLoader ()
{
	
	this->loaded_textures = new std::map<std::string, std::pair<GLuint, GLuint> >;
}

TextureLoader::~TextureLoader()
{
	delete loaded_textures;
}

void TextureLoader::add_texture(std::string filename)
{
	if(!is_texture_built(filename))
	{
		load_texture(filename);
	}
}

void TextureLoader::remove_texture(std::string filename)
{
	if(is_texture_built(filename))
	{
		loaded_textures->at(filename).second--;
	}
	if(loaded_textures->at(filename).second < 1)
	{
		GLuint* texID = &loaded_textures->at(filename).first;
		glDeleteTextures(1, texID);
		loaded_textures->erase(filename);
	}
}

GLuint TextureLoader::get_texture(std::string filename)
{
	if(!is_texture_built(filename))
	{
		load_texture(filename);
	}
	loaded_textures->at(filename).second++;

	return loaded_textures->at(filename).first;
}

void TextureLoader::load_texture(std::string texture_file)
{
	std::cout << "Loading Texture: " << texture_file << std::endl;
	GLuint load_texture;

	glGenTextures(1, &load_texture);
	glBindTexture(GL_TEXTURE_2D, load_texture); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   // Set texture wrapping to GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load, create texture and generate mipmaps
	int width, height, num_channels;

	unsigned char* image = stbi_load(texture_file.c_str(), &width, &height, &num_channels, 0);

	if (num_channels == 3) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	}
	else {
		fprintf(stderr, "Image pixels are not RGB. Texture may not load correctly.");
	}

	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(image);

	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.

	loaded_textures->emplace(std::make_pair(texture_file, std::make_pair(load_texture, 0)));
}

bool TextureLoader::is_texture_built(std::string filename)
{
	if(loaded_textures->count(filename))
		return true;

	return false;
}
