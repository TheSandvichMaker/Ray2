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
ASinF(float A)
{
    f32 Result = asinf(A);
    return Result;
}

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

static inline float
CopySignF(float ValueOf, float SignOf)
{
#ifdef HANDMADE_MATH__USE_SSE
    // NOTE: This produces trash code on GCC, but I don't care about GCC very much
#if 0
    __m128 SignMask = _mm_castsi128_ps(_mm_set1_epi32(1 << 31));
    __m128 NotSignMask = _mm_castsi128_ps(_mm_set1_epi32(~(1 << 31)));
    float Result = _mm_cvtss_f32(_mm_or_ps(_mm_and_ps(SignMask, _mm_set_ss(SignOf)),
                                           _mm_and_ps(NotSignMask, _mm_set_ss(ValueOf))));
#else
    __m128 SignMask = _mm_castsi128_ps(_mm_set1_epi32(1 << 31));
    float Result = _mm_cvtss_f32(_mm_blendv_ps(_mm_set_ss(SignOf), _mm_set_ss(ValueOf), SignMask));
#endif
#else
    int SignAsBits = *(int *)&SignOf;
    int ValueAsBits = *(int *)&ValueOf;
    ValueAsBits = (SignAsBits & SignMask)|(ValueAsBits & ~SignMask);
    float Result = *(float *)&ValueOf;
#endif
    return Result;
}

static inline vec3
Negate(vec3 A)
{
    vec3 Result;
    Result.X = -A.X;
    Result.Y = -A.Y;
    Result.Z = -A.Z;
    return Result;
}

static inline vec3
operator-(vec3 A)
{
    return Negate(A);
}

static inline float
Max3(vec3 A)
{
    return MaxF(A.X, MaxF(A.Y, A.Z));
}

#endif /* RAY_HANDMADE_MATH_H */
