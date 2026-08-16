
27/02/2026

Setting Up Graphics Pipeline


What we will be doing:
    - Configure graphics pipeline to draw our first triangle


== Overview of  Graphics Pipeline ==

The graphics pipeline is a sequence of operations that take the vertices and textures of meshes
all the way to pixel targets.

Main components

Input assembler: collects raw vertex and index data

Vertex shader: process vertices

Tesselation shaders: allow subdivide geometry based on certain rules to increase mesh quality

Geometry shader: is run on primitive (triangle, line or point) and can discard it or output more primitives.
More modern solution is to use Mesh shader pipeline
Exists outside the standard Graphics pipeline setup.

Rasterization: break primitive into fragments.
They are pixel elements
Fill the framebuffer.
Pixels outside the framebuffer are discarded.

Fragment shader: determines which framebuffer each fragment is written to and with which color and depth values.

Color blending: operations to mix different fragments that map to the same pixel in the framebuffer. 
Can overwrite each other or mixed based on transparency.


Graphics pipeline in Vulkan is immutable.
For e.g., cannot change BlendStage like OpenGL/DirectX

Must recreate the pipeline from scratch to change shaders, bind different framebuffers or change blend function.

Downsides:
Have to create a number of pipelines representing all the different combinations.
Drive can optimize, reason all operations we will be doing are known in advance.


Optional stages:
Tesselation 
Geometry

For shadow mapping, 
we disable fragment stage

We write the shaders and compile them.
A more convenient way is to create a CMake function

We compile slang shaders and create shader modules





