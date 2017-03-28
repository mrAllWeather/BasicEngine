#include "SceneLoader.h"

SceneLoader::SceneLoader(std::string SceneFile, Scene* loading_scene)
{
	this->scene = loading_scene;
	
	std::ifstream fb; // FileBuffer

	std::cout << "Loading: " << (SceneFile) << std::endl;

	fb.open((SceneFile), std::ios::in);

	if (fb.is_open()) {
		std::string ObjectName = SceneFile; // Save Object Name
		std::string LineBuf;
		bool load_success = true;

		while (std::getline(fb, LineBuf)) {
			if (LineBuf == "Actors")
			{
				load_success = BuildActors(&fb, LineBuf);
			}
			if (LineBuf == "Animations")
			{
				load_success = BuildAnimations(&fb, LineBuf);
			}
			if (LineBuf == "Camera:")
			{
				load_success = BuildCamera(&fb, LineBuf);
			}
			if (LineBuf == "Lights:")
			{
				load_success = BuildLights(&fb, LineBuf);
			}
			if (LineBuf == "SceneName:")
			{
				load_success = BuildSceneName(&fb, LineBuf);
			}
			
			if (LineBuf == "Skybox:")
			{
				load_success = BuildSkybox(&fb, LineBuf);
			}
			if (LineBuf == "Statics")
			{
				load_success = BuildStatics(&fb, LineBuf);
			}
			if (!load_success)
			{
				std::cout << "Scene Failed to Load" << std::endl;
				exit(-1);
			}
		}
	}
	else
	{
		std::cout << "Scene Failed to load!" << std::endl;
	}
}

bool SceneLoader::BuildActors(std::ifstream* fb, std::string LineBuf)
{
	return true;
}

bool SceneLoader::BuildAnimations(std::ifstream* fb, std::string LineBuf)
{
	return true;
}

bool SceneLoader::BuildCamera(std::ifstream* fb, std::string LineBuf)
{
	return true;
}

bool SceneLoader::BuildLights(std::ifstream* fb, std::string LineBuf)
{
	return true;
}

bool SceneLoader::BuildSceneName(std::ifstream* fb, std::string LineBuf)
{
	std::getline(*fb, LineBuf);
	std::cout << LineBuf << ":" << this->scene << std::endl;
	this->scene->scene_name = LineBuf;
	return true;
}

bool SceneLoader::BuildSkybox(std::ifstream* fb, std::string LineBuf)
{
	return true;
}

bool SceneLoader::BuildStatics(std::ifstream* fb, std::string LineBuf)
{
		
	std::regex static_regex(CMESH_REGEX); 
	std::streampos last_line = fb->tellg();
	std::getline(*fb, LineBuf);

	while (std::regex_match(LineBuf, static_regex))
	{
		ComplexMesh* c_mesh = new ComplexMesh(LineBuf, this->scene_shader_loader, this->scene_object_loader);
	}

	return true;
}
