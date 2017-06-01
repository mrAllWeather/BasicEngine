#pragma once
/* Author: Ben Weatherall (a1617712) 
 * Description: Scene class to handle graphical side of levels
 * Contains all meshes, lights, and cameras for the scene. Tracks a series of loader
 * which can reduce duplicate loading. Handles graphic changes per frame via the tick function
*/


// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include<string>
#include<vector>

#include "../include/ShaderLoader.h"
#include "../include/SceneLoader.h"
#include "../include/Object.h"
#include "../include/Camera.h"
#include "../include/Light.h"
#include "../include/Mesh.h"
#include "../include/Heightmap.h"

//TODO going to add height and width values for window. This is a poor choice! 
// Intend to fix in long run
const GLuint s_WIDTH = 1024, s_HEIGHT = 768;

class Scene {

public:
	friend class SceneLoader;
	friend class Actor;
	friend class Object;
	friend class Component;

	Scene(std::string scene_file);
	~Scene();
	
	// Add Static (maybe rename?) will add a complex mesh, build all components
	// and then add the each components ComplexMesh*, StaticMesh* paid to our
	// scene_draw_list
	void attachObject(std::string object_scene_name, glm::quat rot, glm::vec3 loc, glm::vec3 scale, std::string file_name, std::string base_dir = "");
	void attachObject(std::string object_scene_name, std::string object_details);
	void removeObject(std::string object_scene_name);

	void attachShader(std::string shader_scene_name, std::string vertex_file, std::string fragment_file);

	void draw();

	void tick(GLfloat delta); // Update All Actors

	void setActiveShader(std::string);
	
	Light* getLight(std::string);

	void attachLight(std::string light_scene_name, std::string light_details);
	void setActiveLight(std::string);
	Light* getActiveLight();

	void attachCamera(std::string, glm::vec3 location, glm::vec3 up, GLfloat yaw, GLfloat pitch);
	void setActiveCamera(std::string);
	Camera* getActiveCamera();

	void setHeightmap(std::string);
	Heightmap* getHeightmap();
	
	
	void setViewMode(GLuint);

	bool hasObject(std::string);
	Object* getObject(std::string);

	// TODO
	// bool attachActor();
	// uint64_t attachLight();
	// void removeActor(Actor*);
	// void removeLight();
	// void setSkybox(void); // Pass texture?

private:
	void update_projection();

	std::string scene_name;
	std::map<std::string, Object*>* objects;
	std::map<std::string, Light*>* lights;
	std::map<std::string, Camera*>* cameras;

	Heightmap* heightmap;

	glm::mat4 m_transform;

	// Active Components
	GLuint active_shader;
	Camera* active_camera;
	// TODO we want all lights; so replace this
	Light* active_light;

	// TODO Shader Switch; Consider using to switch effects on and off
	GLuint view_mode;


	loadedComponents* scene_tracker;
	
	ShaderLoader* shader_loader;








	// TODO with Actors we will have a tick list
	// std::vector< Actor*>* scene_tick_list;
	


	// TODO: Create the following
	// SkyBox
	// Ambient Lighting
	// Lights Vector
	// std::vector<ComplexMesh> Static;
	// std::vector<Actor> Actors;
	// Actors Vector
		// Complex shapes with Update()
	// AnimationTimeline
		// Per Actor Animation
		// Interpolate between initial rotation and final rotation
		// Determine change per tick
		// Allow animation layering?
	// Height map()?
};
