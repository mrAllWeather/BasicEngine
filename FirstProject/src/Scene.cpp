#include "../include/Scene.h"

Scene::Scene(std::string scene_file)
{
	// Everything object in our scene
	statics = new std::map<std::string, ComplexMesh*>;
	// What we need to draw each frame
	scene_draw_list = new std::map<GLuint, std::vector< std::pair<ComplexMesh*, StaticMesh*> > >;
	// What we need to update each frame
	scene_tick_list = new std::vector< ComplexMesh*>;

	lights = new std::vector<Light*>;
	object_loader = new ObjLoader();
	shader_loader = new ShaderLoader();
	texture_loader = new TextureLoader();

	SceneLoader load_scene(scene_file, this);

	// Actor Key (This way we can add balls to scene without recompiling)
	std::string key = "Ball_";

	// Build Scene Draw List
	for (auto const mesh : *statics)
	{
		for(auto const component : (*(mesh.second)->components))
		{
			std::pair<ComplexMesh*, StaticMesh*> tmpPair;
			tmpPair.first = mesh.second;
			tmpPair.second = component.second;
			if (!scene_draw_list->count(component.second->shader_program)) // If the key is already in the map
			{
				scene_draw_list->emplace(std::make_pair(component.second->shader_program, std::vector< std::pair<ComplexMesh*, StaticMesh*> >()));
			}

			scene_draw_list->at(component.second->shader_program).push_back(tmpPair);

			// Add to tick if a ball of some description
			if (mesh.first.compare(0, key.size(), key) == 0 || mesh.first.compare("CueBall") == 0)
			{
				scene_tick_list->push_back(tmpPair.first);
			}
		}
	}

	update_projection();

}

Scene::~Scene()
{
	for (auto mesh : *statics) {
		removeStatic(mesh.first);
	}
	delete statics;
	delete object_loader;
	delete shader_loader;
	delete texture_loader;
}

bool Scene::attachStatic(std::string new_name, ComplexMesh * new_mesh)
{
	std::pair<std::string, ComplexMesh*> tmp_static = std::make_pair(new_name, new_mesh );
	this->statics->insert(tmp_static);


	for(auto const component : (*(tmp_static.second)->components))
	{
		std::pair<ComplexMesh*, StaticMesh*> tmpPair;
		tmpPair.first = new_mesh;
		tmpPair.second = component.second;
		if (!scene_draw_list->count(component.second->shader_program)) // If the key is not already in the map
		{
			scene_draw_list->emplace(std::make_pair(component.second->shader_program, std::vector< std::pair<ComplexMesh*, StaticMesh*> >()));
		}
		scene_draw_list->at(component.second->shader_program).push_back(tmpPair);
	}


	return true;
}

void Scene::setObject(std::string file_path)
{
	obj_loader_model* model_01 = new obj_loader_model;
	model_01->materials = new std::vector<tinyobj::material_t>;
	model_01->shapes = new std::vector<tinyobj::shape_t>;

	std::string err;
	tinyobj::LoadObj(*model_01->shapes, *model_01->materials, err, file_path.c_str(), "./Materials/");
	if (!err.empty())
	{
		std::cout << "Encountered error loading file: " << file_path << "\n" << err << std::endl;
	}

	// Populate Bounding Box
	for (auto shape : *model_01->shapes)
	{
		for (unsigned int i = 0; i < shape.mesh.positions.size() / 3; i += 3)
		{

			for (unsigned int axis = 0; axis < 3; axis++)
			{
				if (!model_01->bounds[0][axis] || model_01->bounds[0][axis] > shape.mesh.positions[i + axis])
				{
					model_01->bounds[0][axis] = shape.mesh.positions[i + axis];
				}

				if (!model_01->bounds[1][axis] || model_01->bounds[0][axis] < shape.mesh.positions[i + axis])
				{
					model_01->bounds[1][axis] = shape.mesh.positions[i + axis];
				}
			}
		}
	}

	std::cout << "Min:\t" << model_01->bounds[0][0] << ":" << model_01->bounds[0][1] << ":" << model_01->bounds[0][2] << std::endl;
	std::cout << "Max:\t" << model_01->bounds[1][0] << ":" << model_01->bounds[1][1] << ":" << model_01->bounds[1][2] << std::endl;
	float scale = 0;
	for (unsigned int axis = 0; axis < 3; axis++)
	{
		float diff = fabs(model_01->bounds[1][axis] - model_01->bounds[0][axis]);
		std::cout << diff << std::endl;
		if (diff > scale)
		{
			scale = diff;
		}
	}
	std::cout << "Scale:\t" << scale << std::endl;

	std::string min_report = "Min: " + std::to_string(model_01->bounds[0][0]) + ":" + std::to_string(model_01->bounds[0][1]) + ":" + std::to_string(model_01->bounds[0][2]);
	std::string max_report = "Max: " + std::to_string(model_01->bounds[1][0]) + ":" + std::to_string(model_01->bounds[1][1]) + ":" + std::to_string(model_01->bounds[1][2]);
	std::string scale_report = "Scale: " + std::to_string(scale);

}

void Scene::removeStatic(std::string static_name)
{
	delete (statics->at(static_name));
	statics->erase(static_name);
}

void Scene::draw()
{
	update_projection();

	for(auto shader_program : *scene_draw_list)
	{
		glUseProgram(shader_program.first);

		// Set up Light (only 1 for now)
		GLuint lightCount = glGetUniformLocation(shader_program.first, "lightCount");
		GLuint ambientStrength = glGetUniformLocation(shader_program.first, "ambientStrength");
		GLuint specularStrength = glGetUniformLocation(shader_program.first, "specularStrength");
		GLuint lightColor = glGetUniformLocation(shader_program.first, "lightColor");
		GLuint lightPos = glGetUniformLocation(shader_program.first, "lightPos");
		GLuint viewPosLoc = glGetUniformLocation(shader_program.first, "viewPos");

		glUniform1i(lightCount, lights->size());
		
		if(lights->size() > 0)
		{
			glUniform1f(ambientStrength, this->lights->at(0)->ambient_strength);
			glUniform3fv(lightColor, 1, glm::value_ptr(*this->lights->at(0)->color));
			glUniform3fv(lightPos, 1, glm::value_ptr(*this->lights->at(0)->location));
			glUniform3fv(viewPosLoc, 1, glm::value_ptr(this->camera->Position));
		}

		GLuint projectionLoc = glGetUniformLocation(shader_program.first, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection_transform));

		/*
		std::cout << "Projection matrix:\t";
		const float *pSource = (const float*)glm::value_ptr(projection_transform);
		for (int i = 0; i < 16; ++i)
			std::cout << pSource[i] << "\t";
		std::cout << std::endl;
		*/

		GLuint cameraLoc = glGetUniformLocation(shader_program.first, "view");
		glUniformMatrix4fv(cameraLoc, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));

		/*
		std::cout << "View matrix:\t";
		const float *vSource = (const float*)glm::value_ptr(camera->GetViewMatrix());
		for (int i = 0; i < 16; ++i)
			std::cout << vSource[i] << "\t";
		std::cout << std::endl;
		*/
		for(auto component : shader_program.second)
		{
			// Get component Specular Value
			glUniform1f(specularStrength, component.second->specular);

			GLuint modelLoc = glGetUniformLocation(shader_program.first, "model");
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(component.first->static_transform));
	
			for (unsigned int tex_num = 0; tex_num < component.second->textures.size() && tex_num < 32; tex_num++)
			{
				std::string texture_name = "texture_" + std::to_string(tex_num);
				glActiveTexture(GL_TEXTURE0+tex_num);
				glBindTexture(GL_TEXTURE_2D, component.second->textures.at(tex_num).second);
				glUniform1i(glGetUniformLocation(shader_program.first, texture_name.c_str()), tex_num);
			}
	
			GLuint componentLoc = glGetUniformLocation(shader_program.first, "component");
			glUniformMatrix4fv(componentLoc, 1, GL_FALSE, glm::value_ptr(component.second->component_transform));
		
			glBindVertexArray(*component.second->VAO);
			glDrawArrays(GL_TRIANGLES, 0, component.second->vertices);
			glBindVertexArray(0);
		}
	}
}

void Scene::tick(GLfloat delta)
{
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
	camera->tick();
}

void Scene::update_projection()
{
	projection_transform = glm::perspective(camera->Zoom, (float)s_WIDTH / (float)s_HEIGHT, 0.1f, 1000.0f);
}
