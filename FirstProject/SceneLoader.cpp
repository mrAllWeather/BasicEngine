#include "SceneLoader.h"

SceneLoader::SceneLoader(std::string SceneFile, Scene* loading_scene)
{
	this->scene = loading_scene;
	this->scene_object_loader = loading_scene->object_loader;
	this->scene_shader_loader = loading_scene->shader_loader;
	
	std::ifstream fb; // FileBuffer

	std::cout << "Loading: " << (SceneFile) << std::endl;

	fb.open((SceneFile), std::ios::in);

	if (fb.is_open()) {
		std::string ObjectName = SceneFile; // Save Object Name
		std::string LineBuf;
		bool load_success = true;
		
		while (std::getline(fb, LineBuf)) {
			std::cout << LineBuf << std::endl;
			if (LineBuf == "Actors:")
			{
				load_success = BuildActors(&fb, &LineBuf);
			}
			else if (LineBuf == "Animations:")
			{
				load_success = BuildAnimations(&fb, &LineBuf);
			}
			else if (LineBuf == "Camera:")
			{
				load_success = BuildCamera(&fb, &LineBuf);
			}
			else if (LineBuf == "Lights:")
			{
				load_success = BuildLights(&fb, &LineBuf);
			}
			else if (LineBuf == "SceneName:")
			{
				load_success = BuildSceneName(&fb, &LineBuf);
			}
			else if (LineBuf == "Skybox:")
			{
				load_success = BuildSkybox(&fb, &LineBuf);
			}
			else if (LineBuf == "Statics:")
			{
				std::streampos last_line = fb.tellg();

				std::cout << "Loading Static:" << LineBuf << std::endl;
				while (std::getline(fb, LineBuf) && std::regex_match(LineBuf, std::regex(CMESH_REGEX)))
				{
					ComplexMesh* c_mesh = new ComplexMesh(LineBuf, this->scene_shader_loader, this->scene_object_loader);
					this->scene->statics->operator[](c_mesh->name) = c_mesh;
					last_line = fb.tellg();
				}

				fb.seekg(last_line);
			}
			else if (!load_success)
			{
				std::cout << "Scene Failed to Load" << std::endl;
				exit(-1);
			}
		}
		std::cout << "Scene Built!\n";
	}
	else
	{
		std::cout << "Scene Failed to load!" << std::endl;
	}
}

bool SceneLoader::BuildActors(std::ifstream* fb, std::string* LineBuf)
{
	return true;
}

bool SceneLoader::BuildAnimations(std::ifstream* fb, std::string* LineBuf)
{
	return true;
}

bool SceneLoader::BuildCamera(std::ifstream* fb, std::string* LineBuf)
{
	return true;
}

bool SceneLoader::BuildLights(std::ifstream* fb, std::string* LineBuf)
{
	return true;
}

bool SceneLoader::BuildSceneName(std::ifstream* fb, std::string* LineBuf)
{
	std::getline(*fb, *LineBuf);
	this->scene->scene_name = *LineBuf;
	std::cout << "Scene name: " << this->scene->scene_name << std::endl;
	return true;
}

bool SceneLoader::BuildSkybox(std::ifstream* fb, std::string* LineBuf)
{
	return true;
}

bool SceneLoader::BuildStatics(std::ifstream* fb, std::string* LineBuf)
{


	return true;
}
