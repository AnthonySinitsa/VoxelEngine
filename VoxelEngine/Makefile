CFLAGS = -std=c++20 -O2

LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

SRC_DIR = src
SRC_FILES = $(shell find $(SRC_DIR) -name "*.cpp")

SHADER_SCRIPT = compile.sh
SHADER_FILES = shaders/vertex_shader.vert shaders/fragment_shader.frag

VoxelOutput: $(SRC_FILES)
	g++ $(CFLAGS) -o VoxelOutput $(SRC_FILES) $(LDFLAGS)

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
