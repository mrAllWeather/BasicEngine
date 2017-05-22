#include "../include/Object.h"

Object::Object(std::string cmesh_details, loadedComponents* scene_tracker)
{
	glm::vec3 rotation;
	this->m_location = new glm::vec3;
	this->m_scale = new glm::vec3;
	this->scene_tracker = scene_tracker;

	components = new std::map<std::string, Component*>;

	// 'Static_Name' COMPLEX_FILE scale.x scale.y scale.z loc.x loc.y loc.z rot.x rot.y rot.z // World
	std::string object_file_name;

	// Load Complex Mesh Details
	std::istringstream iss(cmesh_details);

	iss >> m_name >> object_file_name >>
		m_scale->x >> m_scale->y >> m_scale->z >>
		m_location->x >> m_location->y >> m_location->z >>
		rotation.x >> rotation.y >> rotation.z;

	std::cout << "Loading: " << m_name << " (" << object_file_name << ")" << std::endl;
	std::cout << "\tScale: " << m_scale->x << " " << m_scale->y << " " << m_scale->z << std::endl;
	std::cout << "\tLocation: " << m_location->x << " " << m_location->y << " " << m_location->z << std::endl;
	std::cout << "\tRotation: " << rotation.x << " " << rotation.y << " " << rotation.z << std::endl;

	m_rotation = new glm::quat(rotation);

	std::ifstream fb; // FileBuffer
	fb.open((object_file_name), std::ios::in);
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
			components->operator[](component_name) = new Component(LineBuf, scene_tracker);
		}
	}
	fb.close();

	build_static_transform();
}

Object::Object(std::string name, glm::quat rot, glm::vec3 loc, glm::vec3 scale, loadedComponents * scene_tracker)
{
	this->m_name = name;
	this->m_rotation = new glm::quat(rot);
	this->m_location = new glm::vec3(loc);
	this->m_scale = new glm::vec3(scale);
	this->scene_tracker = scene_tracker;

	components = new std::map<std::string, Component*>;
}

void Object::addComponent(std::string name, std::string mesh_name, glm::quat rot, glm::vec3 loc, glm::vec3 scale)
{
	if(components->find(name) == components->end())
		components->operator[](name) = new Component(name, mesh_name, rot, loc, scale, scene_tracker);
}

void Object::remComponent(std::string name)
{
	if (components->find(name) != components->end())
	{
		delete components->at(name);
		components->erase(name);
	}
}

void Object::draw(GLuint shader)
{
	GLuint modelLoc = glGetUniformLocation(shader, "object");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m_transform));

	for (auto component : *components)
	{
		component.second->draw(shader);
	}
}

void Object::build_static_transform()
{
	m_transform = glm::mat4();
	m_transform = glm::translate(m_transform, *m_location);

	m_transform *= glm::toMat4(*m_rotation);

	m_transform = glm::scale(m_transform, *m_scale);


}
