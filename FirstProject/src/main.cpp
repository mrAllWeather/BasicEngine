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
#include "../include/tiny_obj_loader.h"
#include "../include/File_IO.h"
#include "../include/Skybox.h"

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

// View Mode
glm::vec3 origin(0.0f);

int inspection_mode = 0;
int lighting_mode = 0;
bool is_fully_rendered = true; // False = Inspection Mode, True = Lighting Mode
GLenum fill_mode = GL_FILL;

// View / Focus Swapping
const double VIEW_SWAP_DELAY = 0.5;	// Only swap view this many times per second
double time_since_last_swap = 0; // How long since we last swapped view focus

// Callbacks
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void Keyboard_Input(float deltaTime);

int SKYBOX_TRIS = 36;
bool SHOW_FPS = false;

int main()
{
	double start_time = glfwGetTime();

	// Init GLFW
	glfwInit();
	// Set GLFW req options
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
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
	current_level = new Scene("./Scenes/Test.scene");
	// current_level = new Scene("./Scenes/Chopper.scene");

	// Attaching Scene Shaders (Move into Level.scene).
	current_level->attachShader("Debug", "./Shaders/debug.vert", "./Shaders/debug.frag");
	current_level->attachShader("Light-Texture", "./Shaders/light-texture.vert", "./Shaders/light-texture.frag");
	current_level->attachShader("Skybox", "./Shaders/skybox.vert", "./Shaders/skybox.frag");
	
	// Defaulting to active lighting
	current_level->setActiveShader("Light-Texture");

	// Lighting
	// current_level->getLight("CamLight")->attach_light(&current_level->getActiveCamera()->Position, &current_level->getActiveCamera()->Front);
	current_level->getLight("RotateLight")->circle_location(&origin, 25.0, origin);

	current_level->attachPlayer("./Meshes/Assign_3/cube-tex.obj", keys, mouse_button, glm::vec3(0, 1, 0), origin, glm::vec3(0.2, 0.2, 0.2));

	// If a character is present, attach camera there. Otherwise circle origin
	if (current_level->hasPlayer())
	{
		current_level->getActiveCamera()->SetCircleFocus(current_level->getPlayer()->get_location(), 2, glm::vec3(0,1,0));
		current_level->getActiveCamera()->SetLookFocus(current_level->getPlayer()->get_location());
	}
	else
	{
		current_level->getActiveCamera()->SetCircleFocus(&origin, 2, origin);
		current_level->getActiveCamera()->SetLookFocus(&origin);
	}
	Skybox* sky = new Skybox();

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
		current_level->setActiveShader("Light-Texture");
		lastFrame = currentFrame;
		currentFrame = glfwGetTime();
		delta = currentFrame - lastFrame;

		// FPS Report
		spf_report->tick();

		// Check and call events
		glfwPollEvents();

		// Player Input
		// System Controls
		Keyboard_Input(delta);
		// Character Controls
		if (current_level->hasPlayer())
			current_level->getPlayer()->tick(delta);

		// Update Level
		current_level->tick(delta);

		// Rendering Commands here
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glPolygonMode(GL_FRONT_AND_BACK, fill_mode);

		current_level->draw();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		
		current_level->setActiveShader("Skybox");
		
		current_level->rendSky();
			
		glDepthFunc(GL_LEQUAL);
		glBindVertexArray(sky->skyVaoId);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, sky->skyTexId);
		glDrawArrays(GL_TRIANGLES, 0, SKYBOX_TRIS);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		// Swap Buffers
		glfwSwapBuffers(window);

	}

	glfwTerminate();
	return 0;
}

// Moves/alters the camera positions based on user input
void Keyboard_Input(float deltaTime)
{
    // Keeping this input there as we may use it later.
    (void)deltaTime;
	// Inspection Mode
	if (keys[GLFW_KEY_2])
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
				std::cout << "Wire Frame\n";
			}
			time_since_last_swap = glfwGetTime();
			current_level->setViewMode(inspection_mode);
		}
	}
	// Switch between Lighting / Inspection Mode
	if(keys[GLFW_KEY_1])
	{
		if (glfwGetTime() - time_since_last_swap > VIEW_SWAP_DELAY)
		{
			if (is_fully_rendered == false) // If Currently Debug
			{
				current_level->setActiveShader("Light-Texture");
				fill_mode = GL_FILL;
				lighting_mode = 0;
				current_level->setActiveLight("Overhead");
				std::cout << "Lighting Mode\n";

			}
			else
			{
				current_level->setActiveShader("Debug");
				inspection_mode = 0;
				fill_mode = GL_LINE;
				std::cout << "Debug Mode\n";
			}

			is_fully_rendered = !is_fully_rendered;

			time_since_last_swap = glfwGetTime();
		}
	}
}

// Capture Input via Callbacks
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // Silencing warnings for unused vars but needed for OpenGL
    (void)scancode;
    (void)mode;
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
    // Silencing warnings for unused vars but needed for OpenGL
    (void)window;
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

	// std::cout << "xoffset: " << xoffset << "\tmod: " << xoffset * MOUSE_SPEED << std::endl;

	// Camera is locked to an actor: lets circle him with mouse
	current_level->getActiveCamera()->CircleObject(xoffset * MOUSE_SPEED, yoffset * MOUSE_SPEED);

	if (mouse_button[GLFW_MOUSE_BUTTON_RIGHT])
	{
		// On right click update front vector (direction)
		current_level->getPlayer()->setForwardVector(current_level->getActiveCamera()->Front);
	}

}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Silencing warnings for unused vars but needed for OpenGL
    (void)window;
    (void)mods;
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
