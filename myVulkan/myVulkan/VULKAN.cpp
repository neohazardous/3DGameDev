#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h> //this way GLFW will include its own definitions and automatically load the Vulkan header
#include <iostream>
#include <stdexcept> 
#include <vector>
#include <cstring>
#include <cstdlib> //This header provides exit_success/exit_failure macros
#include <optional>
#include <set>
#include <algorithm>

const int WIDTH = 800;
const int HEIGHT = 600;

const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }; //declare a list of required device extensions. similar to validation layers to enable
const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };

//add 2 config variables to specify the layers to enable/not enable
//the ndebug macro is part of the c++ standard and means "not debug"
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif // NDEBUG
	
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
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

//anything from drawing to uploading textures requires commands to be submitted to a queue. 
//there are different types of queues that originate from different queue families and each family of queues allows only a subset of c ommands
//ex: one queue fam that only allows processing of compute commands. one queue fam that only allows memory transfer related commands

//using find queuefamilies, we look for all queue families we need 
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

//a structto pass details around once they've been queried. 
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class TriangleRend {
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
	VkDevice device;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkSurfaceKHR surface; 
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

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
		CreateSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		CreateSwapChain();
	}


	//mainLoop will interate rendering frames until the window is closed
	void mainLoop() {

		while (!glfwWindowShouldClose) {
			glfwPollEvents();
		}
	}

	//cleanup is used to deallocate resources
	void cleanup() {
		vkDestroySwapchainKHR(device, swapChain, nullptr);
		vkDestroyDevice(device, nullptr);

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, callback, nullptr);
		}
		
		vkDestroySurfaceKHR(instance, surface, nullptr); //destroy surface instance
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
		appInfo.pApplicationName = "3D_GameDev";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
	
		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance); //questionably needed

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

	void CreateSurface() {

		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create a window surface.");
		}
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

	//the creation of a logical device involves specifying a lot of details in structs again, of which the 1st one will be vkdevicequeuecreateinfo
	//this struct describes the number of queues we want for a single queue family 
	void createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physDevice);
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}
		VkPhysicalDeviceFeatures deviceFeatures = {};
		VkDeviceCreateInfo createInfo= {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		//currently available drivers will onyl allow you to create a small numbero f queues for each queue family
		//vulkan lets you assign priorities to queues to influenced the scheduling of command buffer execution
		//you use floating point numbers between 0.0-1.0. this is required even if theres only 1 queue
		
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		//explicitly enable the extensions
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		//requires you to specify extensions and validation layers. these are device sepcific this time
		createInfo.enabledExtensionCount = 0;
		

		///////////creating the logical device////////////

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		//now to instantiate the logical device named vkcreatedevice
		if (vkCreateDevice(physDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error(" failed to create a logical device.");
		}

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	void CreateSwapChain() {
		
		SwapChainSupportDetails swapchainSupport = querySwapChainSupport(physDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities);
		//the number of images in a swap chain (queue length). specifies the min amt of images to function properly
		uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
		if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) {
			imageCount = swapchainSupport.capabilities.maxImageCount;
			//val 0 for maximagecount means theres no limit besides memory requirements
		}
		//creating swapchain obj requires filling in a large structure
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		//after specifying which surface the swapchain should be tied to, the details of the swapchain imgs are specified
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent; 
		createInfo.imageArrayLayers = 1; //specifies the amount of layers each image consists of. this is always 1 unless youre developing a stereoscopic 3d app
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //specifies what kind of operations well use the images in the swapchain for

		QueueFamilyIndices indices = findQueueFamilies(physDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; //images can be used across multiple queue families w/o explicit ownership transfers
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //an image is owned by one queue family at a time & ownership must be explicitly transfered before using it in another queue family
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = swapchainSupport.capabilities.currentTransform; //specify that a certain transform should be applied 
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //compisitealpha specifies if the alpha channel should be used for blendings
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE; //clipped member = vk_true means that we dont care about the colour of pixels that are obscured
		createInfo.oldSwapchain = VK_NULL_HANDLE; //this is for when the swapchain needs to be recreated from scratch because it became unoptimized or invalid and it needs a reference to an old swap chain
		
		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("FAILED TO CREATE A SWAPCHAIN");
		}

		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
			return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}
		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) {
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
			else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
				bestMode = availablePresentMode;
			}
		}
		return bestMode;
	}

	//this is the resolution of the swap chain images and its almost always exactly equal to the resolution of the window that were drawing to
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {

		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			VkExtent2D actualExtent = { WIDTH, HEIGHT };

			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;
		//query basic surface capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		//querying supported surface formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		//make sure that the vector is resized to hold all available formats
		//querying supported presentaion modes 
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	//we need to evaluate the devices to see if they're suitable for what we need to do. 
	//after all, not all GPUs are equal in ability.
	bool isDeviceSuitable(VkPhysicalDevice device) {

		//we use findQueueFamilies here to ensure tehat the device can process whatever commands we want to use
		QueueFamilyIndices indices = findQueueFamilies(device);
		bool extensionsSupported = checkDeviceExtSupport(device); 

		//swapchain support is sufficient if there is at least one supported image format and one supported presentation mode given the window surface we have
		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}
		//verifying that the extension is available
		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	bool checkDeviceExtSupport(VkPhysicalDevice device) {

		//enumerate the exensions & check if all of the required extensions are amongst them
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
		
		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}
		return requiredExtensions.empty();
	}
	
	//the process of retrieving the list of queue families
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {

		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;

		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		//we need at least one queuefamily that supports vk_queue_graphics_bit
		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			
			if (queueFamily.queueCount > 0 && presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}
			i++;
		}
		return indices;
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