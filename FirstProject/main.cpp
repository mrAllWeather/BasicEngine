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

// Window Dimensions
const GLuint WIDTH = 1024, HEIGHT = 768;
bool firstMouse = true;
GLfloat lastX = 400, lastY = 300;
Camera* camera;
bool keys[1024];

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement(float deltaTime);


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
	glfwSetScrollCallback(window, scroll_callback);

	// Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set up viewport
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	glEnable(GL_DEPTH_TEST);

	// Clear Color
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

	// Initial Scene
	Scene* currentLevel;

	if(argc > 1)
	{
		currentLevel = new Scene(argv[1]);
	}
	else
	{
		currentLevel = new Scene("./Scenes/Level_01.scene");
	}

	// Scene* currentLevel = new Scene("./Scenes/Test.scene");
	camera = currentLevel->camera;
	// SceneLoader load_scene("./Scenes/Level_01.scene", currentLevel);

	// Initialise Seconds per Frame counter
	SPF_Counter* spf_report;

        if(argc > 2)
		spf_report = new SPF_Counter(true);
	else
		spf_report = new SPF_Counter(false);

	// Line Mode
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Program Loop
	while (!glfwWindowShouldClose(window))
	{
		// Show current time per frame
		spf_report->tick();
		currentLevel->tick(spf_report->delta());

		Do_Movement(spf_report->delta());
		// Check and call events
		glfwPollEvents();

		// Rendering Commands here
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		currentLevel->draw();

		// Swap Buffers
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// Need to shift Key callback into it's own class
	// Wish to extend into a state machine and have the ability to add or remove keybindings on the fly

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

// Moves/alters the camera positions based on user input
void Do_Movement(float deltaTime)
{
	// Camera controls
	if (keys[GLFW_KEY_W])
	{
		camera->ProcessKeyboard(FORWARD, deltaTime);
	}
	if (keys[GLFW_KEY_S])
	{
		camera->ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (keys[GLFW_KEY_A])
	{
		camera->ProcessKeyboard(LEFT, deltaTime);
	}
	if (keys[GLFW_KEY_D])
	{
		camera->ProcessKeyboard(RIGHT, deltaTime);
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
	GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

	lastX = xpos;
	lastY = ypos;

	camera->ProcessMouseMovement(xoffset, yoffset, true);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera->ProcessMouseScroll(yoffset);
}
