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
const GLuint WIDTH = 1024, HEIGHT = 768;
bool firstMouse = true;
GLfloat lastX = 400, lastY = 300;


// Controls
bool keys[1024];
bool mouse_button[8];
float MOUSE_SPEED = 0.01;

// Scene Components
Scene* current_level;
Camera* camera;

// Game Mode
Bouncer* gamemode;
glm::vec3 cue_force = glm::vec3(1.0);


// Callbacks
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void Keyboard_Input(float deltaTime);


int main(int argc, char* argv[])
{
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

	// Billiard's table has an offset due to how the table was designed, so instead we are using origin as our focus
	glm::vec3* origin = new glm::vec3(0.0);
	
	// Configure our look at and circling
	camera->SetCircleFocus(origin, 4.0, camera->Position);
	camera->SetLookFocus(current_level->statics->at("CueBall")->location);

	// Initialise Seconds per Frame counter
	SPF_Counter* spf_report;
    spf_report = new SPF_Counter(true);

	
	// Delta values
	double delta, lastFrame, currentFrame = glfwGetTime();

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

		gamemode->strike(0, (glm::normalize(camera->Front) * cue_force));
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
		// We only wish to circle left and right, Not up and down thus 0.0 for yoffset
		std::cout << "xoffset: " << xoffset << "\tmod: " << xoffset * MOUSE_SPEED << std::endl;
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