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
	glUniform3fv(viewPosLoc, 1, glm::value_ptr(this->camera->Position));

	GLuint cameraLoc = glGetUniformLocation(active_shader, "view");
	glUniformMatrix4fv(cameraLoc, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));

	// -- Scene Uniforms --
	GLuint projectionLoc = glGetUniformLocation(active_shader, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection_transform));

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
	if(active_light)
		active_light->tick(delta);
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
	else if (light_scene_name == "NULL")
	{
		active_light = NULL;
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

void Scene::update_projection()
{
	projection_transform = glm::perspective(camera->Zoom, (float)s_WIDTH / (float)s_HEIGHT, 0.1f, 1000.0f);
}
