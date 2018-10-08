//#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h> //this way GLFW will include its own definitions and automatically load the Vulkan header
#include <iostream>
#include <stdexcept> 
#include <functional> //will be used for a lambda functions in the resource mangement sections 
#include <cstdlib> //This header provides exit_success/exit_failure macros

const int WIDTH = 800;
const int HEIGHT = 600;

class TriangleRend {
	
	//make constants for window creation vs just hard coding the window size

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
	GLFWwindow * window;
	VkInstance instance;

	void initWindow() {
		
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	void initVulkan() {

		createInstance();

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

	void createInstance() {

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello world";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		uint32_t glfwExtCount = 0;
		const char ** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtCount);

		createInfo.enabledExtensionCount = glfwExtCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;

		createInfo.enabledLayerCount = 0;

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {

			throw std::runtime_error("failed to create instance");

		}

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