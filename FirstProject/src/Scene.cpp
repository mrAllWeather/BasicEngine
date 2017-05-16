#include "../include/Scene.h"

Scene::Scene(std::string scene_file)
{
	// Everything object in our scene
	// statics = new std::map<std::string, ComplexMesh*>;
	// What we need to draw each frame
	// scene_draw_list = new std::map<GLuint, std::vector< std::pair<ComplexMesh*, StaticMesh*> > >;
	// What we need to update each frame
	// scene_tick_list = new std::vector< ComplexMesh*>;

	meshes = new std::map<std::string, Mesh*>;

	lights = new std::map<std::string, Light*>;
	object_loader = new ObjLoader();
	shader_loader = new ShaderLoader();
	texture_loader = new TextureLoader();
	
	SceneLoader load_scene(scene_file, this);

	// Default to first light
	active_light = this->lights->begin()->second;

	update_projection();

}

Scene::~Scene()
{
	/*
	for (auto mesh : *statics) {
		removeStatic(mesh.first);
	}
	delete statics;
	delete object_loader;
	delete shader_loader;
	delete texture_loader;
	*/
}

void Scene::attachObject(std::string object_scene_name, std::string file_name, std::string base_dir)
{
	if (meshes->find(object_scene_name) != meshes->end())
	{
		delete meshes->at(object_scene_name);
	}

	Mesh* loading_mesh;
	if (base_dir != "")
		loading_mesh = new Mesh(file_name, textures, base_dir);
	else
		loading_mesh = new Mesh(file_name, textures, base_dir);

	if (loading_mesh->loaded_successfully)
	{
		meshes->operator[](object_scene_name) = loading_mesh;
	}
}

void Scene::attachShader(std::string shader_scene_name, std::string vertex_file, std::string fragment_file)
{
	if (shaders.find(shader_scene_name) != shaders.end())
	{
		glDeleteProgram(shaders.at(shader_scene_name));
		shaders.erase(shader_scene_name);
	}

	shaders[shader_scene_name] = shader_loader->build_program(std::make_pair(fragment_file, vertex_file));
}

void Scene::removeStatic(std::string static_name)
{
	/*
	delete (statics->at(static_name));
	statics->erase(static_name);
	*/
}

void Scene::draw()
{
	update_projection();

	glUseProgram(active_shader);

	// Scene Uniforms
	// Set up Light (only 1 for now)
	GLuint hasLight = glGetUniformLocation(active_shader, "light.active");
	GLuint ambientStrength = glGetUniformLocation(active_shader, "light.ambientStrength");
	GLuint specularStrength = glGetUniformLocation(active_shader, "light.specularStrength");
	GLuint lightColor = glGetUniformLocation(active_shader, "light.diffuse");
	GLuint lightPos = glGetUniformLocation(active_shader, "light.position");
	
	glUniform1i(hasLight, lights->size());

	glUniform1f(ambientStrength, active_light->ambient_strength);
	glUniform3fv(lightColor, 1, glm::value_ptr(*active_light->color));
	glUniform3fv(lightPos, 1, glm::value_ptr(*active_light->location));

	GLuint viewPosLoc = glGetUniformLocation(active_shader, "viewPos");
	glUniform3fv(viewPosLoc, 1, glm::value_ptr(this->camera->Position));

	GLuint projectionLoc = glGetUniformLocation(active_shader, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection_transform));

	GLuint cameraLoc = glGetUniformLocation(active_shader, "view");
	glUniformMatrix4fv(cameraLoc, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));

	GLuint viewer_mode = glGetUniformLocation(active_shader, "view_mode");
	glUniform1i(viewer_mode, view_mode);

	for(auto object : *meshes)
	{
		object.second->draw(active_shader);
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

	camera->tick();
}

void Scene::setActiveShader(std::string shader_scene_name)
{
	if (shaders.find(shader_scene_name) != shaders.end())
	{
		active_shader = shaders.at(shader_scene_name);
	}
	else
	{
		std::cout << "Shader " << shader_scene_name << " does not exist in scene.\n";
	}
}

void Scene::setActiveLight(std::string light_scene_name)
{
	if (lights->find(light_scene_name) != lights->end())
	{
		active_light = lights->at(light_scene_name);
	}
	else
	{
		std::cout << "Light " << light_scene_name << " does not exist in scene.\n";
	}
}

Light * Scene::getActiveLight()
{
	return active_light;
}

void Scene::setViewMode(GLuint mode)
{
	view_mode = mode;
}

void Scene::update_projection()
{
	projection_transform = glm::perspective(camera->Zoom, (float)s_WIDTH / (float)s_HEIGHT, 0.1f, 1000.0f);
}
