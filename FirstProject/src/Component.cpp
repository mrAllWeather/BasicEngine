#include "../include/Component.h"

Component::Component() {
    m_Mesh        = nullptr;
    m_rotation    = nullptr;
	m_location    = nullptr;
	m_scale       = nullptr;
    scene_tracker = nullptr;
}

Component::Component(std::string name, std::string static_details, loadedComponents* scene_tracker) : Component()
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

	std::cerr << "Component Bounds: (" << m_lower_bounds.x << ", " << m_lower_bounds.y << ", " << m_lower_bounds.z << ") - (" << m_upper_bounds.x << ", " << m_upper_bounds.y << ", " << m_upper_bounds.z << ")\n" << std::endl;
}

Component::Component(std::string name, std::string mesh_name, glm::quat rot, glm::vec3 loc, glm::vec3 scale, loadedComponents* scene_tracker) : Component()
{
	this->m_name = name;
	this->m_mesh_name = mesh_name;
	this->m_rotation = new glm::quat(rot);
	this->m_location = new glm::vec3(loc);
	this->m_scale = new glm::vec3(scale);

	if (scene_tracker->Meshes->find(m_mesh_name) == scene_tracker->Meshes->end())
	{
		this->m_Mesh = new Mesh(m_mesh_name, scene_tracker);
		scene_tracker->Meshes->insert(std::make_pair(m_mesh_name, std::make_pair(m_Mesh, 1)));
	}
	else
	{
		std::cout << "Loaded cached mesh.\n";
		this->m_Mesh = scene_tracker->Meshes->at(m_mesh_name).first;
		scene_tracker->Meshes->at(m_mesh_name).second++;
	}


	build_component_transform();
	compute_bounds();

	std::cerr << "Component Bounds: (" << m_lower_bounds.x << ", " << m_lower_bounds.y << ", " << m_lower_bounds.z << ") - (" << m_upper_bounds.x << ", " << m_upper_bounds.y << ", " << m_upper_bounds.z << ")\n" << std::endl;

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
	// Shift our upper and lower bounds by transform, then reasses each vertex for the new max and min positions
	glm::vec4 tmp_vec_L = glm::vec4(m_Mesh->get_lower_bounds(), 1.0);
	tmp_vec_L = m_transform * tmp_vec_L;

	glm::vec4 tmp_vec_H = glm::vec4(m_Mesh->get_upper_bounds(), 1.0);
	tmp_vec_H = m_transform * tmp_vec_H;

	m_lower_bounds = glm::vec3(glm::min(tmp_vec_L.x, tmp_vec_H.x), glm::min(tmp_vec_L.y, tmp_vec_H.y), glm::min(tmp_vec_L.z, tmp_vec_H.z));

	m_upper_bounds = glm::vec3(glm::max(tmp_vec_L.x, tmp_vec_H.x), glm::max(tmp_vec_L.y, tmp_vec_H.y), glm::max(tmp_vec_L.z, tmp_vec_H.z));
}

void Component::compute_scene_bounds(glm::mat4 & scene_transform)
{
	glm::vec4 tmp_vec_L = glm::vec4(m_lower_bounds, 1.0);
	tmp_vec_L = scene_transform * tmp_vec_L;

	glm::vec4 tmp_vec_H = glm::vec4(m_upper_bounds, 1.0);
	tmp_vec_H = scene_transform * tmp_vec_H;

	m_scene_lower_bounds = glm::vec3(glm::min(tmp_vec_L.x, tmp_vec_H.x), glm::min(tmp_vec_L.y, tmp_vec_H.y), glm::min(tmp_vec_L.z, tmp_vec_H.z));
	m_scene_upper_bounds = glm::vec3(glm::max(tmp_vec_L.x, tmp_vec_H.x), glm::max(tmp_vec_L.y, tmp_vec_H.y), glm::max(tmp_vec_L.z, tmp_vec_H.z));
}

bool Component::is_collision(glm::vec3 lower_bound, glm::vec3 upper_bound, glm::mat4 &scene_transform)
{
	if (m_scene_lower_bounds == glm::vec3(0) && m_scene_upper_bounds == glm::vec3(0))
	{
		compute_scene_bounds(scene_transform);
	}

	bool x_collision = (lower_bound.x >= m_scene_lower_bounds.x && upper_bound.x <= m_scene_upper_bounds.x) || // Within Bounds
		(lower_bound.x <= m_scene_lower_bounds.x && upper_bound.x >= m_scene_lower_bounds.x) || // Within Lower
		(lower_bound.x <= m_scene_upper_bounds.x && upper_bound.x >= m_scene_upper_bounds.x); // Within Hight

	bool y_collision = (lower_bound.y >= m_scene_lower_bounds.y && upper_bound.y <= m_scene_upper_bounds.y) || // Within Bounds
		(lower_bound.y <= m_scene_lower_bounds.y && upper_bound.y >= m_scene_lower_bounds.y) || // Within Lower
		(lower_bound.y <= m_scene_upper_bounds.y && upper_bound.y >= m_scene_upper_bounds.y); // Within Hight

	bool z_collision = (lower_bound.z >= m_scene_lower_bounds.z && upper_bound.z <= m_scene_upper_bounds.z) || // Within Bounds
		(lower_bound.z <= m_scene_lower_bounds.z && upper_bound.z >= m_scene_lower_bounds.z) || // Within Lower
		(lower_bound.z <= m_scene_upper_bounds.z && upper_bound.z >= m_scene_upper_bounds.z); // Within Hight

	return x_collision & y_collision & z_collision;

}
