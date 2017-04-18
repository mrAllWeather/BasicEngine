#include "StaticMesh.h"

StaticMesh::StaticMesh(	std::string static_details, 
			ShaderLoader* scene_shader_loader, 
			ObjLoader* scene_object_loader,
			TextureLoader* scene_texture_loader)
{
	this->rotation = new glm::vec3;
	this->location = new glm::vec3;
	this->scale = new glm::vec3;

	std::string static_file_name;

	// Load Component
	std::istringstream iss(static_details);

	iss >> name >> static_file_name >>
		scale->x >> scale->y >> scale->z >>
		location->x >> location->y >> location->z >>
		rotation->x >> rotation->y >> rotation->z;

	std::cout << "Loading Static: " << name << " (" << static_file_name << ")" << std::endl;

	std::ifstream fb; // FileBuffer
	fb.open((static_file_name), std::ios::in);
	std::string LineBuf;
	std::stringstream ss;
	std::vector<std::string> shader_files;

	if(fb.is_open()){
		// Load Obj
		std::getline(fb, LineBuf);

		scene_object_loader->build_static_mesh(LineBuf, &VAO, &VBO, &vertices);

		// Load Textures
		std::getline(fb, LineBuf);
		ss.str(LineBuf);
		for (std::string each; std::getline(ss, each, ','); textures.push_back(std::make_pair(each, scene_texture_loader->get_texture(each))));

		// Load Shaders
		std::getline(fb, LineBuf);
		ss.clear();
		ss.str(LineBuf);
		for (std::string each; std::getline(ss, each, ','); shader_files.push_back(each));
		std::pair<std::string, std::string> shaders = std::make_pair(shader_files[0], shader_files[1]);
		shader_program = scene_shader_loader->build_program(shaders);
	}

	fb.close();

	build_component_transform();
}

StaticMesh::~StaticMesh()
{
	glDeleteProgram(this->shader_program);
}

void StaticMesh::build_component_transform()
{
	component_transform = glm::mat4();
	component_transform = glm::translate(component_transform, *location);

	component_transform = glm::rotate(component_transform, rotation->x, glm::vec3(1.0, 0.0, 0.0));
	component_transform = glm::rotate(component_transform, rotation->y, glm::vec3(0.0, 1.0, 0.0));
	component_transform = glm::rotate(component_transform, rotation->z, glm::vec3(0.0, 0.0, 1.0));

	component_transform = glm::scale(component_transform, *scale);


}
