#include "../include/SceneLoader.h"

SceneLoader::SceneLoader(std::string SceneFile, Scene* loading_scene)
{
	this->scene = loading_scene;
	this->scene_shader_loader = loading_scene->shader_loader;
	
	std::ifstream fb; // FileBuffer

	std::cout << "Loading: " << (SceneFile) << std::endl;

	fb.open((SceneFile), std::ios::in);

	if (fb.is_open()) {
		std::string ObjectName = SceneFile; // Save Object Name
		std::string LineBuf;
		bool load_success = true;
		
		while (std::getline(fb, LineBuf)) {

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
				std::cout << LineBuf << std::endl;
				std::streampos last_line = fb.tellg();
				
				std::string name;
				glm::vec3 location, up;
				GLfloat yaw, pitch;

				// TODO set Camera_REGEX (Tho' Light regex will probably work for now :P)
				while (std::getline(fb, LineBuf) && std::regex_match(LineBuf, std::regex(LIGHT_REGEX)))
				{
					std::stringstream iss(LineBuf);

					iss >> name >> location.x >> location.y >> location.z >>
						up.x >> up.y >> up.z >>
						yaw >> pitch;

					std::cout << "\tLocation: " << location.x << " " << location.y << " " << location.z;
					std::cout << "\n\tUp:" << up.x << " " << up.y << " " << up.z;
					std::cout << "\n\tYaw: " << yaw << "\tPitch: " << pitch << std::endl;

					this->scene->attachCamera(name, location, up, yaw, pitch);

				}
				fb.seekg(last_line);

			}
			else if (LineBuf == "Heightmap:")
			{
				std::streampos last_line = fb.tellg();

				if (std::getline(fb, LineBuf) && std::regex_match(LineBuf, std::regex(LIGHT_REGEX)))
				{
					// Clean Up leading spaces
					LineBuf = std::regex_replace(LineBuf, std::regex("^[ \t]+"), "");
					std::cout << LineBuf << std::endl;
					scene->setHeightmap(LineBuf);

					last_line = fb.tellg();
				}

				fb.seekg(last_line);
				
			}
			else if (LineBuf == "Lights:")
			{
				std::cout << LineBuf << std::endl;
				std::streampos last_line = fb.tellg();

				while (std::getline(fb, LineBuf) && std::regex_match(LineBuf, std::regex(LIGHT_REGEX)))
				{
					std::stringstream iss(LineBuf);
					std::string name;
					iss >> name;

					scene->attachLight(name, iss.str());

					last_line = fb.tellg();
				}

				fb.seekg(last_line);

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
				std::cout << LineBuf << std::endl;
				std::streampos last_line = fb.tellg();

				while (std::getline(fb, LineBuf) && std::regex_match(LineBuf, std::regex(LIGHT_REGEX)))
				{
					std::stringstream iss(LineBuf);
					std::string name;
					iss >> name;

					scene->attachObject(name, iss.str());

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
		exit(-1);
	}
	fb.close();
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
