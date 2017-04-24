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

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Seconds_Per_Frame_Counter.h"	// Handy way to check performance
#include "Camera.h"						// Our game camera class
#include "SceneLoader.h"
#include "Bouncer.h"

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


// Game Mode
const glm::vec3 CUE_FORCE = glm::vec3(1.0);	// How strong we hit the cue ball
const double VIEW_SWAP_DELAY = 1;	// Only swap view focus (<TAB>) once every this many seconds

Bouncer* gamemode;
bool is_look_ball = true;	// Are we looking at the cue ball or the table?
glm::vec3* origin = new glm::vec3(0.0); // Our table is living at origin but has a slight offset due to the model. Origin lets us circle without a weird offset
double time_since_last_circle_swap = 0; // How long since we last swapped view focus


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

	// Clear Color
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

	// Load Scene
	current_level = new Scene("./Scenes/Billiards.scene");
	camera = current_level->camera;

	// Load Gamemode
	gamemode = new Bouncer(current_level, 0.05, glm::vec3(2.07, 0, 0.95), 0.5);
	gamemode->initialise();

	// Configure our look at and circling
	camera->SetCircleFocus(current_level->statics->at("CueBall")->location, 1.0, camera->Position);
	camera->SetLookFocus(current_level->statics->at("CueBall")->location);

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
		gamemode->update(delta);
		current_level->tick(delta);

		// Rendering Commands here
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		current_level->draw();

		// Swap Buffers
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

// Moves/alters the camera positions based on user input
void Keyboard_Input(float deltaTime)
{
	// Use Pool Cue
	if (keys[GLFW_KEY_SPACE])
	{
		gamemode->strike(0, (glm::normalize(camera->Front) * CUE_FORCE));
	}
	// Switch between circling (Ball or Table)
	if (keys[GLFW_KEY_TAB])
	{
		if (glfwGetTime() - time_since_last_circle_swap > VIEW_SWAP_DELAY)
		{
			if (is_look_ball)
			{
				camera->SetCircleFocus(origin, 3.5, glm::vec3(0, 1.5, 0));
			}
			else
			{
				camera->SetCircleFocus(current_level->statics->at("CueBall")->location, 1.0, glm::vec3(0, 1.0, 0));
			}
			is_look_ball = !is_look_ball;
			time_since_last_circle_swap = glfwGetTime();
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