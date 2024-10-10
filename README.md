# VoxelEngine

[Project Board](https://trello.com/b/2YI795DN/voxelengine)

## Project Layout

![Windowing](VoxelEngine/images/Windowing.png)

1. GLFW Window

    Purpose: The GLFW Window is a high-level abstraction used to create and manage windows, input, and OpenGL/Vulkan contexts across different platforms. GLFW is a cross-platform library that simplifies window management and input handling.
    Interaction: In this diagram, the GLFW window is responsible for creating a window on the operating system and managing input events. This window will serve as the target for rendering content. However, since GLFW itself is not directly tied to Vulkan, it needs to be linked to the Vulkan API via a Vulkan Window Surface.

2. Vulkan Window Surface

    Purpose: A Vulkan Window Surface is an interface between Vulkan and the windowing system. It represents the surface on which Vulkan will render its images. Vulkan itself does not inherently know how to interact with the OS window system, so the surface provides an abstraction that allows Vulkan to draw to the window created by GLFW (or other windowing libraries).
    Interaction: The Vulkan surface is created from the GLFW window and is passed into Vulkan's rendering pipeline. It is used by the Swapchain, which manages the images that will be rendered to this surface.

3. Swapchain

    Purpose: The Swapchain is a key component in Vulkan's rendering process. It is responsible for managing a set of images (usually referred to as "swapchain images") that are presented to the screen. The swapchain works in conjunction with the window surface to handle the double or triple buffering needed for smooth rendering.
    Interaction: The swapchain acquires images from a pool and presents them to the window surface. It ensures that images are ready to be drawn to the screen at the right time, handling synchronization between rendering and presentation to avoid tearing or stuttering. The images in the swapchain are used by the Framebuffer in the rendering process.

4. Framebuffer

    Purpose: The Framebuffer is a collection of memory attachments (usually including color, depth, and stencil attachments) that Vulkan uses to store the result of a rendering operation. The framebuffer represents the intermediate storage for the pixels before they are presented on the screen.
    Interaction: The framebuffer is linked to one of the images from the Swapchain. After rendering commands are executed, the results are written to the framebuffer, which is then copied to the swapchain image for display. The framebuffer interacts with the Image object, which stores the actual pixel data.

5. Image

    Purpose: An Image in Vulkan represents a collection of pixels in memory that can be used for various purposes, including rendering and texturing. The image holds the memory where the pixel data is stored.
    Interaction: The Framebuffer uses an Image to store the pixels resulting from the rendering process. The Image is then used by the Swapchain to present to the window surface. The image has two main components:
        Memory: This is the actual memory allocated to store the image's pixel data. Vulkan manages memory explicitly, so the application needs to allocate and bind memory to the image.
        View: The Image View is how the image is interpreted. It defines how Vulkan views the image (e.g., as a 2D texture, 3D texture, or color attachment). The image view is used when the image is attached to a framebuffer.

How They Work Together

    The GLFW Window is created and displayed on the screen. It serves as the container for rendering.
    A Vulkan Window Surface is created from the GLFW window, allowing Vulkan to present images to the window.
    The Swapchain is created for the surface. It manages multiple images (usually two or more) that Vulkan will render to and present on the screen. This ensures smooth rendering through double or triple buffering.
    The Framebuffer is used in the rendering pipeline to store the output of rendering commands. It is linked to one of the images from the swapchain.
    Each Image in the swapchain has Memory allocated to store pixel data, and an Image View that defines how the image is interpreted by Vulkan.
    The images in the swapchain are continuously updated with new frame data from the Framebuffer, and then presented to the Vulkan Window Surface, which displays the rendered content in the GLFW Window.

Summary of Flow:

    The GLFW Window is presented on the screen.
    A Vulkan Window Surface is created to link Vulkan to the window.
    A Swapchain manages the images to be rendered and displayed.
    A Framebuffer stores the pixel data from rendering operations.
    An Image holds the pixel data in memory and provides a view of how that memory is used (e.g., as a 2D image).
    The Swapchain presents the framebuffer image to the surface, which shows it in the window.

![Graphics Pipeline](VoxelEngine/images/GraphicsPipeline.png)

1. Renderpass

    Purpose: A Renderpass is a key concept in Vulkan that defines how rendering will occur, specifying the attachments (such as color, depth, and stencil buffers) and their operations (e.g., clear, load, store). It essentially describes the rendering process to the framebuffer.
    Subpass: Within a renderpass, a Subpass represents a single rendering operation. Multiple subpasses can occur within a renderpass, allowing the results of one subpass to be fed into the next without having to write to memory in-between. Subpasses are useful for optimizing rendering performance.
    Interaction: The renderpass is tied directly to the Graphics Pipeline. The pipeline needs to know which renderpass to use because it determines how the framebuffers will be used and how the rendering results will be stored.

2. Graphics Pipeline

    Purpose: The Graphics Pipeline defines the entire configuration for rendering, including how vertices are processed, how fragments are shaded, and how the final image is written to the framebuffer. It is a collection of state configurations and shader stages that are compiled into a fixed pipeline, which is extremely efficient to execute but inflexible once created.
    Components:
        Stage Configurations: These define various stages of the pipeline, including input assembly, vertex shading, tessellation (if used), geometry shading, rasterization, fragment shading, and output to the framebuffer.
        Renderpass: The pipeline is tied to a specific Renderpass, which dictates how rendering results are handled (clearing, loading, storing attachments).
        Descriptor Set Layouts: These define how resources such as textures, buffers, and samplers are accessed by the shaders. Descriptor sets allow shaders to access data in a way that is efficient and flexible.
        Shader Modules: These contain the compiled shader programs in SPIR-V format. The pipeline loads these compiled shaders and uses them during the various stages of rendering (e.g., vertex, fragment).
    Interaction: The Graphics Pipeline ties together all the necessary configurations, including the renderpass, shader modules, and descriptor sets. It dictates how the rendering commands will be processed and how the GPU will execute the rendering tasks.

3. Shader (glsl)

    Purpose: GLSL (OpenGL Shading Language) is a high-level shading language used to write shaders in a more human-readable form. It is similar in syntax to C/C++ and allows developers to write the logic that runs on the GPU for various stages of the pipeline (vertex, fragment, etc.).
    Interaction: While GLSL is easy to write, the Vulkan pipeline does not directly accept GLSL. Instead, GLSL shaders must be compiled into SPIR-V (Standard Portable Intermediate Representation), which is a binary format that Vulkan understands.

4. Shader (SPIR-V)

    Purpose: Once the GLSL shaders are compiled, they are converted into SPIR-V, which is a low-level, intermediate representation of the shader code. SPIR-V is a portable binary format optimized for high performance. Vulkan requires shaders to be in SPIR-V format for use in the graphics pipeline.
    Interaction: The SPIR-V shaders are loaded into the graphics pipeline as Shader Modules, which are responsible for executing the shader stages (e.g., vertex and fragment processing). SPIR-V shaders are compiled once and can be reused across different pipelines, as long as the pipeline stages align.

5. Pipeline Layout

    Purpose: The Pipeline Layout defines the organization of resources that the shaders will use, such as buffers, textures, and samplers. It describes the layout of descriptor sets and push constants that the shaders will access during rendering.
    Interaction: The Pipeline Layout is used by the Graphics Pipeline to define how the resources are bound and accessed. It includes the Descriptor Set Layouts, which describe how descriptors (i.e., resources) are structured and accessed by the shaders.

How They Work Together

    Shader Creation:
        Shaders are written in GLSL (or another high-level language), which is then compiled into SPIR-V. The SPIR-V shaders are loaded into the Graphics Pipeline as Shader Modules.

    Renderpass and Subpass:
        A Renderpass is created, which defines how attachments (color, depth, etc.) will be used during rendering. The renderpass can include multiple Subpasses, which allow for efficient reuse of intermediate rendering results.

    Pipeline Layout:
        The Pipeline Layout is configured with Descriptor Set Layouts, which describe how textures, buffers, and other resources are accessed by the shaders. This layout is then passed into the Graphics Pipeline for use during rendering.

    Graphics Pipeline:
        The Graphics Pipeline is set up with all the necessary configurations, including the Renderpass, Shader Modules, Descriptor Set Layouts, and Stage Configurations. This pipeline defines how the GPU will process the rendering commands.

    Execution:
        When rendering begins, the Graphics Pipeline is bound, and the Renderpass is started. The pipeline then uses the compiled SPIR-V shaders to process vertices, apply textures, and compute fragment colors, finally writing the results to the framebuffer according to the renderpass.

Summary of Flow:

    Write shaders in GLSL, compile them to SPIR-V, and load them into the Graphics Pipeline as Shader Modules.
    Define a Renderpass with one or more Subpasses to control how rendering results are handled.
    Configure the Pipeline Layout and Descriptor Set Layouts to specify how resources are accessed by the shaders.
    Set up the Graphics Pipeline with the shaders, renderpass, stage configurations, and descriptor sets.
    Execute the rendering commands, using the Graphics Pipeline to process vertices, fragments, and resources, and output the final image through the renderpass.

![Drawing](VoxelEngine/images/Drawing.png)

1. Queue

    Purpose: A Queue in Vulkan represents a series of commands submitted to the GPU for execution. Queues are used to schedule and manage rendering and compute operations on the GPU.
    Interaction: The Queue processes Command Buffers that are submitted to it. In this diagram, the queue receives command buffers (created in the command pool) and executes them on the GPU.

2. Command Pool

    Purpose: The Command Pool is a memory space from which Command Buffers are allocated. Command buffers are used to record and store rendering and compute commands that are then submitted to the GPU for execution.
    Interaction: The Command Pool allocates Command Buffers. These buffers are used by the Draw Loop to record rendering commands, which are then submitted to the Queue for execution.

3. Command Buffer

    Purpose: A Command Buffer is the primary mechanism for recording Vulkan commands that are submitted to the GPU for execution. Commands such as binding pipelines, setting descriptor sets, and drawing are recorded into command buffers.
    Interaction: The Command Buffer is allocated from the Command Pool and recorded with rendering commands. Once the command buffer is filled, it is submitted to the Queue for execution. The Draw Loop will continuously record new commands to the command buffer each frame.

4. Swapchain

    Purpose: The Swapchain manages the images that are presented to the window surface. It handles the back-and-forth swapping of images between rendering and presentation, ensuring smooth rendering.
    Interaction: The Draw Loop acquires images from the Swapchain, rendering to them and presenting them back to the window. The Pipeline is responsible for executing the rendering process on the images acquired from the swapchain.

5. Draw Loop

    Purpose: The Draw Loop represents the main rendering loop in a Vulkan application. It repeatedly acquires images from the swapchain, submits command buffers, and presents the rendered images to the screen.
    Interaction: The Draw Loop coordinates the rendering process. It interacts with the Swapchain to acquire images, the Command Buffer to submit rendering commands, and the Pipeline to execute the rendering stages. The loop continuously runs to generate new frames for display.

6. Pipeline

    Purpose: The Pipeline defines the entire configuration for rendering, including how vertices are transformed, how fragments are shaded, and how the output image is generated. It includes the shader stages, fixed-function state, and resource bindings.
    Interaction: The Pipeline is bound to the Command Buffer during recording, and it dictates how the rendering commands are processed. The pipeline uses the Descriptor Sets to access resources like textures, buffers, and matrices during rendering.

7. Descriptor Set Layout

    Purpose: The Descriptor Set Layout defines how resources, such as buffers, images, and samplers, are organized and accessed by the shaders. It specifies what types of resources are needed and how they are laid out in memory.
    Interaction: The Pipeline references the Descriptor Set Layout to understand how to bind resources. The Descriptor Pool allocates Descriptor Sets that conform to the layout, and these sets are used during rendering.

8. Descriptor Pool

    Purpose: The Descriptor Pool manages memory for Descriptor Sets. It allocates descriptor sets that are used by the pipeline to access resources like buffers and images.
    Interaction: The Descriptor Pool creates and manages multiple Descriptor Sets that conform to the Descriptor Set Layout. These sets are then bound to the pipeline during rendering to provide access to resources.

9. Descriptor Set

    Purpose: A Descriptor Set is a collection of resource bindings (e.g., buffers, images, samplers) that are used by shaders during rendering. It provides a way for shaders to access external data needed for rendering (e.g., textures, transformation matrices).
    Interaction: The Descriptor Set is created from the Descriptor Pool and is based on the Descriptor Set Layout. The pipeline uses the descriptor set to bind resources like Matrix, Image, and Buffer, which are used in the shaders for rendering.

10. Matrix

    Purpose: The Matrix is likely a transformation matrix (such as a model, view, or projection matrix) that is used by the shaders to transform vertices. It could represent transformations like scaling, rotation, or translation applied to objects in the scene.
    Interaction: The Matrix is stored in a buffer and accessed by the shaders through the Descriptor Set. It plays a key role in vertex processing, transforming vertices from object space to screen space.

11. Image

    Purpose: The Image represents a texture or framebuffer image that is used in rendering. It could be a texture map applied to a 3D model or a render target where the final image is written.
    Interaction: The Image is bound to the pipeline through the Descriptor Set and accessed by the shaders. For example, shaders may sample from the image to apply textures to objects or use it as a render target.

12. Buffer

    Purpose: A Buffer in Vulkan is a memory resource used to store data such as vertex attributes, indices, or uniform data. Buffers are often used to store large arrays of data that are accessed by the GPU during rendering.
    Interaction: The Buffer is bound to the pipeline through the Descriptor Set. It can contain data like vertex positions, normals, colors, or uniform data that are accessed by the shaders during rendering.

How They Work Together

    Command Recording:
        Command Buffers are allocated from the Command Pool and filled with rendering commands in the Draw Loop.
        These commands include binding the Pipeline, setting the Descriptor Sets, and issuing draw commands.

    Queues and Command Submission:
        When the command buffer is ready, it is submitted to the Queue, which processes the commands and sends them to the GPU for execution.

    Swapchain and Rendering:
        The Draw Loop continuously interacts with the Swapchain to acquire images, render to them, and present the final result to the window.
        The Pipeline is responsible for executing the rendering process, processing vertices, applying shaders, and writing the final image.

    Resource Binding:
        The Descriptor Sets provide access to various resources, such as the Matrix, Image, and Buffer. These resources are bound to the pipeline and accessed by the shaders during rendering.
        The Descriptor Pool manages the allocation of descriptor sets, while the Descriptor Set Layout defines how the resources are structured.

    Continuous Drawing:
        The Draw Loop repeats the process of acquiring images, recording commands, and submitting them to the queue, ensuring that the application renders continuously, frame by frame.

Summary of Flow:

    Command Buffers are recorded with drawing commands and submitted to the Queue.
    The Draw Loop acquires images from the Swapchain, submits commands to the Pipeline, and presents the rendered images.
    The Pipeline uses Descriptor Sets to access resources like Matrices, Images, and Buffers.
    The Descriptor Pool manages descriptor sets, and the Descriptor Set Layout defines how resources are organized.
    The Queue executes the commands, and the rendering process is repeated each frame for continuous drawing.

<!-- command to run on linux `make test` --->
