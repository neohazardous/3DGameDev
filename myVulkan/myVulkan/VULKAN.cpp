//#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h> //this way GLFW will include its own definitions and automatically load the Vulkan header
#include <iostream>
#include <stdexcept> 
#include <functional> //will be used for a lambda functions in the resource mangement sections 
#include <cstdlib> //This header provides exit_success/exit_failure macros

const int WIDTH = 800;
const int HEIGHT = 600;

const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };

//add 2 config variables to specify the layers to enable/not enable
//the ndebug macro is part of the c++ standard and means "not debug"
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif // NDEBUG



class TriangleRend {
	
	//make constants for window creation vs just hard coding the window size

public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

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
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();

	}

	void createInstance() {

		//use checkValidationSupport() here in createInstance()
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error(" validation layers requested, but they are not available.");
		}


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

		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

		//if this check is successful, then vkcreateinstance should not return "VK_ERROR_LAYER_NOT_PRESENT"
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else 
		{
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance");
		}
	}

	//setting to get message callback.
	void setupDebugCallback() {
		
		//we create a getrequiredextensions function that will return the required list of extensions
		std::vector<const char*> getRequiredExtensions() {
			uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

			if (enableValidationLayers) {
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}
			return extensions;
		}
	}
	
	//this function checks if all of the request layers are available
	bool checkValidationLayerSupport() {

		//first, list all of the available layers using vkenumerateinstancelayerproperties
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
		
		//check if all the layers in validationLayers exist in availableLayers list.
		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) { //strcmp compares the 2 variables to see if they match
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}
		return true;
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