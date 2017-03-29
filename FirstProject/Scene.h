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
#include "ComplexMesh.h"
#include "Camera.h"

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

	Scene();
	~Scene();
	
	bool attachStatic(std::string, ComplexMesh*);
	
	void removeStatic(std::string); // Chance to Complex in the long run
	void draw();
	void tick(GLfloat delta); // Update All Actors

	// TODO
	// bool attachActor();
	// uint64_t attachLight();
	// void removeActor(Actor*);
	// void removeLight();
	// void setSkybox(void); // Pass texture?
private:
	std::string scene_name;
	std::map<std::string, ComplexMesh*>* statics;
	ObjLoader* object_loader;
	ShaderLoader* shader_loader;

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