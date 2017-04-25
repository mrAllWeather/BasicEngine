#pragma once
/* Author: Ben Weatherall (a1617712)
 * Description: Container class used to handle collection of component meshes into singular object
 * Handles world coordination and placement. Allows component meshes to be referenced by name
 * opening the way for basic animations.
*/

#define _CRT_SECURE_NO_WARNINGS
#define CMESH_REGEX "\\t\\w* .* ([\\d\\.f]*( )?){9}"

#include <map>
#include <string>
#include <fstream>
#include <sstream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "StaticMesh.h"

// This does need to be heavily reviewed for rights
// Everything is currently public as we give up our details constantly to pretty much everyone
class ComplexMesh {
public:
	friend class StaticMesh;
	ComplexMesh(	std::string cmesh_details, 
			ShaderLoader* scene_shader_loader, 
			ObjLoader* scene_object_loader,
			TextureLoader* scene_texture_loader);

	std::string name;
	// List of all out components (by name so we can call them for local transforms if needed)
	std::map<std::string, StaticMesh*>* components;
	glm::vec3* location;
	glm::vec3* rotation;
	glm::vec3* scale;
	glm::mat4 static_transform;
	glm::quat* rot;
	void build_static_transform();

private:
};
