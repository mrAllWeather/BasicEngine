#include "../include/Light.h"

Light::Light( std::string light_details )
{
	// Set up local pointers
	this->location = new glm::vec3;
	this->direction = new glm::vec3;
	this->ambient = new glm::vec3;
	this->diffuse = new glm::vec3;
	this->specular = new glm::vec3;

	// Load Component
	std::istringstream iss(light_details);
	std::istringstream stream_2;

	int l_type;

	iss >> name >> l_type;


	switch ((light_types)l_type)
	{
	case lDIRECTIONAL:
		this->type = lDIRECTIONAL;
		iss >> direction->x >> direction->y >> direction->z >>
			ambient->x >> ambient->y >> ambient->z >>
			diffuse->x >> diffuse->y >> diffuse->z >>
			specular->x >> specular->y >> specular->z;
			linear = 0;
			quadratic = 0;
			cut_off = 0;
			outer_cut_off = 0;
		break;
	case lPOINT:
		this->type = lPOINT;
		iss >> location->x >> location->y >> location->z >>
			ambient->x >> ambient->y >> ambient->z >>
			diffuse->x >> diffuse->y >> diffuse->z >>
			specular->x >> specular->y >> specular->z >>
			linear >> quadratic;
			cut_off = 0;
			outer_cut_off = 0;
		break;
	case lSPOT:
		this->type = lSPOT;
		iss >> location->x >> location->y >> location->z >>
			direction->x >> direction->y >> direction->z >>
			ambient->x >> ambient->y >> ambient->z >>
			diffuse->x >> diffuse->y >> diffuse->z >>
			specular->x >> specular->y >> specular->z >>
			linear >> quadratic >> cut_off >> outer_cut_off;
		break;
	default:
		break;
	}
	std::cout << name <<
		"\n\tLocation: " << location->x << ":" << location->y << ":" << location->z <<
		"\n\tDirection: " << direction->x << ":" << direction->y << ":" << direction->z <<
		"\n\tAmbient: " << ambient->x << ":" << ambient->y << ":" << ambient->z <<
		"\n\tDiffuse: " << diffuse->x << ":" << diffuse->y << ":" << diffuse->z <<
		"\n\tSpecular: " << specular->x << ":" << specular->y << ":" << specular->z <<
		"\n\tLinear: " << linear << "\tQuadratic: " << quadratic <<
		"\n\tCutoff: " << cut_off << "\tOuter Cutoff: " << outer_cut_off << std::endl;
}

Light::~Light()
{
}

std::string Light::get_name()
{
	return name;
}

void Light::tick(GLfloat delta)
{
	if (circling)
	{
		// We wish to circle the focused object at a set distance
		// https://www.opengl.org/discussion_boards/showthread.php/176504-Orbit-around-object
		this->circle_phi = this->circle_phi + delta; // horizontal movement

		// We take a world offset to allow us control over where the camera is in relation to the object before circling

		this->location->x = this->offset.z + this->circle_focus->x + circle_radius*glm::cos(this->circle_phi)*glm::sin(this->circle_theta);
		this->location->z = this->offset.z + this->circle_focus->z + circle_radius*glm::sin(this->circle_phi)*glm::sin(this->circle_theta);
		this->location->y = this->offset.y + this->circle_focus->y + circle_radius*glm::cos(this->circle_theta);

	}
}

void Light::circle_location(glm::vec3 * circle_location, float radius, glm::vec3 position_offset)
{
	std::cout << "Start Circling\n";
	if (circling)
		stop_circling();
	if (attached)
		detach_light();

	old_location = this->location;

	this->circle_focus = circle_location;
	this->offset = position_offset;
	this->circle_radius = radius;

	circling = true;

	tick(0.0);
}

void Light::stop_circling()
{
	if (circling)
	{
		this->location = old_location;
		this->circle_radius = 0;
		this->offset = glm::vec3(0);
		circling = false;
	}
}

void Light::attach_light(glm::vec3 * attach_location, glm::vec3 * attach_direction)
{
	if (circling)
		stop_circling();
	if (attached)
		detach_light();

	old_location = this->location;
	old_direction = this->direction;

	this->location = attach_location;
	this->direction = attach_direction;

	attached = true;
}

void Light::detach_light()
{
	if (attached)
	{
		location = old_location;
		direction = old_direction;
		attached = false;
	}
}

void Light::watch_location(glm::vec3* location)
{
	if (watching)
		stop_watching();

	watching = true;
	watch_point = location;
}

void Light::stop_watching()
{
	if (watching)
	{
		watch_point = nullptr;
		watching = false;
	}
}

std::string Light::report()
{
	std::string report;
	report += type + "\t";

	switch (type)
	{
	case lDIRECTIONAL:
		report += std::to_string(direction->x) + " " + std::to_string(direction->y) + " " + std::to_string(direction->z) + "\t" +
			std::to_string(ambient->x) + " " + std::to_string(ambient->y) + " " + std::to_string(ambient->z) + "\t" +
			std::to_string(diffuse->x) + " " + std::to_string(diffuse->y) + " " + std::to_string(diffuse->z) + "\t" +
			std::to_string(specular->x) + " " + std::to_string(specular->y) + " " + std::to_string(specular->z);
		break;
	case lPOINT:
		report += std::to_string(location->x) + " " + std::to_string(location->y) + " " + std::to_string(location->z) + "\t" +
			std::to_string(ambient->x) + " " + std::to_string(ambient->y) + " " + std::to_string(ambient->z) + "\t" +
			std::to_string(diffuse->x) + " " + std::to_string(diffuse->y) + " " + std::to_string(diffuse->z) + "\t" +
			std::to_string(specular->x) + " " + std::to_string(specular->y) + " " + std::to_string(specular->z) + "\t" +
			std::to_string(linear) + " " + std::to_string(quadratic);
		break;
	case lSPOT:
		report += std::to_string(location->x) + " " + std::to_string(location->y) + " " + std::to_string(location->z) + "\t" +
			std::to_string(direction->x) + " " + std::to_string(direction->y) + " " + std::to_string(direction->z) + "\t" +
			std::to_string(ambient->x) + " " + std::to_string(ambient->y) + " " + std::to_string(ambient->z) + "\t" +
			std::to_string(diffuse->x) + " " + std::to_string(diffuse->y) + " " + std::to_string(diffuse->z) + "\t" +
			std::to_string(specular->x) + " " + std::to_string(specular->y) + " " + std::to_string(specular->z) + "\t" +
			std::to_string(linear) + " " + std::to_string(quadratic) + " " + std::to_string(cut_off) + " " + std::to_string(outer_cut_off);
		break;
	default:
		break;
	}


	return report += "\n";
}
