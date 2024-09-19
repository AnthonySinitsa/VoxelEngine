CFLAGS = -std=c++20 -O2

LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

# Paths to ImGui
IMGUI_DIR = $(shell echo ~/Projects/ExternalLibs/imgui)
IMGUI_BACKENDS_DIR = $(IMGUI_DIR)/backends

# ImGui source files
IMGUI_SRC = $(IMGUI_DIR)/imgui.cpp \
            $(IMGUI_DIR)/imgui_draw.cpp \
            $(IMGUI_DIR)/imgui_tables.cpp \
            $(IMGUI_DIR)/imgui_widgets.cpp \
            $(IMGUI_BACKENDS_DIR)/imgui_impl_vulkan.cpp \
            $(IMGUI_BACKENDS_DIR)/imgui_impl_glfw.cpp

SRC_DIR = src
SRC_FILES = $(shell find $(SRC_DIR) -name "*.cpp")

# ImGui include directories
INCLUDES = -I$(IMGUI_DIR) -I$(IMGUI_BACKENDS_DIR)

SHADER_SCRIPT = compile.sh
SHADER_FILES = shaders/vertex_shader.vert shaders/fragment_shader.frag

# Main build target
VoxelOutput: $(SRC_FILES) $(IMGUI_SRC)
	g++ $(CFLAGS) $(INCLUDES) -o VoxelOutput $(SRC_FILES) $(IMGUI_SRC) $(LDFLAGS)

.PHONY: test clean shaders

shaders: $(SHADER_FILES)
	chmod +x $(SHADER_SCRIPT)
	./$(SHADER_SCRIPT)

test: shaders VoxelOutput
	./VoxelOutput

clean:
	rm -f VoxelOutput

shaders/vertex_shader.vert.spv: shaders/vertex_shader.vert
shaders/fragment_shader.frag.spv: shaders/fragment_shader.frag
