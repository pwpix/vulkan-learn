#include <algorithm>
#include <cassert>
#include <format>
#include <ios>
#include <memory>
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

#include <fstream>
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


// helper to load binary data from files
static std::vector<char> readFile (const std::string& filename)
{
    std::ifstream file (filename, std::ios::ate | std::ios::binary);

    if (!file.is_open ()) {
        throw std::runtime_error ("failed to open file!");
    }

    std::vector<char> buffer (file.tellg ());
    file.seekg (0, std::ios::beg);
    file.read (buffer.data (), static_cast<std::streamsize> (buffer.size ()));

    file.close ();
    return buffer;
}

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
    vk::raii::Context                context;
    vk::raii::Instance               instance       = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
    vk::raii::SurfaceKHR             surface        = nullptr;
    vk::raii::PhysicalDevice         physicalDevice = nullptr;
    vk::raii::Device                 device         = nullptr;
    vk::raii::Queue                  graphicsQueue  = nullptr;
    vk::raii::SwapchainKHR           swapChain      = nullptr;
    std::vector<vk::Image>           swapChainImages;
    vk::SurfaceFormatKHR             swapChainSurfaceFormat;
    vk::Extent2D                     swapChainExtent;
    vk::raii::PipelineLayout         pipelineLayout   = nullptr;
    vk::raii::Pipeline               graphicsPipeline = nullptr;

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
        createGraphicsPipeline ();
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
        vk::DebugUtilsMessageTypeFlagsEXT     messageTypeFlags (vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

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
        auto       physicalDevices = instance.enumeratePhysicalDevices ();
        auto const devIter         = std::ranges::find_if (physicalDevices, [&] (auto const& physicalDevice) { return isDeviceSuitable (physicalDevice); });
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


    // helper for shader module
    [[nodiscard]] vk::raii::ShaderModule createShaderModule (const std::vector<char>& code) const
    {
        vk::ShaderModuleCreateInfo createInfo{ .codeSize = code.size () * sizeof (char), .pCode = reinterpret_cast<const uint32_t*> (code.data ()) };
        vk::raii::ShaderModule     shaderModule (device, createInfo);
        return shaderModule;
    }

    void createGraphicsPipeline ()
    {
        auto                   shaderCode   = readFile ("shaders/slang.spv");
        vk::raii::ShaderModule shaderModule = createShaderModule (shaderCode);

        // vertex shader
        vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
            .stage  = vk::ShaderStageFlagBits::eVertex, // shader stage
            .module = shaderModule,                     // shader module with the code
            .pName  = "vertMain"                        // function to invoke
        };

        // fragment shader
        vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
            .stage  = vk::ShaderStageFlagBits::eFragment,
            .module = shaderModule,
            .pName  = "fragMain"
        };

        vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        // Specify no vertex data to load, because vertices are hardcoded in VS
        // NOTE: if you have loaded vertex data you need to configure this
        vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
        // Specify draw primitives, we intend to use only triangles for this application
        // For 'Strip' topologies: If you use primitiveRestartEnable = vk::True, it's possibe to break up lines and
        // triangles by using a special index of '0xFFFF' or '0xFFFFFFFF'
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{ .topology = vk::PrimitiveTopology::eTriangleList };

        vk::Viewport viewport{ 0.0f, 0.0f, static_cast<float> (swapChainExtent.width), static_cast<float> (swapChainExtent.height), 0.0f, 1.0f };

        // Scissor rectangle
        // Viewports and scissor rectangles can be specified as static state or can be dynamic state set in command buffer.
        // configuring this as dynamic state is often more convinient because we get lot more flexibility
        // All implementations can handle this dynamic state without a performance penalty.
        vk::Rect2D scissor{ vk::Offset2D{ 0, 0 }, swapChainExtent };

        // We opt for dynamic state, and enable them for the pipeline respectively
        std::vector<vk::DynamicState>      dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
        vk::PipelineDynamicStateCreateInfo dynamicState{ .dynamicStateCount = static_cast<uint32_t> (dynamicStates.size ()), .pDynamicStates = dynamicStates.data () };

        // we only need to specify the count
        // for a static state we specify them in the struct and this makes them immutable
        // We can specify multiple viewports and scissors, this requires enabling a GPU feature
        // For this purpose the struct members reference an array of them
        vk::PipelineViewportStateCreateInfo viewportState{ .viewportCount = 1, .pViewports = &viewport, .scissorCount = 1, .pScissors = &scissor };

        // Rasterizer
        // rasterizer performs depth testing, face culling and the scissor test
        // https://en.wikipedia.org/wiki/Z-buffering[depth testing, https://en.wikipedia.org/wiki/Back-face_culling[face culling
        // We can configure this for filling entire polygons or wireframe rendering
        // TODO: experiment using other configurations
        vk::PipelineRasterizationStateCreateInfo rasterizer{
            .depthClampEnable        = vk::False,
            .rasterizerDiscardEnable = vk::False,
            .polygonMode             = vk::PolygonMode::eFill,
            .cullMode                = vk::CullModeFlagBits::eBack,
            .frontFace               = vk::FrontFace::eClockwise,
            .depthBiasEnable         = vk::False, // used for shadowmapping
            .lineWidth               = 1.0f       // thickness of lines in terms of fragments, maximum line width depends on hardware and >1.0f requires enabling 'wideLines' GPU feature
        };

        // Multisampling
        vk::PipelineMultisampleStateCreateInfo multisampling{ .rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False };

        // Depth and stencil buffer
        // None

        // Color blending
        // Theres two ways of doing this. 1. mix old and new to produce a final color 2. combine old and new using bitwise operations
        // There are two structs to configure color blending, vk::PipelineColorBlendAttachmentState contains per attached framebuffer settings,
        // and vk::PipelineColorBlendStateCreateInfo contains _global_ color blending state

        // This is commonly used when implementing alpha blending, where we blend new color with old color based on opacity

        // This configures the first way of color blending
        vk::PipelineColorBlendAttachmentState colorBlendAttachment{
            .blendEnable         = vk::False,
            .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
            //.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrc1Alpha,
            .colorBlendOp        = vk::BlendOp::eAdd,
            .srcAlphaBlendFactor = vk::BlendFactor::eOne,
            .dstAlphaBlendFactor = vk::BlendFactor::eZero,
            .alphaBlendOp        = vk::BlendOp::eAdd,
            .colorWriteMask      = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        };

        vk::PipelineColorBlendStateCreateInfo colorBlending{
            .logicOpEnable = vk::False, // if you want to use the second method of blending (using bitwise combination), set this to vk::True.
                                        // The bitwise operation can be specified in the logicOp field
            .logicOp         = vk::LogicOp::eCopy,
            .attachmentCount = 1,
            .pAttachments    = &colorBlendAttachment
        };
        // we have disabled both modes here, in this case the fragment colors are written to framebuffer unmodified

        // Pipeline layout
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
            .setLayoutCount         = 0,
            .pushConstantRangeCount = 0

        };
        pipelineLayout = vk::raii::PipelineLayout (device, pipelineLayoutInfo);

        // Dynamic rendering Pipeline rendering create info to specify format attachments that will be used during rendering.
        // using struct chaining but each struct can be defined independently
        vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
            { .stageCount        = 2,
            .pStages             = shaderStages,
            .pVertexInputState   = &vertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState      = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState   = &multisampling,
            .pColorBlendState    = &colorBlending,
            .pDynamicState       = &dynamicState,
            .layout              = pipelineLayout,
            .renderPass          = nullptr },

            { .colorAttachmentCount = 1, .pColorAttachmentFormats = &swapChainSurfaceFormat.format }
        };

        // the second parameter which is nullptr references an optional vk::raii::PipelineCache object
        graphicsPipeline = vk::raii::Pipeline (device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo> ());
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
