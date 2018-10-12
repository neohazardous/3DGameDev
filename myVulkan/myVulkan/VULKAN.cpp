
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h> //this way GLFW will include its own definitions and automatically load the Vulkan header
#include <iostream>
#include <stdexcept> 
#include <vector>
#include <cstring>
//#include <functional> //will be used for a lambda functions in the resource mangement sections 
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
	
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, callback, pAllocator);
	}
}

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
	VkDebugUtilsMessengerEXT callback;

	//we'll have to select a graphics card and store it in a vkphysicaldevice handle thats added as a new class member
	//this object will be destroyed when vkinstance is destroyed, so we dont need to add anything new to the cleanup function
	VkPhysicalDevice physDevice = VK_NULL_HANDLE;

	void initWindow() {

		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	void initVulkan() {

		createInstance();
		setupDebugCallback();
		pickPhysicalDevice();
	}

	void pickPhysicalDevice() {
		
		//listing the gpu is similar to listing extensions & starts with querying just the #
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		//if theres 0 devices w/ vulkan support, then there isn't a point in continuing
		if (deviceCount == 0) {
			throw std::runtime_error(" failed to find a GPU with Vulkan support");
		}

		//if we do find something, allocate an array to hold all the vkphysicaldevice handles
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		//we check if the physical devices found meet the requirements
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physDevice = device;
				break;
			}
		}

		if (physDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU.");
		}
	}

	//we need to evaluate the devices to see if they're suitable for what we need to do. 
	//after all, not all GPUs are equal in ability.
	bool isDeviceSuitable(VkPhysicalDevice device) {

		return true;
	}



	//mainLoop will interate rendering frames until the window is closed
	void mainLoop() {

		while (!glfwWindowShouldClose) {
			glfwPollEvents();
		}
	}

	//cleanup is used to deallocate resources
	void cleanup() {
		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, callback, nullptr);
		}

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
		/*
		uint32_t glfwExtCount = 0;
		const char ** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtCount);
		
		createInfo.enabledExtensionCount = glfwExtCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;
		createInfo.enabledLayerCount = 0;
		*/
		//
		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

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
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;

		/*
		messageSeverity allows you to specify all the types of severities you want your callback to ba called for.
		messageType lets you filter types of messages your callback is notified about
		pfnUserCallback specifies the pointer to the callback function. 
		
		vk_debug_utils_message_severity_verbose_bit_ext: diagnostic message
		vk_debug_utils_message_severity_info_bit_ext: informational message like the creation of a resource
		vk_debug_utils_message_severity_warning_bit_ext: message about behaviour thats not necessarily an error but very likely a bug in your application
		vk_debug_utils_message_severity_error_bit_ext: message about behaviour thats invalid and may cause crashes
		*/


		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up a debug callback.");
		}
	}

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

	//this function checks if all of the request layers are available
	bool checkValidationLayerSupport() {

		//first list all of the available layers using vkenumerateinstancelayerproperties
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) { //strcmp compares the 2 variables to see if they match
				if (strcmp(layerName, layerProperties.layerName) == 0) {
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

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
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
