#pragma once
/* Author: Ben Weatherall (a1617712)
 * Description: Point light class to be used within scene. 
 * Provides details of the location, strength and color of light.
*/

#define _CRT_SECURE_NO_WARNINGS
#include <cstdlib>
#include <regex>
#include <string>
#include <iostream>
#include <sstream>
#define _USE_MATH_DEFINES
#include <math.h>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// GLM
#include <glm/glm.hpp>

#define LIGHT_REGEX "\\t.*"

enum light_types{
	lDIRECTIONAL,
	lPOINT,
	lSPOT
};

class Light {
public:
	Light(	std::string light_details );
	~Light();

	std::string get_name();
	
	void tick(GLfloat delta);

	void circle_location(glm::vec3* location, float radius, glm::vec3 offset);
	void stop_circling();
	void attach_light(glm::vec3* location, glm::vec3* direction);
	void detach_light();
	void watch_location(glm::vec3* location);
	void stop_watching();

	std::string name;

	glm::vec3* ambient;
	glm::vec3* diffuse;
	glm::vec3* specular;

	// Light Radius Values (Spotlight)
	GLfloat cut_off;
	GLfloat outer_cut_off;

	// Point Light / Spotlight
	glm::vec3* location; 

	// Direction Light / Spotlight
	glm::vec3* direction; 

	// Attenuation values (Defaulting to a light of 65 meters)
	float constant = 1.0;
	float linear = 0.07;
	float quadratic = 0.017;

	light_types type;
private:
	glm::vec3* circle_focus;
	// Point Light / Spotlight
	glm::vec3* old_location;

	// Direction Light / Spotlight
	glm::vec3* old_direction;

	// Watch location
	glm::vec3* watch_point;

	float circle_radius;
	glm::vec3 offset;

	bool attached = false;
	bool circling = false;
	bool watching = false;
	float circle_theta = M_PI / 2;
	float circle_phi = 0;
};
