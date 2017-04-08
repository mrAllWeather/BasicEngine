#pragma once
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

#include "ShaderLoader.h"
#include "ObjLoader.h"
#include "TextureLoader.h"
#include "SceneLoader.h"
#include "ComplexMesh.h"
#include "Camera.h"

//TODO going to add height and width values for window. This is a poor choice! 
// Intend to fix in long run
const GLuint s_WIDTH = 1024, s_HEIGHT = 768;

struct technician {
	Camera* camera;
	//std::vector<Lights*> lights
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
	bool attachStatic(std::string, ComplexMesh*);
	
	// Remove Static wil remove each component from the scene_draw_list and
	// then call delete on the ComplexMesh*
	void removeStatic(std::string); // Chance to Complex in the long run

	void draw();

	void tick(GLfloat delta); // Update All Actors

	// TODO
	// bool attachActor();
	// uint64_t attachLight();
	// void removeActor(Actor*);
	// void removeLight();
	// void setSkybox(void); // Pass texture?
	Camera* camera;
private:
	std::string scene_name;
	std::map<std::string, ComplexMesh*>* statics;
	ObjLoader* object_loader;
	ShaderLoader* shader_loader;
	TextureLoader* texture_loader;
	glm::mat4 projection_transform;

	void update_projection();
	// We store every static/actor component by shader program. We can then call
	// the stored values we need for the draw call. As we are calling from the scene
	// We also have access to our Camera and Lighting.
	std::map<GLuint, std::vector< std::pair<ComplexMesh*, StaticMesh*> > >* scene_draw_list;
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
