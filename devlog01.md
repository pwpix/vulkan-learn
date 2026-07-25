

== Setup ==

Initial set up of development environment, includes configuring CMake, installing required 
libraries and verifying a working pipeline.

== First run ==

We do a test run to check functionality. Involves modifying CMake files and modules external dependencies. (had to build libraries from source, use list APPEND, cmake modules
setting dir variables, adding compiler specific flags, cmake module prefix etc.)

== Base code ==

After that, we run a simple application. We define a simple HelloTriangleApplication
class.

Since I'm too swag... I wrote my own win32 window implementation, and refuse to use 
GLFW. This requires improvements but suffices for the current requirements for displaying 
a basic window.

== Instance creation ==

This is the beginning of the Vulkan API code.
We declare two additional data members 'context' and 'instance'.
We create a Vulkan instance. For creating a context we are required to create 
a appInfo struct which contains the application information and engine information which 
will be required later. 
Then we check for available extensions and since I'm using my own windowing using win32
I retrieve that information by querying using vulkan API. We can get all the available
extensions using context.enumerateInstanceExtensionProperties(); which returns a vector
of VkExtensionProperties. And we pass a uint32_t to the API for the extension count. 
And that was not really necessary, just fun.

Then we specify a array of required extensions we would like to have. And because I need 
win32 surface, I have to specify the win32 surface extension and also define the macro 
VK_USE_PLATFORM_WIN32_KHR for vulkan to load platform specific definitions for win32.

And finally we create vk::raii::Instance.

That's all. And before we move forward with instance creation, yeah, we need to set up debugging.
So here comes Validation layers.

== Validation Layers ==

Since Vulkan is focused on low driver overhead, there is minimal error checking.
Provides functionality called validation layers for debugging
We can configure them to be disabled in release builds 
We can debug in debug builds

We add debug utils extension to required extensions.
Query layers. There are two layers, one for instance and another for device
Device layers have been merged with instance layers

We implement static method debug callback used by debug messenger utility.
We configure debug extension flags.
Set up debugging after instance is created.

For further information, check official vulkan documentation for configuring validation layers.


"OpenGL is like a Volkswagen, Vulkan is like driving a Buggati or Ferrari" 

== Physical devices and queue families ==

After initializing vulkan instance, we need a graphics card that supports 
features we need.
Can use any number of graphics cards and use simultaneously.

We query for available device. The process is similar to extensions and layers.
Here we try to find a suitable device, which supports the Vulkan API version, has all the required features and the required extensions.

== Logical device and queues == 

We need a logical device to interface with the physical device. 
Process is similar to the process of creating an instance
We describe the features we want to use
We need to specify which queues to create now form the queried list of available queue families
We can create multiple logical device from the same physical device for varying requirements.


We start by adding a new class member vk::raii::Device device;
We call createLogicalDevice() during initialisation process of the application.
Process involves specifying details in structs again. 
We specify the queues to be created
We will focus on queues with graphics capabilities

We specify device features, use struct chaining.
(For struct chaining to work DISABLE constructors, designated initialization need aggregate type. Cant have user defined constructors)

Enable device extensions.
We enable 'VK_KHR_swapchain' extension, required for presenting image to window.

Finally we create the logical device
We pass the physical device to interface with and create the info we specified.
Additionally we can pass allocation callback pointers
and pointer to store the created logical device handle

Enabling non extensions can throw errors

Logical device is not included as a parameter because it doesnt interact directly with instances.



We need handle to interface with queues 
We add a new class member to store the graphics queue

With the logical device and queue handles we can now start using the GPU to do things.


