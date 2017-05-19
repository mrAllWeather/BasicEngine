#pragma once
/* Author: Ben Weatherall
 * Description: Scene Loader; intended to read a scene file and then populat a Scene object
 * with the relevant objects. A number of Circumstances are not currently handled.
 * Currently handles Camera (only first), Light (only first), Statics and SceneName
*/

#define _CRT_SECURE_NO_WARNINGS
#include <regex>
#include <string>
#include <iostream>
#include <istream>

#include "../include/Scene.h"
#include "../include/ComplexMesh.h"
#include "../include/TextureLoader.h"
#include "../include/Light.h"

class Scene; // TODO find out why I am using parent references

class SceneLoader
{
public:
	SceneLoader(std::string scene_file, Scene* loading_scene);
private:
	Scene * scene;
	ShaderLoader * scene_shader_loader;
	ObjLoader * scene_object_loader;
	TextureLoader * scene_texture_loader;
	bool BuildActors(std::ifstream* fb, std::string* LineBuf);
	bool BuildAnimations(std::ifstream* fb, std::string* LineBuf);
	bool BuildCamera(std::ifstream* fb, std::string* LineBuf);
	bool BuildLights(std::ifstream* fb, std::string* LineBuf);
	bool BuildSceneName(std::ifstream* fb, std::string* LineBuf);
	bool BuildSkybox(std::ifstream* fb, std::string* LineBuf);
	bool BuildStatics(std::ifstream* fb, std::string* LineBuf);
};
