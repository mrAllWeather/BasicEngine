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
#include "../include/File_IO.h"

// Window Dimensions
const GLuint WIDTH = 1024, HEIGHT = 768;
// const GLuint WIDTH = 1280, HEIGHT = 1024;
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
glm::vec3 origin(0.0f);
bool show_details = false;
bool student_note = true;

int inspection_mode = 0;
int lighting_mode = 0;
bool is_fully_rendered; // False = Inspection Mode, True = Lighting Mode
GLenum fill_mode = GL_LINE;

// View / Focus Swapping
const double VIEW_SWAP_DELAY = 0.5;	// Only swap view this many times per second
double time_since_last_swap = 0; // How long since we last swapped view focus

void show_ui(RenderText);

// Callbacks
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void Keyboard_Input(float deltaTime);

bool SHOW_FPS = false;

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
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ben Weatherall - a1617712 (Assignment 3 Part 1)", nullptr, nullptr); // Window 1
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
	RenderText ui_text(WIDTH, HEIGHT);

	// Attaching Scene Shaders (Move into Level.scene).
	current_level->attachShader("Debug", "./Shaders/debug.vert", "./Shaders/debug.frag");
	current_level->attachShader("Light-Texture", "./Shaders/light-texture.vert", "./Shaders/light-texture.frag");
	// Default to First Shader?
	current_level->setActiveShader("Debug");

	// We can attach lights to locations and set up circling.
	current_level->getLight("CamLight")->attach_light(&current_level->getActiveCamera()->Position, &origin);
	current_level->getLight("RotateLight")->circle_location(&origin, 10.0, origin);

	// Configure our look at and circling
	current_level->getActiveCamera()->SetCircleFocus(&origin, 2, origin);
	current_level->getActiveCamera()->SetLookFocus(&origin);

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
		current_level->tick(delta);

		// Rendering Commands here
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glPolygonMode(GL_FRONT_AND_BACK, fill_mode);

		current_level->draw(delta);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Add UI to screen (only text atm)
		show_ui(ui_text);

		// Swap Buffers
		glfwSwapBuffers(window);

		// Hide notes for assessor after 30 seconds (they get in the way)
		if (currentFrame - start_time > 30)
			student_note = false;
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
		if (glfwGetTime() - time_since_last_swap > VIEW_SWAP_DELAY && !is_fully_rendered)
		{
			inspection_mode++;
			if (inspection_mode > 0)
			{
				fill_mode = GL_FILL;
			}

			if(inspection_mode > 2)
			{
				fill_mode = GL_LINE;
				inspection_mode = 0;
			}
			time_since_last_swap = glfwGetTime();
			current_level->setViewMode(inspection_mode);
		}
	}
	// Lighting Mode
	if (keys[GLFW_KEY_L])
	{
		if (glfwGetTime() - time_since_last_swap > VIEW_SWAP_DELAY && is_fully_rendered)
		{
			lighting_mode++;
			if (lighting_mode > 3)
			{
				lighting_mode = 0;
			}
			switch (lighting_mode)
			{
			case 0:
				current_level->setActiveLight("Overhead");
				break;
			case 1:
				current_level->setActiveLight("CamLight");
				break;
			case 2:
				current_level->setActiveLight("RotateLight");
				break;
			case 3:
				current_level->setActiveLight("NULL");
				break;
			default:
				break;
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
			{
				current_level->setActiveShader("Light-Texture");
				fill_mode = GL_FILL;
				lighting_mode = 0;
				current_level->setActiveLight("Overhead");

			}
			else
			{
				current_level->setActiveShader("Debug");
				inspection_mode = 0;
				fill_mode = GL_LINE;
			}

			is_fully_rendered = !is_fully_rendered;

			time_since_last_swap = glfwGetTime();
		}
	}
	if (keys[GLFW_KEY_GRAVE_ACCENT])
	{
		if (glfwGetTime() - time_since_last_swap > VIEW_SWAP_DELAY)
		{
			show_details = !show_details;
			time_since_last_swap = glfwGetTime();
		}
	}
}

void show_ui(RenderText ui_text)
{
	if (student_note)
	{
		ui_text.DrawString("Sorry, I am generating my own normals instead of setting them to (0,0,0)", 15.0f, 30.0f, 0.25f, glm::vec3(0.5, 0.8f, 0.2f));
		ui_text.DrawString("Assignment makes heavy use of suggested resources OpenGlTutorials and Tiny_Obj_Loader's example viewer.cc (available with latest version of loader)", 15.0f, 15.0f, 0.25f, glm::vec3(0.5, 0.8f, 0.2f));
	}
	if (show_details)
	{
		/* TODO: Add bounds calls for Component and Object. Component will use Mesh * transform, Object will use Object.Bounds * transform (tho' consider using vector of bounds at Object for collision)
		ui_text.DrawString("Press ` to hide details", 15.0f, HEIGHT - 15.0f, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
		std::string bounds = "Bounds: [" + current_level->getObject("model_01")->get_lower_bounds() +
			"]-[" + current_level->getObject("model_01")->get_upper_bounds() + "]";
		ui_text.DrawString(bounds, 15.0f, HEIGHT - 30.0f, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));

		ui_text.DrawString("Scale: 1:" + current_level->meshes->at("model_01")->get_scale(), 15.0f, HEIGHT - 45.0f, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
		*/
		std::string drawmode;
		if (is_fully_rendered)
		{
			drawmode = "Fully Rendered";
		}
		else
		{
			switch (inspection_mode)
			{
			case 0:
				drawmode = "Wire Frame";
				break;
			case 1:
				drawmode = "Normals";
				break;
			case 2:
				drawmode = "Diffuse colour";
			default:
				break;
			}
		}
		ui_text.DrawString("Draw Mode: " + drawmode, 15.0f, HEIGHT - 60.0f, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));

		ui_text.DrawString("Camera: " + std::to_string(current_level->getActiveCamera()->Position.x) + ":" +
			std::to_string(current_level->getActiveCamera()->Position.y) + ":" +
			std::to_string(current_level->getActiveCamera()->Position.z),
			15.0f, HEIGHT - 75.0f, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));

		if (is_fully_rendered)
		{
			if (lighting_mode < 3) 
			{
				ui_text.DrawString("Active Light: " + current_level->getActiveLight()->get_name(), 15.0f, HEIGHT - 90.0f, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
				ui_text.DrawString("Light Location: " +
				std::to_string(current_level->getActiveLight()->location->x) + ":" +
				std::to_string(current_level->getActiveLight()->location->y) + ":" +
				std::to_string(current_level->getActiveLight()->location->z),
				15.0f, HEIGHT - 105.0f, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));

			}
			else
			{
				ui_text.DrawString("Unlit", 15.0f, HEIGHT - 90.0f, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
			}
		}
	}
	else
	{
		ui_text.DrawString("Press ` for details", 15.0f, HEIGHT - 15.0f, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
		ui_text.DrawString("Press S to switch between shaders", 15.0f, HEIGHT - 30.0f, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
		if (is_fully_rendered)
			ui_text.DrawString("Press L to switch active lights", 15.0f, HEIGHT - 45.0f, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
		else
			ui_text.DrawString("Press D to switch debug modes", 15.0f, HEIGHT - 45.0f, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
		
		ui_text.DrawString("Hold Left mouse and drag to rotate model", 15.0f, HEIGHT - 60.0f, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
		ui_text.DrawString("Hold Right mouse and drag to zoom in and out", 15.0f, HEIGHT - 75.0f, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
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
	GLfloat yoffset = ypos - lastY;
	lastX = xpos;
	lastY = ypos;

	if (mouse_button[GLFW_MOUSE_BUTTON_LEFT])
	{
		// std::cout << "xoffset: " << xoffset << "\tmod: " << xoffset * MOUSE_SPEED << std::endl;

		// We only wish to circle left and right, Not up and down thus 0.0 for yoffset
		camera->CircleObject(xoffset * MOUSE_SPEED, yoffset * MOUSE_SPEED);
	}

	if (mouse_button[GLFW_MOUSE_BUTTON_RIGHT])
	{
		// TIL that Zoom works the opposite to the way I thought, high numbers is zoomed out, low numbers zoomed in
		camera->Zoom += xoffset * MOUSE_SPEED;
		if (camera->Zoom > 3)
			camera->Zoom = 3;
		if (camera->Zoom < 0.1)
			camera->Zoom = 0.1;
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
