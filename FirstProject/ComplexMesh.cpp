#include "ComplexMesh.h"

ComplexMesh::ComplexMesh(std::string cmesh_details, Scene* parent)
{
	this->rotation = new glm::vec3;
	this->location = new glm::vec3;
	this->scale = new glm::vec3;

	components = new std::map<std::string, StaticMesh*>;

	// 'Static_Name', COMPLEX_FILE, scale.x, scale.y, scale.z, loc.x, loc.y, loc.z, rot.x, rot.y, rot.z // World
	std::string cmesh_file_name;

	// Load Complex Mesh Details
	sscanf(cmesh_details.c_str(), "\t%s, %s, %f, %f, %f, %f, %f, %f, %f, %f, %f\n",
		&name, &cmesh_file_name,
		&scale->x, &scale->y, &scale->z,
		&location->x, &location->y, &location->z,
		&rotation->x, &rotation->y, &rotation->z);

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
			std::getline(ss, component_name, ',');

			// Create component
			components->operator[](component_name) = new StaticMesh(LineBuf, this);
		}
	}

	fb.close();
}

ComplexMesh::~ComplexMesh()
{
	for (auto component : *Components)
	{
		delete component.second;
	}
}

void ComplexMesh::tick(GLfloat delta)
{
}

void ComplexMesh::draw()
{
	for (auto component : *Components)
	{
		component.second->draw();
	}
}
