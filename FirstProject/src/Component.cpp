#include "../include/Component.h"

Component::Component(std::string static_details, loadedComponents* scene_tracker)
{
	glm::vec3 rotation;
	this->m_location = new glm::vec3;
	this->m_scale = new glm::vec3;

	std::string mesh_file_name;

	// Load Component
	std::istringstream iss(static_details);

	// 'Component_Name' COMPONENT_FILE scale.x scale.y scale.z loc.x loc.y loc.z rot.x rot.y rot.z // Local
	iss >> m_name >> mesh_file_name >>
		m_scale->x >> m_scale->y >> m_scale->z >>
		m_location->x >> m_location->y >> m_location->z >>
		rotation.x >> rotation.y >> rotation.z;

	// Quaternion Rotation attempt
	m_rototation = new glm::quat(rotation);

	std::cout << "Loading Static: " << m_name << " (" << mesh_file_name << ")" << std::endl;
	this->m_mesh_name = mesh_file_name;

	this->m_Mesh = new Mesh(mesh_file_name, scene_tracker);

	build_component_transform();
}

Component::Component(std::string name, std::string mesh_name, glm::quat rot, glm::vec3 loc, glm::vec3 scale, loadedComponents* scene_tracker)
{
	this->m_name = name;
	this->m_mesh_name = mesh_name;
	this->m_rototation = new glm::quat(rot);
	this->m_location = new glm::vec3(loc);
	this->m_scale = new glm::vec3(scale);

	this->m_Mesh = new Mesh(m_mesh_name, scene_tracker);

}

Component::~Component()
{
	m_Mesh->remove_instance();
	// TODO Clean Up our memory but DO NOT delete Mesh
}

void Component::draw(GLuint shader)
{
	GLuint modelLoc = glGetUniformLocation(shader, "component");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m_transform));
	m_Mesh->draw(shader);
}

void Component::build_component_transform()
{
	m_transform = glm::mat4();
	m_transform = glm::translate(m_transform, *m_location);

	m_transform *= glm::toMat4(*m_rototation);

	m_transform = glm::scale(m_transform, *m_scale);

}
