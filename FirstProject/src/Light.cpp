#include "../include/Light.h"

Light::Light( std::string light_details ) 
{
	this->location = new glm::vec3;
	this->color = new glm::vec3;

	// Load Component
	std::istringstream iss(light_details);

	iss >> name >>	
		location->x >> location->y >> location->z >>
		color->x >> color->y >> color->z >>
		ambient_strength;
}

Light::~Light()
{
}

std::string Light::get_name()
{
	return name;
}

void Light::tick()
{
	// pass
}
