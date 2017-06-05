#include "../include/Player_Controller.h"

#include <fstream>
Player_Controller::Player_Controller(Component * component_pointer, bool * keyboard_input, bool * mouse_buttons, glm::vec3 position, glm::vec3 rotation, loadedComponents * scene_tracker)
{
}

Player_Controller::Player_Controller(std::string component_file_name, bool * keyboard_input, bool * mouse_buttons, glm::vec3 position, glm::vec3 rotation, loadedComponents * scene_tracker)
{
	this->scene_tracker = scene_tracker;

	this->m_location = position;
	this->m_rotation = glm::quat(rotation);

	// TODO We'll keep this at 1 for now, review later
	this->m_scale = glm::vec3(1);

	std::ifstream fb; // FileBuffer
	fb.open((component_file_name), std::ios::in);
	std::string LineBuf, component_name;
	std::stringstream ss;


	
	
	build_static_transform();
	computer_bounds();
}

void Player_Controller::set_create_model(std::string object_file_name)
{
	if(player_model)
		// TODO remove player model
	
	player_model = new Component(object_file_name, LineBuf, scene_tracker);
}

