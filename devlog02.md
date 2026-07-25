
== Window Surface ==

We create a window surface to interface with the window system
Vulkan is platform agnostic

We enable macros for WIN32 
Create win32 Surface KHR

Raii API provides us a method through the vk instance 
to create a surface.
If we are interacting with the C API we require a C struct
it can be easily converted to raii vk::raii::Surface(inttance, surface)
GLFW uses C API

We check for presentation support 
We query the queue family
We look for capability of both supporting graphics operations and presentation support
Window surface needs to be created right after instance creation
Reason is it can influence physical device creation.
Window surface are optional component, 
If we need off screen rendering, Vulkan allows that
no hacks like invisible window (openGL)
Supports remote render from non-presenting GPU or remotely over internet
Run compute acceleration for AI without render or presentation target


== Swapchains ==

To render we require framebuffers

No concept of default framebuffer
Requires infrastructure that will own the framebuffers we will render 
We visualize them to the screen after

creation is Explicit 

Swapchain short definition - essentially a queue of images that are waiting to be presented to the screen

Our application acquire image to draw to it, returns it to the queue
How exactly the queue works and the conditions for presenting an image from the queue depend on how 
the swap chain is setup

== Checking for swapchain support ==

Not all graphics cards are capable of presenting images directly to a screen for various reasons,
for example, because they are designed for servers and don't have any display outputs.

Secondly, image presentation is heavily tied into window system and the surface associated with the windows.
It is not part of Vulkan core.
We enable the VK_KHR_swapchain device extension after querying for it's support.








