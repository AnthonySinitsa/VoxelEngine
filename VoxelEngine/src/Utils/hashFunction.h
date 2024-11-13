#pragma once

#include <cstdint> // For uint32_t

#ifdef __cplusplus
namespace vge {
#endif

// Integer hash copied from Hugo Elias
// Returns a value between 0 and 1
inline float hash(uint32_t n) {
    n = (n << 13U) ^ n;
    n = n * (n * n * 15731U + 0x789221U) + 0x1376312589U;
    return float(n & uint32_t(0x7fffffffU)) / float(0x7fffffff);
}

// Variant that takes a float and returns a float
inline float hash(float n) {
    return hash(static_cast<uint32_t>(n));
}

// 2D hash function useful for 2D coordinates
inline float hash2(float x, float y) {
    return hash(static_cast<uint32_t>(x * 1847.0f + y * 374.0f));
}

// 3D hash function useful for 3D coordinates
inline float hash3(float x, float y, float z) {
    return hash(static_cast<uint32_t>(x * 1847.0f + y * 374.0f + z * 169.0f));
}

#ifdef __cplusplus
}  // namespace
#endif



/*

USE THIS BELOW TO COPY TO SHADER FILE

// Integer hash copied from Hugo Elias
// Returns a value between 0 and 1
float hash(uint n) {
    n = (n << 13U) ^ n;
    n = n * (n * n * 15731U + 0x789221U) + 0x1376312589U;
    return float(n & uint(0x7fffffffU)) / float(0x7fffffff);
}

// Variant that takes a float and returns a float
float hash(float n) {
    return hash(uint(n));
}

// 2D hash function useful for 2D coordinates
float hash2(float x, float y) {
    return hash(uint(x * 1847.0f + y * 374.0f));
}

// 3D hash function useful for 3D coordinates
float hash3(float x, float y, float z) {
    return hash(uint(x * 1847.0f + y * 374.0f + z * 169.0f));
}

*/
