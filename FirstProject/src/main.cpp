/*	LEARNING BRANCH:
	Author: Ben Weatherall
	Description: Increasingly complex OpenGL program aiming to develop my skills and then
	assist with course projects.
	Current aims: Implement Camera, Scene Component and Complex Mesh Handling
	Future aims: Inverse Kinematics animation system utilising weighting
*/

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cmath>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/Seconds_Per_Frame_Counter.h"	// Handy way to check performance
#include "../include/Camera.h"						// Our game camera class
#include "../include/SceneLoader.h"
#include "../include/RenderText.h"
#include "../include/tiny_obj_loader.h"

// Window Dimensions
// const GLuint WIDTH = 1024, HEIGHT = 768;
const GLuint WIDTH = 1280, HEIGHT = 1024;
bool firstMouse = true;
GLfloat lastX = 400, lastY = 300;

// Controls
const float MOUSE_SPEED = 0.01; // How fast the camera should rotate around our focus object
bool keys[1024];
bool mouse_button[8];

// Scene Components
Scene* current_level;
Camera* camera;

// View Mode
int inspection_mode = 0;
int lighting_mode = 0;
bool is_fully_rendered; // False = Inspection Mode, True = Lighting Mode
GLuint active_program, lighting_program, inspection_program;


// View / Focus Swapping
const double VIEW_SWAP_DELAY = 1;	// Only swap view focus (<TAB>) once every this many seconds
double time_since_last_swap = 0; // How long since we last swapped view focus


// Callbacks
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void Keyboard_Input(float deltaTime);

bool SHOW_FPS = false;

// TODO TMP STRUCT 
struct obj_loader_model {
	std::vector<tinyobj::shape_t>* shapes;
	std::vector<tinyobj::material_t>* materials;
	glm::vec3 bounds[2];
};

int main(int argc, char* argv[])
{
	double start_time = glfwGetTime();

	// Init GLFW
	glfwInit();
	// Set GLFW req options
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create window
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Complex Mesh Project", nullptr, nullptr); // Window 1
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// I.E This is where the following actions shall occur
	glfwMakeContextCurrent(window);

	// Initialise GLEW
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialise GLEW" << std::endl;
		exit(-1);
	}

	// Set required callbacks: Guessing this becomes a large switch which will then be converted into a hash (after all you always want rebindable keys!)
	// Following from that; I wonder how one does context... State machine?
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set up viewport
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Set OpenGL Options
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Clear Color
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

	// Load Scene
	current_level = new Scene("./Scenes/Blank.scene");
	camera = current_level->camera;
	RenderText ui_text(WIDTH, HEIGHT);

	// Load Gamemode
	obj_loader_model* model_01 = new obj_loader_model;
	model_01->materials = new std::vector<tinyobj::material_t>;
	model_01->shapes = new std::vector<tinyobj::shape_t>;

	if (argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " File.obj" << std::endl;
		return -1;
	}
	else
	{
		std::string err;
		tinyobj::LoadObj(*model_01->shapes, *model_01->materials, err, argv[1], "./Materials/");
		if (!err.empty())
		{
			std::cout << "Encountered error loading file: " << argv[1] << "\n" << err << std::endl;
		}
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

	// gamemode->initialise();

	// Configure our look at and circling
	// camera->SetCircleFocus(current_level->statics->at("CueBall")->location, 1.0, camera->Position);
	// camera->SetLookFocus(current_level->statics->at("CueBall")->location);

	// Initialise Seconds per Frame counter
	SPF_Counter* spf_report;
	spf_report = new SPF_Counter(SHOW_FPS);

	// Set up delta tracking
	double delta, lastFrame, currentFrame = glfwGetTime();

	// How long did loading take? (Plants take up close to 4 seconds!)
	std::cout << "Loaded after " << (currentFrame - start_time) << " seconds.\n";
	
	// Program Loop	
	while (!glfwWindowShouldClose(window))
	{
		// Frame Delta
		lastFrame = currentFrame;
		currentFrame = glfwGetTime();
		delta = currentFrame - lastFrame;

		// FPS Report
		spf_report->tick();

		// Player Input
		Keyboard_Input(delta);

		// Check and call events
		glfwPollEvents();

		// Update Level
		// gamemode->update(delta);
		current_level->tick(delta);

		// Rendering Commands here
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		current_level->draw();

		ui_text.DrawString(min_report, 15.0f, HEIGHT - 15.0f, 0.25f, glm::vec3(0.5, 0.8f, 0.2f));
		ui_text.DrawString(max_report, 15.0f, HEIGHT - 30.0f, 0.25f, glm::vec3(0.5, 0.8f, 0.2f));
		ui_text.DrawString(scale_report, 15.0f, HEIGHT - 45.0f, 0.25f, glm::vec3(0.5, 0.8f, 0.2f));

		// Swap Buffers
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

// Moves/alters the camera positions based on user input
void Keyboard_Input(float deltaTime)
{
	// Inspection Mode
	if (keys[GLFW_KEY_D])
	{
		if (glfwGetTime() - time_since_last_swap > VIEW_SWAP_DELAY)
		{
			inspection_mode++;
			if (inspection_mode > 0)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			else if(inspection_mode > 2)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				inspection_mode = 0;
			}
			time_since_last_swap = glfwGetTime();
		}
	}
	// Lighting Mode
	if (keys[GLFW_KEY_L])
	{
		if (glfwGetTime() - time_since_last_swap > VIEW_SWAP_DELAY)
		{
			lighting_mode++;
			if (lighting_mode > 2)
			{
				lighting_mode = 0;
			}
			time_since_last_swap = glfwGetTime();
		}
	}
	// Switch between Lighting / Inspection Mode
	if(keys[GLFW_KEY_S])
	{
		if (glfwGetTime() - time_since_last_swap > VIEW_SWAP_DELAY)
		{
			if (is_fully_rendered == false) // If Currently Debug
				active_program = lighting_program;
			else
				active_program = inspection_program;
			time_since_last_swap = glfwGetTime();
		}
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// When user presses the escape key, we set the windowShouldClose property to true,
	// closing the application
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	lastX = xpos;

	if (mouse_button[GLFW_MOUSE_BUTTON_LEFT])
	{
		// std::cout << "xoffset: " << xoffset << "\tmod: " << xoffset * MOUSE_SPEED << std::endl;

		// We only wish to circle left and right, Not up and down thus 0.0 for yoffset
		camera->CircleObject(xoffset * MOUSE_SPEED, 0.0);
	}

	if (mouse_button[GLFW_MOUSE_BUTTON_RIGHT])
	{

	}

}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button >= 0 && button <= 8)
	{
		if (action == GLFW_PRESS)
		{
			mouse_button[button] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			mouse_button[button] = false;
		}
	}
}
