#ifndef RAY_HANDMADE_MATH_H
#define RAY_HANDMADE_MATH_H

#define HMM_PREFIX(X) X
#define HANDMADE_MATH_IMPLEMENTATION
#define HMM_EXTERN static
#include "external/HandmadeMath.h"

typedef hmm_vec2 vec2;
typedef hmm_vec3 vec3;
typedef hmm_vec4 vec4;
typedef hmm_mat4 mat4;
typedef hmm_quaternion quaternion;

static inline float
MinF(float A, float B)
{
    return A < B ? A : B;
}

static inline float
MaxF(float A, float B)
{
    return A > B ? A : B;
}

static inline vec3
Reflect(vec3 D, vec3 N)
{
    return D - 2.0f*Dot(N, D)*N;
}

#endif /* RAY_HANDMADE_MATH_H */
