#include <algorithm>
#include <print>
#include <string>
#include <unordered_set>
#define VK_USE_PLATFORM_WIN32_KHR
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif


#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_core.h>

#define _DEBUG
#include "window/win32.hpp"
#undef _DEBUG

#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>
#include <stdexcept>

#include "generic.hpp"
#include <cstdint>
#include <cstdlib>

const uint32_t WIDTH  = 800;
const uint32_t HEIGHT = 600;

const std::vector<char const*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

class HelloTriangleApplication {

    public:
    void run ()
    {
        initWindow ();
        initVulkan ();
        mainLoop ();
        cleanup ();
    }

    private:
    WIN32_Window_Manager winMan;

    // Vulkan data members
    vk::raii::Context context;
    vk::raii::Instance instance                     = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

    // internal methods

    void initWindow ()
    {
        WindowConfig conf ("Vulkan", WIDTH, HEIGHT);
        winMan.initializeWindowManager (conf);
    }

    void initVulkan ()
    {
        createInstance ();
        setupDebugMessenger ();
    }


    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback (vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT type,
    const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
    {
        std::cerr << "Validation layer: type " << vk::to_string (type) << " msg: " << pCallbackData->pMessage << '\n';
        return vk::False;
    }

    std::vector<const char*> getRequiredInstanceExtensions ()
    {
        std::vector<const char*> requiredExtensions = {
            vk::KHRSurfaceExtensionName,
            vk::KHRWin32SurfaceExtensionName,
        };

        if (enableValidationLayers) {
            requiredExtensions.push_back (vk::EXTDebugUtilsExtensionName);
        }
        return requiredExtensions;
    }

    void createInstance ()
    {
        vk::ApplicationInfo appInfo;
        appInfo.pApplicationName   = "HelloTriangle";
        appInfo.applicationVersion = VK_MAKE_VERSION (1, 0, 0);
        appInfo.pEngineName        = "No Engine";
        appInfo.engineVersion      = VK_MAKE_VERSION (1, 0, 0);
        appInfo.apiVersion         = vk::ApiVersion14;


        // Get the required instance extensions manual procedure?

        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties (nullptr, &extensionCount, nullptr);
        auto extensionProperties = context.enumerateInstanceExtensionProperties ();

        // std::vector<VkExtensionProperties> extensions (extensionCount);
        // vkEnumerateInstanceExtensionProperties (nullptr, &extensionCount, extensions.data ());

        /*
        std::vector<const char*> requiredExtensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME
        };*/

        auto requiredExtensions = getRequiredInstanceExtensions ();

        std::println ("available extensions:");
        std::unordered_set<std::string> availableNames;
        for (const auto& ext : extensionProperties) {
            std::println ("{}", std::string (ext.extensionName));
            availableNames.insert (ext.extensionName);
        }

        std::println ("checking required extensions:");
        for (auto req : requiredExtensions) {
            if (!availableNames.contains (req))
                throw std::runtime_error ("Required extension(s) are missing " + std::string (req));
            else
                std::println ("Found extension: {}", req);
        }


        // Get the required layers
        std::vector<char const*> requiredLayers;
        if (enableValidationLayers) {
            requiredLayers.assign (validationLayers.begin (), validationLayers.end ());
        }

        // Check if the required layers are supported by the Vulkan implementation
        auto layerProperties    = context.enumerateInstanceLayerProperties ();
        auto unsupportedlayerIt = std::ranges::find_if (requiredLayers,
        [&layerProperties] (auto const& requiredLayer) { return std::ranges::none_of (layerProperties,
                                                         [requiredLayer] (auto const& layerProperty) { return strcmp (layerProperty.layerName, requiredLayer) == 0; }); });

        if (unsupportedlayerIt != requiredLayers.end ()) {
            throw std::runtime_error ("Required layer not supported: " + std::string (*unsupportedlayerIt));
        }

        vk::InstanceCreateInfo createInfo;
        createInfo.pApplicationInfo        = &appInfo;
        createInfo.enabledLayerCount       = static_cast<uint32_t> (requiredLayers.size ());
        createInfo.ppEnabledLayerNames     = requiredLayers.data ();
        createInfo.enabledExtensionCount   = static_cast<uint32_t> (requiredExtensions.size ());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data ();

        instance = vk::raii::Instance (context, createInfo);
    }


    void setupDebugMessenger ()
    {
        if (!enableValidationLayers)
            return;

        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags (vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
        vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags (vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

        vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT;

        debugUtilsMessengerCreateInfoEXT.messageSeverity = severityFlags;
        debugUtilsMessengerCreateInfoEXT.messageType     = messageTypeFlags;
        debugUtilsMessengerCreateInfoEXT.pfnUserCallback = &debugCallback;

        debugMessenger = instance.createDebugUtilsMessengerEXT (debugUtilsMessengerCreateInfoEXT);
    }


    void
    mainLoop ()
    {
        winMan.startWindowLoop ();
    }
    void cleanup ()
    {
        winMan.cleanup ();
    }
};

struct Massive {
    int* data = new int[2500000000];
    void showVal ()
    {
        std::cout << data[234587] << '\n';
    }
};

int main ()
{

    // WindowConfig conf ("test window", 800, 400);

    {
        try {
            HelloTriangleApplication app;
            app.run ();
        }
        catch (const std::exception& e) {
            std::cerr << e.what () << '\n';
            return EXIT_FAILURE;
        }
    }

    // WIN32_Window_Manager winMan;
    // winMan.initializeWindowManager (conf);
    // winMan.startWindowLoop ();

    /*
    {
        UniqueHandle<int*, IntPointerDeleter> smartInt (new int (42));
        std::cout << "Value of smartInt: " << *(smartInt.resource);

        UniqueHandle<const char*, FileDeleter> smartFile ("save_game.dat");
        std::cout << "Working with file: " << smartFile.resource << "\n";

        // Your code should refuse to compile this line to protect against copying
        // UniqueHandle<const char*, FileDeleter> copyAttempt = smartFile;
    }

    auto s = std::to_address (std::make_unique<Massive> ());

    s->showVal ();
    */

    return EXIT_SUCCESS;
}
