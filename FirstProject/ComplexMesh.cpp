#include "ComplexMesh.h"

ComplexMesh::ComplexMesh(std::string cmesh_details, 
	ShaderLoader* scene_shader_loader, 
	ObjLoader* scene_object_loader,
	TextureLoader* scene_texture_loader)
{
	this->rotation = new glm::vec3;
	this->location = new glm::vec3;
	this->scale = new glm::vec3;

	components = new std::map<std::string, StaticMesh*>;

	// 'Static_Name', COMPLEX_FILE, scale.x, scale.y, scale.z, loc.x, loc.y, loc.z, rot.x, rot.y, rot.z // World
	std::string cmesh_file_name;

	// Load Complex Mesh Details
	std::istringstream iss(cmesh_details);

	iss >> name >> cmesh_file_name >>
		scale->x >> scale->y >> scale->z >>
		location->x >> location->y >> location->z >>
		rotation->x >> rotation->y >> rotation->z;

	std::cout << "Loading: " << name << " (" << cmesh_file_name << ")" << std::endl;
	std::cout << "\tScale: " << scale->x << " " << scale->y << " " << scale->z << std::endl;
	std::cout << "\tLocation: " << location->x << " " << location->y << " " << location->z << std::endl;
	std::cout << "\tRotation: " << rotation->x << " " << rotation->y << " " << rotation->z << std::endl;

	std::ifstream fb; // FileBuffer
	fb.open((cmesh_file_name), std::ios::in);
	std::string LineBuf, component_name;
	std::stringstream ss;

	
	if (fb.is_open()) {
		while (std::getline(fb, LineBuf))
		{
			// Get component name
			ss.clear();
			ss.str(LineBuf);
			std::getline(ss, component_name, ' ');

			// Create component
			components->operator[](component_name) = new StaticMesh(LineBuf, scene_shader_loader, scene_object_loader, scene_texture_loader);
		}
	}
	fb.close();

	build_static_transform();
}

void ComplexMesh::build_static_transform()
{
	static_transform = glm::mat4();
	static_transform = glm::translate(static_transform, *location);

	static_transform = glm::rotate(static_transform, rotation->x, glm::vec3(1.0, 0.0, 0.0));
	static_transform = glm::rotate(static_transform, rotation->y, glm::vec3(0.0, 1.0, 0.0));
	static_transform = glm::rotate(static_transform, rotation->z, glm::vec3(0.0, 0.0, 1.0));

	static_transform = glm::scale(static_transform, *scale);


}
