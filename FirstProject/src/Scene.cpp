#include "../include/Scene.h"

Scene::Scene(std::string scene_file)
{
	// Everything object in our scene
	
	scene_tracker = new loadedComponents;

	objects = new std::map<std::string, Object*>;
	cameras = new std::map<std::string, Camera*>;
	
	lights = new std::map<std::string, Light*>;
	shader_loader = new ShaderLoader();
	
	SceneLoader load_scene(scene_file, this);

	// Default to first light
	active_light = this->lights->begin()->second;
	active_camera = this->cameras->begin()->second;

	update_projection();

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
		objects->operator[](object_scene_name) = new Object(object_details, scene_tracker);
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

void Scene::draw(GLfloat delta)
{
	update_projection();

	glUseProgram(active_shader);

	// -- Light Uniforms -- 
	if (active_light)
	{
		GLuint hasLight = glGetUniformLocation(active_shader, "light.active");
		glUniform1i(hasLight, 1);

		GLuint light_type = glGetUniformLocation(active_shader, "light.type");
		glUniform1i(light_type, active_light->type);

		GLuint lightPos = glGetUniformLocation(active_shader, "light.position");
		glUniform3fv(lightPos, 1, glm::value_ptr(*active_light->location));

		GLuint lightDir = glGetUniformLocation(active_shader, "light.direction");
		glUniform3fv(lightDir, 1, glm::value_ptr(*active_light->direction));

		GLuint cut_off = glGetUniformLocation(active_shader, "light.cut_off");
		glUniform1f(cut_off, active_light->cut_off);

		GLuint outer_cut_off = glGetUniformLocation(active_shader, "light.outer_cut_off");
		glUniform1f(cut_off, active_light->outer_cut_off);

		GLuint constant = glGetUniformLocation(active_shader, "light.constant");
		glUniform1f(constant, active_light->constant);

		GLuint linear = glGetUniformLocation(active_shader, "light.linear");
		glUniform1f(linear, active_light->linear);

		GLuint quadratic = glGetUniformLocation(active_shader, "light.quadratic");
		glUniform1f(quadratic, active_light->quadratic);

		GLuint ambient_color = glGetUniformLocation(active_shader, "light.ambient");
		glUniform3fv(ambient_color, 1, glm::value_ptr(*active_light->ambient));

		GLuint specular_color = glGetUniformLocation(active_shader, "light.specular");
		glUniform3fv(specular_color, 1, glm::value_ptr(*active_light->specular));

		GLuint diffuse_color = glGetUniformLocation(active_shader, "light.diffuse");
		glUniform3fv(diffuse_color, 1, glm::value_ptr(*active_light->diffuse));
	}
	else
	{
		GLuint hasLight = glGetUniformLocation(active_shader, "light.active");
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

	for(auto object : *objects)
	{
		object.second->draw(delta);
	}
}

void Scene::tick(GLfloat delta)
{
	// TODO Transform Light position and orientation within scene (for spinning light)
	/*
	for (auto mesh : *scene_tick_list)
	{
		// Build Complex Transform (actor moving around world) (Ball Moving on table)
		mesh->build_static_transform();
		for (auto component : (*mesh->components))
		{
			// Build Component Transform (component moving around actor) (Ball Rolling)
			component.second->build_component_transform();
		}
	}
	*/

	active_camera->tick();
	if(active_light)
		active_light->tick(delta);
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
		return objects->at(scene_object_name);
}


void Scene::update_projection()
{
	m_transform = glm::perspective(active_camera->Zoom, (float)s_WIDTH / (float)s_HEIGHT, 0.1f, 1000.0f);
}
