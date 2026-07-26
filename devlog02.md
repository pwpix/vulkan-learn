
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


Checking swapchain support is available is not enough
because it may not be compatible with our window surface.

Basically three kinds of properties we need to check:
1. Basic surface capabilites (min/max number of images in swapchain, min/max width and height of images)
2. Surface formats (pixel format, color space)
3. Available presentation modes


Start with basic surface capabilites 
These properties are straightforward to query and returned into a single 'vk::SurfaceCapabilitiesKHR' struct

Surface is the core component of the swap chain
Querying functions have it as a first parameter

There may still be many different modes of varying optimally in the swap chain.
We find the right settings for the best possible swapchain.
Three types of settings to determine:
1. Surface format (color depth)
2. Presentation mode (conditions for swapping images to the screen)
3. Swap extent (resolution of image in swapchain)


Presentation mode is arguably the most important thing in swap chain

Theres various modes available:
1. immediate
2. fifo
3. fifo relaxed
4. mail box (triple buffer)

Only fifo is guranteed to be available


== Swap extent ==
Swap extent is the resolution of swapchain images. It is almost exactly equal to the resolution of the window.
that we are drawing to, in pixels.

The range of possible resolutions is defined in the vk::SurfaceCapabilitiesKHR struct


Request one more image than the minimum since we may sometimes have to wait for the
driver to complete internal operations before we can acquire another image (recommmended)


Swapchain can be invalidated on window resize
in that case we re create swapchain from scratch


== Image Views ==
To use any vk::Image including those in the swap chain, in the 
render pipeline we have to create a vk::raii::ImageView object

An image view is quite literally a view into an image. 

Describes how to access the image and which part of the image to
access.

An image view is sufficient to start using an image as a texture, but it's 
not quite ready to be used as a render target.




