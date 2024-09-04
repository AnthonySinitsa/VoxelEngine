CFLAGS = -std=c++20 -O2

LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

SRC_DIR = src
SRC_FILES = $(shell find $(SRC_DIR) -name "*.cpp")

VoxelOutput: $(SRC_FILES)
	g++ $(CFLAGS) -o VoxelOutput $(SRC_FILES) $(LDFLAGS)

.PHONY: test clean

test: VoxelOutput
	./VoxelOutput

clean:
	rm -f VoxelOutput
