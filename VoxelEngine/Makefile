CFLAGS = -std=c++20 -O2

LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

VoxelOutput: src/main.cpp
	g++ $(CFLAGS) -o VoxelOutput src/main.cpp $(LDFLAGS)

.PHONY: test clean

test: VoxelOutput
	./VoxelOutput

clean:
	rm -f VoxelOutput
