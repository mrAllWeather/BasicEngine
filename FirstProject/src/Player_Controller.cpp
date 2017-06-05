#include "../include/Player_Controller.h"

#include <fstream>
Player_Controller::Player_Controller(Component * component_pointer, bool * keyboard_input, bool * mouse_buttons, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, loadedComponents * scene_tracker, std::map<std::string, Object*>* objects, Heightmap* heightmap)
{
	// Scene Details
	this->objects = objects;
	this->heightmap = heightmap;
	this->scene_tracker = scene_tracker;

	this->m_limits.x = LIMIT_X;
	this->m_limits.y = LIMIT_Y;
	this->m_limits.z = LIMIT_Z;

	this->m_location = position;
	this->m_scale = scale;
	this->m_rotation = glm::quat(rotation);

	set_model(component_pointer);

	build_static_transform();
	computer_bounds();
}

Player_Controller::Player_Controller(std::string component_file_name, bool * keyboard_input, bool * mouse_buttons, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, loadedComponents * scene_tracker, std::map<std::string, Object*>* objects, Heightmap* heightmap)
{
	// Scene Details
	this->objects = objects;
	this->heightmap = heightmap;
	this->scene_tracker = scene_tracker;

	this->m_limits.x = LIMIT_X;
	this->m_limits.y = LIMIT_Y;
	this->m_limits.z = LIMIT_Z;

	this->m_scale = scale;
	this->m_location = position;
	this->m_rotation = glm::quat(rotation);

	set_create_model(component_file_name);
	
	build_static_transform();
	computer_bounds();
}

void Player_Controller::set_model(Component * component_pointer)
{
	// TODO: ADD DESTRUCTION BOOLEAN. ARE WE DESTROYING OLD CONTAINER OR JUST SWAPPING
	if (player_model)
	{
		delete player_model;
	}
	// Any rotations, shifts, or scales can come from the player controller's values
	player_model = component_pointer;

	//TODO : Get our newly possessed objects location
}

void Player_Controller::set_create_model(std::string component_file_name)
{
	if (player_model)
	{
		delete player_model;
	}
	// Any rotations, shifts, or scales can come from the player controller's values
	player_model = new Component("Player_Body", component_file_name, glm::quat(), glm::vec3(), glm::vec3(), scene_tracker);
}

void Player_Controller::draw(GLuint shader)
{
	//std::cout << "Object::Draw\n";
	GLuint objLoc = glGetUniformLocation(shader, "object");
	glUniformMatrix4fv(objLoc, 1, GL_FALSE, glm::value_ptr(m_transform));

	if (player_model)
	{
		player_model->draw(shader);
	}
	
}

void Player_Controller::ProcessKeyboard(GLfloat deltaTime)
{
	// Forwards
	if (keyboard_input[GLFW_KEY_W])
	{
		m_velocity += m_forward*m_acceleration*deltaTime;
	}
	// Backwards (Move backwards at half speed)
	if (keyboard_input[GLFW_KEY_S])
	{
		m_velocity -= m_forward*(m_acceleration * static_cast<float>(0.5))*deltaTime;
	}
	// Left
	if (keyboard_input[GLFW_KEY_A])
	{
		m_velocity -= m_right*m_acceleration*deltaTime;
	}
	// Right
	if (keyboard_input[GLFW_KEY_D])
	{
		m_velocity += m_right*m_acceleration*deltaTime;
	}

	if (m_velocity.x > m_limits.x || m_velocity.x < -m_limits.x)
	{
		m_velocity.x = glm::min(m_limits.x, m_velocity.x);
		m_velocity.x = glm::max(m_limits.x, - m_velocity.x);
	}
	if (m_velocity.y > m_limits.y || m_velocity.y < -m_limits.y)
	{
		m_velocity.y = glm::min(m_limits.y, m_velocity.y);
		m_velocity.y = glm::max(m_limits.y, -m_velocity.y);
	}
	if (m_velocity.z > m_limits.z || m_velocity.z < -m_limits.z)
	{
		m_velocity.z = glm::min(m_limits.z, m_velocity.z);
		m_velocity.z = glm::max(m_limits.z, -m_velocity.z);
	}
}

void Player_Controller::ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch)
{
	// TODO: If Mouse_2 held, rotate forward vector

}

void Player_Controller::tick(GLfloat delta)
{
	m_location += m_velocity * delta;

	// if(m_location.y > heightmap->get_image_value(m_location.x, m_location.z, 0))
}

glm::vec3 Player_Controller::get_lower_bounds()
{
	return m_lower_bounds;
}

glm::vec3 Player_Controller::get_upper_bounds()
{
	return m_upper_bounds;
}

glm::vec3 * Player_Controller::get_location()
{
	return &m_location;
}

void Player_Controller::build_static_transform()
{
	m_transform = glm::mat4();
	m_transform = glm::translate(m_transform, m_location);

	m_transform *= glm::toMat4(m_rotation);

	m_transform = glm::scale(m_transform, m_scale);
}

void Player_Controller::computer_bounds()
{
	// Lower
	glm::vec4 tmp_vec = glm::vec4(player_model->get_lower_bounds(), 1.0);
	tmp_vec = m_transform * tmp_vec;
	m_lower_bounds = glm::vec3(tmp_vec.x, tmp_vec.y, tmp_vec.z);

	// Upper
	tmp_vec = glm::vec4(player_model->get_upper_bounds(), 1.0);
	tmp_vec = m_transform * tmp_vec;
	m_upper_bounds = glm::vec3(tmp_vec.x, tmp_vec.y, tmp_vec.z);
}

