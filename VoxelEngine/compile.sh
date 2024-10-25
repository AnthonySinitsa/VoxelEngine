# default vertex and fragment
/usr/bin/glslc shaders/vertex_shader.vert -o shaders/vertex_shader.vert.spv
/usr/bin/glslc shaders/fragment_shader.frag -o shaders/fragment_shader.frag.spv

# point light
/usr/bin/glslc shaders/point_light_vertex.vert -o shaders/point_light_vertex.vert.spv
/usr/bin/glslc shaders/point_light_fragment.frag -o shaders/point_light_fragment.frag.spv

# galaxy
/usr/bin/glslc shaders/galaxy_vertex.vert -o shaders/galaxy_vertex.vert.spv
/usr/bin/glslc shaders/galaxy_fragment.frag -o shaders/galaxy_fragment.frag.spv
# /usr/bin/glslc shaders/galaxy_compute.comp -o shaders/galaxy_compute.comp.spv
