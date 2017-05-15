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
#include "../include/ObjLoader.h"
#include "../include/TextureLoader.h"
#include "../include/SceneLoader.h"
#include "../include/ComplexMesh.h"
#include "../include/Camera.h"
#include "../include/Light.h"
#include "../include/Mesh.h"

//TODO going to add height and width values for window. This is a poor choice! 
// Intend to fix in long run
const GLuint s_WIDTH = 1024, s_HEIGHT = 768;

struct technician {
	Camera* camera;
	//std::vector<Lights*> lights
};

// TODO TMP STRUCT 
struct obj_loader_model {
	std::vector<tinyobj::shape_t>* shapes;
	std::vector<tinyobj::material_t>* materials;
	glm::vec3 bounds[2];
};

class Scene {
public:
	friend class SceneLoader;
	friend class Actor;
	friend class ComplexMesh;
	friend class StaticMesh;

	Scene(std::string scene_file);
	~Scene();
	
	// Add Static (maybe rename?) will add a complex mesh, build all components
	// and then add the each components ComplexMesh*, StaticMesh* paid to our
	// scene_draw_list
	void attachObject(std::string object_scene_name, std::string file_name, std::string base_dir = "");
	void attachShader(std::string shader_scene_name, std::string vertex_file, std::string fragment_file);
	// Remove Static wil remove each component from the scene_draw_list and
	// then call delete on the ComplexMesh*
	void removeStatic(std::string); // Chance to Complex in the long run

	void draw();

	void tick(GLfloat delta); // Update All Actors

	void setActiveShader(std::string);
	void setViewMode(GLuint);
	// TODO
	// bool attachActor();
	// uint64_t attachLight();
	// void removeActor(Actor*);
	// void removeLight();
	// void setSkybox(void); // Pass texture?
	Camera* camera;
	std::map<std::string, Mesh*>* meshes;
	std::map<std::string, GLuint> textures;
	std::map<std::string, GLuint> shaders;
private:
	GLuint view_mode;
	GLuint active_shader;
	Light* active_light;
	std::string scene_name;
	std::vector<Light*>* lights;

	ObjLoader* object_loader;
	ShaderLoader* shader_loader;
	TextureLoader* texture_loader;
	glm::mat4 projection_transform;

	void update_projection();
	// We store every static/actor component by shader program. We can then call
	// the stored values we need for the draw call. As we are calling from the scene
	// We also have access to our Camera and Lighting.

	// std::map<GLuint, std::vector< std::pair<ComplexMesh*, StaticMesh*> > >* scene_draw_list;
	// std::vector< ComplexMesh*>* scene_tick_list;
	
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
