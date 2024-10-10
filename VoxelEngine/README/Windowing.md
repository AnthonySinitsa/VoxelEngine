# Windowing

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