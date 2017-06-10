#include "../include/Scene.h"

Scene::Scene(std::string scene_file)
{
	// Everything object in our scene

	heightmap = nullptr;
    player    = nullptr;

	scene_tracker = new loadedComponents;

	objects = new std::map<std::string, Object*>;
	cameras = new std::map<std::string, Camera*>;

	lights = new std::map<std::string, Light*>;
	shader_loader = new ShaderLoader();

	SceneLoader load_scene(scene_file, this);

	std::cerr << "Scene Loader Finished\n";
	// Default to first light
	active_light = this->lights->begin()->second;
	active_camera = this->cameras->begin()->second;

	std::cerr << "Active Components Set\n";

	update_projection();

	std::cerr << "Projection Updated\n";

}

Scene::~Scene()
{
	for (auto object : *objects) {
		removeObject(object.first);
	}
}

void Scene::attachObject(std::string object_scene_name, glm::quat rot, glm::vec3 loc, glm::vec3 scale, std::string file_name, std::string base_dir)
{
	removeObject(object_scene_name);
	objects->operator[](object_scene_name) = new Object(object_scene_name, rot, loc, scale, scene_tracker);
}

void Scene::attachObject(std::string object_scene_name, std::string object_details)
{
	if (objects->find(object_scene_name) == objects->end())
	{
		objects->operator[](object_scene_name) = new Object(object_scene_name, object_details, scene_tracker);
	}

}

void Scene::attachShader(std::string shader_scene_name, std::string vertex_file, std::string fragment_file)
{
	if (scene_tracker->Shaders->find(shader_scene_name) != scene_tracker->Shaders->end())
	{
		glDeleteProgram(scene_tracker->Shaders->at(shader_scene_name));
		scene_tracker->Shaders->erase(shader_scene_name);
	}

	scene_tracker->Shaders->operator[](shader_scene_name) = shader_loader->build_program(std::make_pair(fragment_file, vertex_file));
}

void Scene::removeObject(std::string object_scene_name)
{
	if (objects->find(object_scene_name) != objects->end())
	{
		delete objects->at(object_scene_name);
	}
}

void Scene::draw()
{
	update_projection();

	glUseProgram(active_shader);

	// -- Light Uniforms --
	if (lights->size() > 0)
	{
		int light_idx = 0;
		for (auto &light : *lights)
		{
			Light* current_light = light.second;
			if (light_idx >= MAX_LIGHTS)
			{
				break;
			}

			std::string shader_light = "light[" + std::to_string(light_idx) + "].";

			GLuint hasLight = glGetUniformLocation(active_shader, (shader_light+"enabled").c_str());
			glUniform1i(hasLight, 1);

			GLuint light_type = glGetUniformLocation(active_shader, (shader_light + "type").c_str());
			glUniform1i(light_type, current_light->type);

			GLuint lightPos = glGetUniformLocation(active_shader, (shader_light + "position").c_str());
			glUniform3fv(lightPos, 1, glm::value_ptr(*current_light->location));

			GLuint lightDir = glGetUniformLocation(active_shader, (shader_light + "direction").c_str());
			glUniform3fv(lightDir, 1, glm::value_ptr(*current_light->direction));

			GLuint cut_off = glGetUniformLocation(active_shader, (shader_light + "cut_off").c_str());
			glUniform1f(cut_off, current_light->cut_off);

			GLuint outer_cut_off = glGetUniformLocation(active_shader, (shader_light + "cut_off").c_str());
			glUniform1f(outer_cut_off, current_light->outer_cut_off);

			GLuint constant = glGetUniformLocation(active_shader, (shader_light + "constant").c_str());
			glUniform1f(constant, current_light->constant);

			GLuint linear = glGetUniformLocation(active_shader, (shader_light + "linear").c_str());
			glUniform1f(linear, current_light->linear);

			GLuint quadratic = glGetUniformLocation(active_shader, (shader_light + "quadratic").c_str());
			glUniform1f(quadratic, current_light->quadratic);

			GLuint ambient_color = glGetUniformLocation(active_shader, (shader_light + "ambient").c_str());
			glUniform3fv(ambient_color, 1, glm::value_ptr(*current_light->ambient));

			GLuint specular_color = glGetUniformLocation(active_shader, (shader_light + "specular").c_str());
			glUniform3fv(specular_color, 1, glm::value_ptr(*current_light->specular));

			GLuint diffuse_color = glGetUniformLocation(active_shader, (shader_light + "diffuse").c_str());
			glUniform3fv(diffuse_color, 1, glm::value_ptr(*current_light->diffuse));

			// Increment light sources
			++light_idx;
		}
	}
	else
	{
		GLuint hasLight = glGetUniformLocation(active_shader, "light[0].enabled");
		glUniform1i(hasLight, 0);
	}

	// -- Camera Uniforms --
	GLuint viewPosLoc = glGetUniformLocation(active_shader, "viewPos");
	glUniform3fv(viewPosLoc, 1, glm::value_ptr(this->active_camera->Position));

	GLuint cameraLoc = glGetUniformLocation(active_shader, "view");
	glUniformMatrix4fv(cameraLoc, 1, GL_FALSE, glm::value_ptr(active_camera->GetViewMatrix()));

	// -- Scene Uniforms --
	GLuint projectionLoc = glGetUniformLocation(active_shader, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(m_transform));

	GLuint viewer_mode = glGetUniformLocation(active_shader, "view_mode");
	glUniform1i(viewer_mode, view_mode);

	// -- Draw Out Scene Components --
	for(auto object : *objects)
	{
		object.second->draw(active_shader);
	}

	if(heightmap)
    {
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
		heightmap->draw(active_shader);
        glDisable(GL_CULL_FACE);
    }

	if (player)
    {
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
		player->draw(active_shader);
        glDisable(GL_CULL_FACE);
    }

	// -- Turn out Lights back off --
	for (uint32_t light_idx = 0; (light_idx < lights->size() && light_idx < MAX_LIGHTS); ++light_idx)
	{
		std::string shader_light = "light[" + std::to_string(light_idx) + "].";
		GLuint hasLight = glGetUniformLocation(active_shader, (shader_light + "enabled").c_str());
		glUniform1i(hasLight, 0);
	}
}

void Scene::rendSky(){
	glUseProgram(active_shader);
	glm::mat4 v = glm::mat4(glm::mat3(active_camera->GetViewMatrix()));
	GLuint cameraLoc = glGetUniformLocation(active_shader, "view");
	glUniformMatrix4fv(cameraLoc, 1, GL_FALSE, glm::value_ptr(v));

	GLuint projectionLoc = glGetUniformLocation(active_shader, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(m_transform));

}

void Scene::tick(GLfloat delta)
{

	active_camera->tick();

	for (auto light : *lights)
	{
		light.second->tick(delta);
	}
}

void Scene::setActiveShader(std::string shader_scene_name)
{
	if (scene_tracker->Shaders->find(shader_scene_name) != scene_tracker->Shaders->end())
	{
		active_shader = scene_tracker->Shaders->at(shader_scene_name);
	}
	else
	{
		std::cout << "Shader " << shader_scene_name << " does not exist in scene.\n";
	}
}

void Scene::attachLight(std::string light_scene_name, std::string light_details)
{
	if (lights->find(light_scene_name) == lights->end())
	{
		lights->operator[](light_scene_name) = new Light(light_details);
	}
}

void Scene::setActiveLight(std::string light_scene_name)
{
	if (lights->find(light_scene_name) != lights->end())
	{
		active_light = lights->at(light_scene_name);
	}
	else if (light_scene_name == "NULL")
	{
		active_light = NULL;
	}
	else
	{
		std::cout << "Light " << light_scene_name << " does not exist in scene.\n";
	}
}

void Scene::attachCamera(std::string camera_scene_name, glm::vec3 location, glm::vec3 up, GLfloat yaw, GLfloat pitch)
{
	if (cameras->find(camera_scene_name) == cameras->end())
	{
		cameras->operator[](camera_scene_name) = new Camera(location, up, yaw, pitch);
	}
}

void Scene::setActiveCamera(std::string camera_scene_name)
{
	if (cameras->find(camera_scene_name) != cameras->end())
	{
		active_camera = cameras->at(camera_scene_name);
	}
	else
	{
		std::cout << "Camera " << camera_scene_name << " does not exist in scene.\n";
	}
}

Camera * Scene::getActiveCamera()
{
	return active_camera;
}

Light * Scene::getActiveLight()
{
	return active_light;
}

Light * Scene::getLight(std::string scene_light_name)
{
	if (lights->find(scene_light_name) != lights->end())
		return lights->find(scene_light_name)->second;

	std::cerr << "Light not found: " << scene_light_name << std::endl;

	return nullptr;
}

void Scene::setViewMode(GLuint mode)
{
	view_mode = mode;
}

bool Scene::hasObject(std::string scene_object_name)
{
	return (objects->find(scene_object_name) != objects->end());
}

Object * Scene::getObject(std::string scene_object_name)
{
	if (hasObject(scene_object_name))
    {
		return objects->at(scene_object_name);
    }
    return nullptr;
}

void Scene::attachPlayer(Component * object_pointer, bool * keyboard_input, bool * mouse_buttons, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
	removePlayer();
	player = new Player_Controller(object_pointer, keyboard_input, mouse_buttons, position, rotation, scale, scene_tracker, objects, heightmap);
}

void Scene::attachPlayer(std::string component_file_name, bool * keyboard_input, bool * mouse_buttons, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
	removePlayer();
	player = new Player_Controller(component_file_name, keyboard_input, mouse_buttons, position, rotation, scale, scene_tracker, objects, heightmap);
}

void Scene::removePlayer()
{
    if (player) {
        delete player;
    }
    player = nullptr;
}

bool Scene::hasPlayer()
{
	return player != nullptr;
}

Player_Controller * Scene::getPlayer()
{
	return player;
}

void Scene::update_projection()
{
	m_transform = glm::perspective(active_camera->Zoom, (float)s_WIDTH / (float)s_HEIGHT, 0.1f, 1000.0f);
}

void Scene::setHeightmap(std::string heightmap_file)
{
	if(heightmap == nullptr)
		delete heightmap;

	heightmap = new Heightmap("ground", heightmap_file, scene_tracker);
}

Heightmap* Scene::getHeightmap()
{
	return heightmap;
}
