CFLAGS = -std=c++20 -O2

LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

# Paths to ImGui
IMGUI_DIR = ./external/ImGuiDocking
IMGUI_BACKENDS_DIR = $(IMGUI_DIR)/backends

# Path to tinyobjloader
TINYOBJ_PATH = external/tinyobjloader

# ImGui source files
IMGUI_SRC = $(IMGUI_DIR)/imgui.cpp \
            $(IMGUI_DIR)/imgui_demo.cpp \
            $(IMGUI_DIR)/imgui_draw.cpp \
            $(IMGUI_DIR)/imgui_tables.cpp \
            $(IMGUI_DIR)/imgui_widgets.cpp \
            $(IMGUI_BACKENDS_DIR)/imgui_impl_vulkan.cpp \
            $(IMGUI_BACKENDS_DIR)/imgui_impl_glfw.cpp

SRC_DIR = src
SRC_FILES = $(shell find $(SRC_DIR) -name "*.cpp")
OBJ_DIR = obj
OBJ_FILES = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES)) $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(IMGUI_SRC))

# Include directories
INCLUDES = -I$(IMGUI_DIR) -I$(IMGUI_BACKENDS_DIR) -I$(TINYOBJ_PATH)

SHADER_SCRIPT = compile.sh
SHADER_FILES = shaders/vertex_shader.vert \
               shaders/fragment_shader.frag \
               shaders/galaxy_vertex.vert \
               shaders/galaxy_fragment.frag \
               shaders/point_light_vertex.vert \
               shaders/point_light_fragment.frag

# Main build target
VoxelOutput: $(OBJ_FILES)
	g++ $(CFLAGS) -o VoxelOutput $(OBJ_FILES) $(LDFLAGS)

# Rule to compile each source file into an object file in the obj directory
$(OBJ_DIR)/%.o: %.cpp
	mkdir -p $(@D)  # Ensure the obj directory exists
	g++ $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: test clean shaders

shaders: $(SHADER_FILES)
	chmod +x $(SHADER_SCRIPT)
	./$(SHADER_SCRIPT)

test: shaders VoxelOutput
	./VoxelOutput

clean:
	rm -f VoxelOutput
	rm -f shaders/*.spv
	rm -rf $(OBJ_DIR)

# Shader compilation rules
shaders/vertex_shader.vert.spv: shaders/vertex_shader.vert
shaders/fragment_shader.frag.spv: shaders/fragment_shader.frag
shaders/galaxy_vertex.vert.spv: shaders/galaxy_vertex.vert
shaders/galaxy_fragment.frag.spv: shaders/galaxy_fragment.frag
shaders/point_light_vertex.vert.spv: shaders/point_light_vertex.vert
shaders/point_light_fragment.frag.spv: shaders/point_light_fragment.frag
