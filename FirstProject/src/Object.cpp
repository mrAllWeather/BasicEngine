#include "../include/Object.h"
#include <algorithm>
#include <limits>

Object::Object(std::string name, std::string cmesh_details, loadedComponents* scene_tracker)
{
	glm::vec3 rotation;
	this->m_location = new glm::vec3;
	this->m_scale = new glm::vec3;
	this->scene_tracker = scene_tracker;

	components = new std::map<std::string, Component*>;

	m_name = name;

	// 'Static_Name' COMPLEX_FILE scale.x scale.y scale.z loc.x loc.y loc.z rot.x rot.y rot.z // World

	// Load Complex Mesh Details
	std::istringstream iss(cmesh_details);

	iss >> m_name >> m_file_name >>
		m_scale->x >> m_scale->y >> m_scale->z >>
		m_location->x >> m_location->y >> m_location->z >>
		rotation.x >> rotation.y >> rotation.z;

	std::cout << "Loading: " << m_name << " (" << m_file_name << ")" << std::endl;
	std::cout << "\tScale: " << m_scale->x << " " << m_scale->y << " " << m_scale->z << std::endl;
	std::cout << "\tLocation: " << m_location->x << " " << m_location->y << " " << m_location->z << std::endl;
	std::cout << "\tRotation: " << rotation.x << " " << rotation.y << " " << rotation.z << std::endl;

	m_rotation = new glm::quat(rotation);

	std::ifstream fb; // FileBuffer
	fb.open((m_file_name), std::ios::in);
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
			components->operator[](component_name) = new Component(component_name, LineBuf, scene_tracker);
		}
	}
	else
	{
		std::cerr << "ERROR: " << m_file_name << " Failed to open.\n";
	}
	fb.close();

	build_static_transform();
	computer_bounds();
	std::cerr << "Object Bounds: (" << m_lower_bounds.x << ", " << m_lower_bounds.y << ", " << m_lower_bounds.z << ") - (" << m_upper_bounds.x << ", " << m_upper_bounds.y << ", " << m_upper_bounds.z << ")\n" << std::endl;
}

Object::Object(std::string object_file_name, glm::quat rot, glm::vec3 loc, glm::vec3 scale, loadedComponents * scene_tracker)
{
	this->m_name = "Object";
	this->m_file_name = object_file_name;
	this->m_rotation = new glm::quat(rot);
	this->m_location = new glm::vec3(loc);
	this->m_scale = new glm::vec3(scale);
	this->scene_tracker = scene_tracker;

	components = new std::map<std::string, Component*>;

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
			components->operator[](component_name) = new Component(component_name, LineBuf, scene_tracker);
		}
	}
	else
	{
		std::cerr << "ERROR: " << object_file_name << " Failed to open.\n";
	}
	fb.close();

	build_static_transform();
	computer_bounds();
}

Object::~Object()
{
	for (auto &component : *components)
	{
		delete component.second;
	}
	components->clear();
	delete components;
	delete m_location;
	delete m_scale;
	delete m_rotation;
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
	//std::cout << "Object::Draw\n";
	GLuint objLoc = glGetUniformLocation(shader, "object");
	glUniformMatrix4fv(objLoc, 1, GL_FALSE, glm::value_ptr(m_transform));

	for (auto component : *components)
	{
		component.second->draw(shader);
	}
}

glm::vec3 * Object::getLocation()
{
	return this->m_location;
}

glm::vec3 Object::get_lower_bounds()
{
	return m_lower_bounds;

}

glm::vec3 Object::get_upper_bounds()
{
	return m_upper_bounds;
}

std::string Object::report_bounds()
{
	std::string report = "[" + std::to_string(m_lower_bounds.x) + ":" + std::to_string(m_lower_bounds.y) + ":" + std::to_string(m_lower_bounds.z) +
		"]-[" + std::to_string(m_upper_bounds.x) + ":" + std::to_string(m_upper_bounds.y) + ":" + std::to_string(m_upper_bounds.z) + "]";

	return report;
}

bool Object::is_collision(glm::vec3 lower_bound, glm::vec3 upper_bound)
{

	if (collision_check(lower_bound, upper_bound, m_lower_bounds, m_upper_bounds))
	{
		for (auto &component : *components)
		{
			if (component.second->is_collision(lower_bound, upper_bound, m_transform))
			{
				return true;
			}
		}
	}

	return false;

}

std::string Object::report()
{

	glm::vec3 rot;
	rot.y = asin(-2.0*(m_rotation->x*m_rotation->z - m_rotation->w*m_rotation->y));
	rot.x = atan2(2.0*(m_rotation->y*m_rotation->z + m_rotation->w*m_rotation->x), m_rotation->w*m_rotation->w - m_rotation->x*m_rotation->x - m_rotation->y*m_rotation->y + m_rotation->z*m_rotation->z);
	rot.z = atan2(2.0*(m_rotation->x*m_rotation->y + m_rotation->w*m_rotation->z), m_rotation->w*m_rotation->w + m_rotation->x*m_rotation->x - m_rotation->y*m_rotation->y - m_rotation->z*m_rotation->z);

	return m_file_name + "\t" +
		std::to_string(m_scale->x) + " " + std::to_string(m_scale->y) + " " + std::to_string(m_scale->z) + "\t" +
		std::to_string(m_location->x) + " " + std::to_string(m_location->y) + " " + std::to_string(m_location->z) + "\t" +
		std::to_string(rot.x) + " " + std::to_string(rot.y) + " " + std::to_string(rot.z) + "\n";
}

void Object::build_static_transform()
{
	m_transform = glm::mat4();
	m_transform = glm::translate(m_transform, *m_location);

	m_transform *= glm::toMat4(*m_rotation);

	m_transform = glm::scale(m_transform, *m_scale);

}

void Object::computer_bounds()
{
	m_lower_bounds = glm::vec3(std::numeric_limits<int>::max());
	m_upper_bounds = glm::vec3(std::numeric_limits<int>::min());

	for (auto component : *components)
	{
		// Lower Bounds
		glm::vec3 component_low = component.second->get_lower_bounds();

		m_lower_bounds.x = std::min(component_low.x, m_lower_bounds.x);
		m_lower_bounds.y = std::min(component_low.y, m_lower_bounds.y);
		m_lower_bounds.z = std::min(component_low.z, m_lower_bounds.z);

		// Upper Bounds
		glm::vec3 component_upper = component.second->get_upper_bounds();

		m_upper_bounds.x = std::max(component_upper.x, m_upper_bounds.x);
		m_upper_bounds.y = std::max(component_upper.y, m_upper_bounds.y);
		m_upper_bounds.z = std::max(component_upper.z, m_upper_bounds.z);
	}

	// Shift our upper and lower bounds by transform, then reasses each vertex for the new max and min positions
	glm::vec4 tmp_vec_L = glm::vec4(m_lower_bounds, 1.0);
	tmp_vec_L = m_transform * tmp_vec_L;

	glm::vec4 tmp_vec_H = glm::vec4(m_upper_bounds, 1.0);
	tmp_vec_H = m_transform * tmp_vec_H;

	m_lower_bounds = glm::vec3(glm::min(tmp_vec_L.x, tmp_vec_H.x), glm::min(tmp_vec_L.y, tmp_vec_H.y), glm::min(tmp_vec_L.z, tmp_vec_H.z));

	m_upper_bounds = glm::vec3(glm::max(tmp_vec_L.x, tmp_vec_H.x), glm::max(tmp_vec_L.y, tmp_vec_H.y), glm::max(tmp_vec_L.z, tmp_vec_H.z));

}

bool Object::collision_check(glm::vec3 player_lower_bound, glm::vec3 player_upper_bound, glm::vec3 mesh_lower_bound, glm::vec3 mesh_upper_bound)
{
	bool x_collision = (player_lower_bound.x >= mesh_lower_bound.x && player_lower_bound.x <= mesh_upper_bound.x) || // Within Bounds
		(player_lower_bound.x <= mesh_lower_bound.x && player_upper_bound.x >= mesh_lower_bound.x) || // Within Lower
		(player_lower_bound.x <= mesh_upper_bound.x && player_upper_bound.x >= mesh_upper_bound.x); // Within Hight

	bool y_collision = (player_lower_bound.y >= mesh_lower_bound.y && player_upper_bound.y <= mesh_upper_bound.y) || // Within Bounds
		(player_lower_bound.y <= mesh_lower_bound.y && player_upper_bound.y >= mesh_lower_bound.y) || // Within Lower
		(player_lower_bound.y <= mesh_upper_bound.y && player_upper_bound.y >= mesh_upper_bound.y); // Within Hight

	bool z_collision = (player_lower_bound.z >= mesh_lower_bound.z && player_upper_bound.z <= mesh_upper_bound.z) || // Within Bounds
		(player_lower_bound.z <= mesh_lower_bound.z && player_upper_bound.z >= mesh_lower_bound.z) || // Within Lower
		(player_lower_bound.z <= mesh_upper_bound.z && player_upper_bound.z >= mesh_upper_bound.z); // Within Hight

	return x_collision & y_collision & z_collision;
}
