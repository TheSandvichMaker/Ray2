#ifndef RAY_MATH_H
#define RAY_MATH_H

#include <xmmintrin.h>
#include <math.h>

//
// Types
// 

typedef float v2 __attribute__((ext_vector_type(2)));
typedef float v3 __attribute__((ext_vector_type(3)));
typedef float v4 __attribute__((ext_vector_type(4)));

#define V2(...) (v2){__VA_ARGS__}
#define V3(...) (v3){__VA_ARGS__}
#define V4(...) (v4){__VA_ARGS__}

//
// Scalar Functions
//

#define Square(x) ((x)*(x))

static inline float
SquareRootF(float A) {
    return sqrtf(A);
}

//
// Overloaded Functions
//

static inline float
MinF(float A, float B) {
    return A < B ? A : B;
}

static inline v2
MinV2(v2 A, v2 B) {
    return V2(MinF(A.x, B.x),
              MinF(A.y, B.y));
}

static inline v3
MinV3(v3 A, v3 B) {
    return V3(MinF(A.x, B.x),
              MinF(A.y, B.y),
              MinF(A.z, B.z));
}

static inline v4
MinV4(v4 A, v4 B) {
    return V4(MinF(A.x, B.x),
              MinF(A.y, B.y),
              MinF(A.z, B.z),
              MinF(A.w, B.w));
}

#define Min(A, B) _Generic((A), float: MinF, v2: MinV2, v3: MinV3, v4: MinV4)(A, B)

static inline float
MaxF(float A, float B) {
    return A > B ? A : B;
}

static inline v2
MaxV2(v2 A, v2 B) {
    return V2(MaxF(A.x, B.x),
              MaxF(A.y, B.y));
}

static inline v3
MaxV3(v3 A, v3 B) {
    return V3(MaxF(A.x, B.x),
              MaxF(A.y, B.y),
              MaxF(A.z, B.z));
}

static inline v4
MaxV4(v4 A, v4 B) {
    return V4(MaxF(A.x, B.x),
              MaxF(A.y, B.y),
              MaxF(A.z, B.z),
              MaxF(A.w, B.w));
}

#define Max(A, B) _Generic((A), float: MaxF, v2: MaxV2, v3: MaxV3, v4: MaxV4)(A, B)

static inline float
ClampF(float A, float Lo, float Hi) {
    return Min(Lo, Max(Hi, A));
}

static inline v2
ClampV2(v2 A, v2 Lo, v2 Hi) {
    return V2(ClampF(A.x, Lo.x, Hi.x),
              ClampF(A.y, Lo.y, Hi.y));
}

static inline v3
ClampV3(v3 A, v3 Lo, v3 Hi) {
    return V3(ClampF(A.x, Lo.x, Hi.x),
              ClampF(A.y, Lo.y, Hi.y),
              ClampF(A.z, Lo.z, Hi.z));
}

static inline v4
ClampV4(v4 A, v4 Lo, v4 Hi) {
    return V4(ClampF(A.x, Lo.x, Hi.x),
              ClampF(A.y, Lo.y, Hi.y),
              ClampF(A.z, Lo.z, Hi.z),
              ClampF(A.w, Lo.w, Hi.w));
}

#define Clamp(A, Lo, Hi) _Generic((A), float: ClampF, v2: ClampV2, v3: ClampV3, v4: ClampV4)(A, Lo, Hi)

static inline float
DotV2(v2 A, v2 B) {
    return A.x*B.x + A.y*B.y;
}

static inline float
DotV3(v3 A, v3 B) {
    return A.x*B.x + A.y*B.y + A.z*B.z;
}

static inline float
DotV4(v4 A, v4 B) {
    return A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;
}

#define Dot(A, B) _Generic((A), v2: DotV2, v3: DotV3, v4: DotV4)(A, B)

static inline float
LengthSqV2(v2 A) {
    return Dot(A, A);
}

static inline float
LengthSqV3(v3 A) {
    return Dot(A, A);
}

static inline float
LengthSqV4(v4 A) {
    return Dot(A, A);
}

#define LengthSq(A) _Generic((A), v2: LengthSqV2, v3: LengthSqV3, v4: LengthSqV4)(A)
#define Length(A) SquareRoot(LengthSq(A))
#define Normalize(A) ((1.0f / Length(A))*(A))
#define Reflect(A, B) ((A) - 2.0f*Dot(A, B)*(B))
#define Lerp(A, B, t) ((1.0f - t)*(A) + (B))

//
// V2 Functions
//

static inline v2
Perp(v2 A) {
    return V2(-A.y, A.x);
}

//
// V3 Functions
//

static inline v3
Cross(v3 A, v3 B) {
    return V3(A.y*B.z - A.z*B.y,
              A.z*B.x - A.x*B.z,
              A.x*B.y - A.y*B.x);
}

#endif /* RAY_MATH_H */
