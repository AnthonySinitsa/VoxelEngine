# Find installed Vulkan SDK using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(VULKAN REQUIRED vulkan)
set(Vulkan_INCLUDE_DIRS ${VULKAN_INCLUDE_DIRS})
set(Vulkan_LIBRARIES ${VULKAN_LIBRARIES})
