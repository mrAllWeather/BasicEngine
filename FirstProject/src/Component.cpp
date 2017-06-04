#include "../include/Component.h"

Component::Component(std::string name, std::string static_details, loadedComponents* scene_tracker)
{
	glm::vec3 rotation;
	this->m_location = new glm::vec3;
	this->m_scale = new glm::vec3;

	std::string mesh_file_name;

	m_name = name;
	// Load Component
	std::istringstream iss(static_details);

	// 'Component_Name' COMPONENT_FILE scale.x scale.y scale.z loc.x loc.y loc.z rot.x rot.y rot.z // Local
	iss >> m_name >> mesh_file_name >>
		m_scale->x >> m_scale->y >> m_scale->z >>
		m_location->x >> m_location->y >> m_location->z >>
		rotation.x >> rotation.y >> rotation.z;

	// Quaternion Rotation attempt
	m_rotation = new glm::quat(rotation);

	std::cout << "Loading: " << m_name << " (" << mesh_file_name << ")" << std::endl;
	std::cout << "\tScale: " << m_scale->x << " " << m_scale->y << " " << m_scale->z << std::endl;
	std::cout << "\tLocation: " << m_location->x << " " << m_location->y << " " << m_location->z << std::endl;
	std::cout << "\tRotation: " << rotation.x << " " << rotation.y << " " << rotation.z << std::endl;

	this->m_mesh_name = mesh_file_name;

	// this->m_Mesh = new Mesh(mesh_file_name, scene_tracker);
	if (scene_tracker->Meshes->find(mesh_file_name) == scene_tracker->Meshes->end())
	{
		this->m_Mesh = new Mesh(mesh_file_name, scene_tracker);
		scene_tracker->Meshes->insert(std::make_pair(mesh_file_name, std::make_pair(m_Mesh, 1)));
	}
	else
	{
		std::cout << "Loaded cached mesh.\n";
		this->m_Mesh = scene_tracker->Meshes->at(mesh_file_name).first;
		scene_tracker->Meshes->at(mesh_file_name).second++;
	}


	build_component_transform();
	compute_bounds();
}

Component::Component(std::string name, std::string mesh_name, glm::quat rot, glm::vec3 loc, glm::vec3 scale, loadedComponents* scene_tracker)
{
	this->m_name = name;
	this->m_mesh_name = mesh_name;
	this->m_rotation = new glm::quat(rot);
	this->m_location = new glm::vec3(loc);
	this->m_scale = new glm::vec3(scale);

	this->m_Mesh = new Mesh(m_mesh_name, scene_tracker);

}

Component::~Component()
{
	m_Mesh->remove_instance();
	if (scene_tracker->Meshes->at(m_mesh_name).first == 0)
	{
		delete m_Mesh;
	}

	delete m_location;
	delete m_scale;
	delete m_rotation;

}

void Component::draw(GLuint shader)
{
	// std::cout << "Component::Draw\n";
	GLuint modelLoc = glGetUniformLocation(shader, "component");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m_transform));
	m_Mesh->draw(shader);
}

glm::vec3 Component::get_lower_bounds()
{
	return m_lower_bounds;

}

glm::vec3 Component::get_upper_bounds()
{
	return m_upper_bounds;
}

void Component::build_component_transform()
{
	m_transform = glm::mat4();
	m_transform = glm::translate(m_transform, *m_location);

	m_transform *= glm::toMat4(*m_rotation);

	m_transform = glm::scale(m_transform, *m_scale);

}

void Component::compute_bounds()
{
	// Lower
	glm::vec4 tmp_vec = glm::vec4(m_Mesh->get_lower_bounds(), 1.0);
	tmp_vec = m_transform * tmp_vec;
	m_lower_bounds = glm::vec3(tmp_vec.x, tmp_vec.y, tmp_vec.z);

	// Upper
	tmp_vec = glm::vec4(m_Mesh->get_upper_bounds(), 1.0);
	tmp_vec = m_transform * tmp_vec;
	m_upper_bounds = glm::vec3(tmp_vec.x, tmp_vec.y, tmp_vec.z);
}
