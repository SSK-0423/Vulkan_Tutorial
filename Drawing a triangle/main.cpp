#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NODEBUG
const bool enableValicationLayers = false;
#else
const bool enableValicationLayers = true;
#endif // NODEBUG

/// <summary>
/// DebugUtilsMessengerEXT�𐶐�
/// </summary>
/// <param name="instance"></param>
/// <param name="pCreateInfo"></param>
/// <param name="pAllocator">�J�X�^���A���P�[�^</param>
/// <param name="pDebugMessenger"></param>
/// <returns></returns>
VkResult CreateDebugUtilsMessengerEXT(
	VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) 
{
	// �g���֐��Ȃ̂�vkGetInstanceProcAddr�ł��̃A�h���X���擾
	// ���s������nullptr
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	// �֐����ǂݍ��߂�
	if (func != nullptr) {
		// VkDebugUtilsMessengerEXT�𐶐�
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

/// <summary>
/// DebugUtilsMessengerEXT�폜
/// </summary>
/// <param name="instance"></param>
/// <param name="debugMesenger"></param>
/// <param name="pAllocator"></param>
void DestroyDebugUtilsMessengerEXT(
	VkInstance instance, VkDebugUtilsMessengerEXT debugMesenger,const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMesenger, pAllocator);
	}
}

class HelloTriangleApplication {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}
private:
	GLFWwindow* window;

	VkInstance instance;

	VkDebugUtilsMessengerEXT debugMessenger;

	void initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	void initVulkan() {
		createInstance();
		setupDebugMessenger();
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void cleanup() {
		if (enableValicationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	}

	// VkInstance����
	void createInstance() {
		if (enableValicationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// �g���@�\�̐ݒ�
		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		// �o���f�[�V�������C���[�̐ݒ�
		if (enableValicationLayers) {	// �o���f�[�V�������C���[���L���Ȃ�
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			// VkInstance�̐����E�j���p�̃f�o�b�O���b�Z���W���[�Z�b�g
			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {	// �o���f�[�V�������C���[���g�p���Ȃ�
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
	}

	// �o���f�[�V�������C���[�̃T�|�[�g�`�F�b�N
	bool checkValidationLayerSupport() {
		uint32_t layerCount;	// �o���f�[�V�������C���[��
		// ���p�\�ȃo���f�[�V�������C���[��񋓂��Ă��̐����擾
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		// �o���f�[�V�������C���[�̃��X�g
		std::vector<VkLayerProperties> availableLayers(layerCount);
		// �����Ńo���f�[�V�������C���[���擾����
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			// ���p�\�ȃo���f�[�V�������C���[�Ɋ܂܂�Ă��邩�`�F�b�N
			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}
			// ���p�\�łȂ����̂����������ꍇ��false
			if (!layerFound) {
				return false;
			}
		}
		// �S�ė��p�\�Ȃ�true
		return true;
	}

	/// <summary>
	/// �g���@�\�̃��X�g��Ԃ�
	/// </summary>
	/// <returns></returns>
	std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;	// �g���@�\��
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValicationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	/// <summary>
	/// �f�o�b�O�R�[���o�b�N�֐�
	/// </summary>
	/// <param name="messageSeverity">���b�Z�[�W�̏d�v�x</param>
	/// <param name="messageType">���b�Z�[�W�̎��</param>
	/// <param name="pCallbackData">�R�[���o�b�N����郁�b�Z�[�W�����i�[���邽�߂̍\����</param>
	/// <param name="pUserData">�Ǝ��f�[�^</param>
	/// <returns>�o���f�[�V�������C���[�������N������Vulkan�R�[���𒆎~���ׂ����ǂ���</returns>
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	/// <summary>
	/// �f�o�b�O���b�Z���W���[�Z�b�g�A�b�v
	/// </summary>
	void setupDebugMessenger() {
		if (!enableValicationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		// VkInstance�̐����Ɣj���p�̃f�o�b�O���b�Z���W���[
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}
};

int main() {
	HelloTriangleApplication app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}