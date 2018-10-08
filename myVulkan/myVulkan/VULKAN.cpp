//#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h> //this way GLFW will include its own definitions and automatically load the Vulkan header
#include <iostream>
#include <stdexcept> 
#include <functional> //will be used for a lambda functions in the resource mangement sections 
#include <cstdlib> //This header provides exit_success/exit_failure macros

class TriangleRend {
	
	//make constants for window creation vs just hard coding the window size
	const int WIDTH = 800; 
	const int HEIGHT = 600;
	GLFWwindow * window;

public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}
/*
private:
	void initWindow() {

	}
	*/


	//vulkan objects will be stored as private class members. functions will be used to initiate them
	//which will be called from the initVulkan function.
private:

	void initWindow() {
		
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	void initVulkan() {

	}

	//mainLoop will interate rendering frames until the window is closed
	void mainLoop() {
		
		while (!glfwWindowShouldClose) {

			glfwPollEvents();

		}

	}

	//cleanup is used to deallocate resources
	void cleanup() {

		glfwDestroyWindow(window);
		glfwTerminate();

	}
};

int main() {

	TriangleRend app;

	try {

		app.run();

	}

	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}