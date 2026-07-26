#include <algorithm>
#include <cassert>
#include <format>
#include <print>
#include <string>
#include <unordered_set>
#include <utility>
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
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
#include <stdexcept>

#include "generic.hpp"
#include <cstdint>
#include <cstdlib>
#include <limits>

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
    vk::raii::SurfaceKHR surface                    = nullptr;

    vk::raii::PhysicalDevice physicalDevice = nullptr;

    vk::raii::Device device       = nullptr;
    vk::raii::Queue graphicsQueue = nullptr;

    vk::raii::SwapchainKHR swapChain = nullptr;
    std::vector<vk::Image> swapChainImages;
    vk::SurfaceFormatKHR swapChainSurfaceFormat;
    vk::Extent2D swapChainExtent;

    std::vector<vk::raii::ImageView> swapChainImageViews;

    std::vector<const char*> requiredDeviceExtension = { vk::KHRSwapchainExtensionName };

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
        createSurface ();
        pickPhysicalDevice ();
        createLogicalDevice ();
        createSwapChain ();
        createImageViews ();
    }

    void mainLoop ()
    {
        winMan.startWindowLoop ();
    }

    void cleanup ()
    {
        winMan.cleanup ();
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


        // Get the required extensions
        auto requiredExtensions = getRequiredInstanceExtensions ();

        std::println ("available extensions:");
        std::unordered_set<std::string> availableNames;
        for (const auto& ext : extensionProperties) {
            std::println ("{}", std::string (ext.extensionName));
            availableNames.insert (ext.extensionName);
        }

        // Check if the required extensions are supported by the vulkan implementation
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

    bool isDeviceSuitable (vk::raii::PhysicalDevice const& physicalDevice)
    {

        // Check if the physicalDevice supports the Vulkan 1.3 API Version
        bool supportsVulkan1_3 = physicalDevice.getProperties ().apiVersion >= vk::ApiVersion13;

        // Check if any of the queue families support graphics operations
        auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties ();
        bool supportsAllRequiredExtensions =
        std::ranges::all_of (requiredDeviceExtension,
        [&availableDeviceExtensions] (auto const& requiredDeviceExtension) {
            return std::ranges::any_of (availableDeviceExtensions,
            [requiredDeviceExtension] (auto const& availableDeviceExtension) { return strcmp (availableDeviceExtension.extensionName, requiredDeviceExtension) == 0; });
        });

        // Check if the physicalDevice supports the required features
        auto features = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> ();

        bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features> ().shaderDrawParameters &&
        features.template get<vk::PhysicalDeviceVulkan13Features> ().dynamicRendering &&
        features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> ().extendedDynamicState;

        // Return true if physical device meets all the criteria
        return supportsVulkan1_3 && supportsAllRequiredExtensions && supportsRequiredFeatures;
    }

    void createSurface ()
    {
        // Creating a Win32 surface

        vk::Win32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType     = vk::StructureType::eWin32SurfaceCreateInfoKHR;
        createInfo.hwnd      = winMan.getWindowHandle ();
        createInfo.hinstance = winMan.getInstance ();


        surface = instance.createWin32SurfaceKHR (createInfo);
    }

    void pickPhysicalDevice ()
    {
        auto physicalDevices = instance.enumeratePhysicalDevices ();
        auto const devIter   = std::ranges::find_if (physicalDevices, [&] (auto const& physicalDevice) { return isDeviceSuitable (physicalDevice); });
        if (devIter == physicalDevices.end ()) {
            std::runtime_error ("No GPUs with?? !(UwU)... with Vulkan support...");
        }

        physicalDevice  = *devIter;
        bool isDiscrete = physicalDevice.getProperties ().deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
        std::println ("Device name: {}\nDevice type: {}", std::string (physicalDevice.getProperties ().deviceName), [isDiscrete] () {
            return isDiscrete ? std::string ("Discrete GPU") : std::string ("Integrated GPU");
        }());
    }

    void createLogicalDevice ()
    {
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties ();

        uint32_t queueIndex = ~0;

        // get first index into queue families which support graphics and is present
        for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size (); ++qfpIndex) {
            if ((queueFamilyProperties.at (qfpIndex).queueFlags & vk::QueueFlagBits::eGraphics) && physicalDevice.getSurfaceSupportKHR (qfpIndex, surface)) {
                queueIndex = qfpIndex;
                break;
            }
        }

        if (queueIndex == ~0) {
            throw std::runtime_error ("Could not find a queue for graphics and present -> terminating");
        }

        float queuePriority = 0.5f;

        vk::DeviceQueueCreateInfo deviceQueueCreateInfo;
        deviceQueueCreateInfo.queueFamilyIndex = queueIndex;
        deviceQueueCreateInfo.queueCount       = 1;
        deviceQueueCreateInfo.pQueuePriorities = &queuePriority;


        vk::PhysicalDeviceFeatures deviceFeatures;

        // Create a chain of feature structs
        vk::StructureChain<vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        featureChain = {
            {},
            { .shaderDrawParameters = true },
            { .dynamicRendering = true },
            { .extendedDynamicState = true }
        };

        vk::DeviceCreateInfo deviceCreateInfo;
        deviceCreateInfo.pNext                   = &featureChain.get<vk::PhysicalDeviceFeatures2> ();
        deviceCreateInfo.queueCreateInfoCount    = 1;
        deviceCreateInfo.pQueueCreateInfos       = &deviceQueueCreateInfo;
        deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t> (requiredDeviceExtension.size ());
        deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtension.data ();

        device        = vk::raii::Device (physicalDevice, deviceCreateInfo);
        graphicsQueue = vk::raii::Queue (device, queueIndex, 0);
    }

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat (std::vector<vk::SurfaceFormatKHR> const& availableFormats)
    {
        assert (!availableFormats.empty ());
        auto formatIt = std ::ranges::find_if (availableFormats,
        [] (const auto& format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear; });
        return formatIt != availableFormats.end () ? *formatIt : availableFormats.front ();
    }

    // looks for the best presentation mode available
    vk::PresentModeKHR chooseSwapPresentMode (std::vector<vk::PresentModeKHR> const& availablePresentModes)
    {

        assert (std::ranges::any_of (availablePresentModes, [] (auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));


        return std::ranges::any_of (availablePresentModes, [] (const vk::PresentModeKHR value) { return value == vk::PresentModeKHR::eMailbox; }) ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
    }


    vk::Extent2D chooseSwapExtent (vk::SurfaceCapabilitiesKHR const& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max ()) {
            return capabilities.currentExtent;
        }

        int width, height;
        winMan.getWindowSize (&width, &height);
        return {
            std::clamp<uint32_t> (width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp<uint32_t> (height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
        };
    }


    uint32_t chooseSwapMinImageCount (vk::SurfaceCapabilitiesKHR const& surfaceCapabilites)
    {
        auto minImageCount = std::max (3u, surfaceCapabilites.minImageCount);
        if ((0 < surfaceCapabilites.maxImageCount) && (surfaceCapabilites.maxImageCount < minImageCount)) {
            minImageCount = surfaceCapabilites.maxImageCount;
        }
        return minImageCount;
    }

    void createSwapChain ()
    {
        vk::SurfaceCapabilitiesKHR surfaceCapabilites = physicalDevice.getSurfaceCapabilitiesKHR (surface);
        swapChainExtent                               = chooseSwapExtent (surfaceCapabilites);

        uint32_t minImageCount = chooseSwapMinImageCount (surfaceCapabilites) + 1;

        std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR (*surface);
        swapChainSurfaceFormat                             = chooseSwapSurfaceFormat (availableFormats);

        std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR (*surface);

        vk::SwapchainCreateInfoKHR swapChainCreateInfo{
            .surface          = *surface,
            .minImageCount    = minImageCount,
            .imageFormat      = swapChainSurfaceFormat.format,
            .imageColorSpace  = swapChainSurfaceFormat.colorSpace,
            .imageExtent      = swapChainExtent,
            .imageArrayLayers = 1, // always 1 unless stereoscopic application or overlay stuff
            .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
            .imageSharingMode = vk::SharingMode::eExclusive,
            .preTransform     = surfaceCapabilites.currentTransform,
            .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode      = chooseSwapPresentMode (availablePresentModes),
            .clipped          = true,
        };

        swapChainCreateInfo.oldSwapchain = nullptr;

        swapChain       = vk::raii::SwapchainKHR (device, swapChainCreateInfo);
        swapChainImages = swapChain.getImages ();
    }

    void createImageViews ()
    {
        assert (swapChainImageViews.empty ());

        vk::ImageViewCreateInfo imageViewCreateInfo{
            .viewType         = vk::ImageViewType::e2D,
            .format           = swapChainSurfaceFormat.format,
            .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
        };
        imageViewCreateInfo.components = {
            vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity
        };

        for (auto& image : swapChainImages) {
            imageViewCreateInfo.image = image;
            swapChainImageViews.emplace_back (device, imageViewCreateInfo);
        }
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

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback (vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
            std::cerr << "Validation layer: type " << vk::to_string (type) << " msg: " << pCallbackData->pMessage << '\n';
        }
        return vk::False;
    }
};


int main ()
{

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


    return EXIT_SUCCESS;
}
