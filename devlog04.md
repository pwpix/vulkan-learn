
Dynamic Rendering

Quick introduction

In previous versions  of Vulkan, a key requirement was specifying the framebuffer attachments that would
be used while rendering through a render pass object. 
With introduction to Dynamic Rendering, Vulkan allows us to specify this information directly when creating
the graphics pipeline and recording command buffers.



Practical advantages of Dynamic rendering:

Dynamic rendering simplifies the rendering process by eliminating the need for render pass and framebuffer objects.
Instead, we can specify the color, depth, and stencil attachments directly when we begin rendering
It provides more flexibility by allowing us to change the attachments we're rendering to without creating new render pass objects.

Command buffer recording

//More in Drawing chapter


A pipeline cache object can be used to store and reuse data relevant to pipeline creation across multiple calls to vk::Pipeline constructors
or across program executions if data is stored into a file. This makes it possible to significantly speed up pipeline creation at a later time.

Next step is setting up dynamic rendering for swapchain images and preparing the drawing commands.
