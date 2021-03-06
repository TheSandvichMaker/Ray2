#ifndef ROGUE_MATH_H
#define ROGUE_MATH_H

#include <xmmintrin.h>
#include <math.h>

#define PI_32  3.14159265359f
#define TAU_32 6.28318530717f
#define DEG_TO_RAD (TAU_32/360.0f)
#define RAD_TO_DEG (360.0f/TAU_32)
#define SQRT_2 1.41421356237f

//
// NOTE: Scalar operations
//

function s32 div_floor(s32 a, s32 b) {
    Assert(b != 0);
    s32 res = a / b;
    s32 rem = a % b;
    s32 corr = (rem != 0 && ((rem < 0) != (b < 0)));
    return res - corr;
}

function f32 deg_to_rad(f32 deg) {
    f32 rad = deg*DEG_TO_RAD;
    return rad;
}

function f32 rad_to_deg(f32 rad) {
    f32 deg = rad*RAD_TO_DEG;
    return deg;
}

function f32 square(f32 a) {
    return a*a;
}

function s32 square(s32 a) {
    return a*a;
}

function u32 square(u32 a) {
    return a*a;
}

function f32 lerp(f32 a, f32 b, f32 t) {
    return a*(1.0f - t) + b*t;
}

#undef max
function f32 max(f32 a, f32 b) {
    return a > b ? a : b;
}

function f64 max(f64 a, f64 b) {
    return a > b ? a : b;
}

function s32 max(s32 a, s32 b) {
    return a > b ? a : b;
}

function s64 max(s64 a, s64 b) {
    return a > b ? a : b;
}

#undef min
function f32 min(f32 a, f32 b) {
    return a < b ? a : b;
}

function f64 min(f64 a, f64 b) {
    return a < b ? a : b;
}

function s32 min(s32 a, s32 b) {
    return a < b ? a : b;
}

function s64 min(s64 a, s64 b) {
    return a < b ? a : b;
}

#undef clamp
function f32 clamp(f32 n, f32 lo, f32 hi) {
    return max(lo, min(hi, n));
}

function f64 clamp(f64 n, f64 lo, f64 hi) {
    return max(lo, min(hi, n));
}

#define abs sd_abs
function f32 abs(f32 x) {
    f32 result = (x < 0.0f ? -x : x);
    return result;
}

function s32 abs(s32 x) {
    s32 result = (x < 0 ? -x : x);
    return result;
}

#define clamp01(n) clamp((n), 0, 1)
#define clamp01_f64(n) clamp_f64((n), 0, 1)

// Source: https://www.iquilezles.org/www/articles/smin/smin.htm 
function f32 smooth_min(f32 a, f32 b, f32 k) {
    f32 h = max(k - abs(a - b), 0.0f) / k;
    return min(a, b) - 0.25f*h*h*k;
}

function f32 map_to_range(f32 t, f32 min, f32 max) {
    f32 result = 0.0f;
    f32 range = max - min;
    if (range != 0.0f) {
        result = (t - min) / range;
    }
    return result;
}

function f32 clamp01_map_to_range(f32 t, f32 min, f32 max) {
    f32 result = clamp01(map_to_range(t, min, max));
    return result;
}

function f32 safe_ratio_n(f32 numerator, f32 divisor, f32 n) {
    f32 result = n;
    
    if (divisor != 0.0f) {
        result = numerator / divisor;
    }
    
    return result;
}

function f32 safe_ratio_0(f32 numerator, f32 divisor) {
    f32 result = safe_ratio_n(numerator, divisor, 0.0f);
    return result;
}

function f32 safe_ratio_1(f32 numerator, f32 divisor) {
    f32 result = safe_ratio_n(numerator, divisor, 1.0f);
    return result;
}

function f32 neighborhood_distance(f32 a, f32 b, f32 period) {
    f32 d0 = abs(a - b);
    f32 d1 = abs(a - period*sign_of(a) - b);
    f32 result = min(d0, d1);
    return result;
}

function f32 smoothstep(f32 x) {
    f32 result = x*x*(3.0f - 2.0f*x);
    return result;
}

function f32 smootherstep(f32 x) {
    f32 result = x*x*x*(x*(x*6.0f - 15.0f) + 10.0f);
    return result;
}

//
// NOTE: V2
//

function V2 perp(V2 v) {
    V2 result = v2(-v.y, v.x);
    return result;
}

function V2 perp_clockwise(V2 v) {
    V2 result = v2(v.y, -v.x);
    return result;
}

function V2 square(V2 v) {
    V2 result;
    result.x = v.x*v.x;
    result.y = v.y*v.y;
    return result;
}

function V2 lerp(V2 a, V2 b, f32 t) {
    return a*(1.0f - t) + b*t;
}

function f32 dot(V2 a, V2 b) {
    return a.x*b.x + a.y*b.y;
}

function V2 reflect(V2 a, V2 b) {
    f32 dot2 = 2.0f*dot(a, b);
    V2 result;
    result.x = a.x - dot2*b.x;
    result.y = a.y - dot2*b.y;
    return result;
}

function f32 length_sq(V2 v) {
    f32 result = dot(v, v);
    return result;
}

function f32 length(V2 v) {
    f32 result = square_root(dot(v, v));
    return result;
}

function V2 normalize(V2 v) {
    Assert(length(v) != 0.0f);
    return v / length(v);
}

function V2 noz(V2 v) {
    V2 result = {};
    
    f32 l_sq = length_sq(v);
    if (l_sq > square(0.0001f)) {
        result = v*(1.0f / square_root(l_sq));
    }
    return result;
}

function V2 min(V2 a, V2 b) {
    V2 result;
    result.x = min(a.x, b.x);
    result.y = min(a.y, b.y);
    return result;
}

function V2 min(V2 a, f32 b) {
    V2 result;
    result.x = min(a.x, b);
    result.y = min(a.y, b);
    return result;
}

function V2 max(V2 a, V2 b) {
    V2 result;
    result.x = max(a.x, b.x);
    result.y = max(a.y, b.y);
    return result;
}

function V2 max(V2 a, f32 b) {
    V2 result;
    result.x = max(a.x, b);
    result.y = max(a.y, b);
    return result;
}

function V2 clamp(V2 n, V2 lo, V2 hi) {
    V2 result;
    result.x = max(lo.x, min(hi.x, n.x));
    result.y = max(lo.y, min(hi.y, n.y));
    return result;
}

function V2 clamp(V2 n, f32 lo, f32 hi) {
    V2 result;
    result.x = max(lo, min(hi, n.x));
    result.y = max(lo, min(hi, n.y));
    return result;
}

function V2 arm(f32 angle) {
    V2 result;
    result.x = cos(angle);
    result.y = sin(angle);
    return result;
}

function V2 rotate_arm(V2 v, V2 cos_sin) {
    V2 result;
    result.x = (v.x*cos_sin.x - v.y*cos_sin.y);
    result.y = (v.x*cos_sin.y + v.y*cos_sin.x);
    return result;
}

function V2 rotate(V2 v, f32 r) {
    V2 result = rotate_arm(v, arm(r));
    return result;
}

function V2 rotate_arm_clockwise(V2 v, V2 cos_sin) {
    V2 result;
    result.x = (v.x*cos_sin.x + v.y*cos_sin.y);
    result.y = (v.y*cos_sin.x - v.x*cos_sin.y);
    return result;
}

function V2 rotate_clockwise(V2 v, f32 r) {
    V2 result = rotate_arm_clockwise(v, arm(r));
    return result;
}

//
// NOTE: V2i
//

function u32 length_sq(V2i x) {
    u32 result = x.x*x.x + x.y*x.y;
    return result;
}

function f32 length_f32(V2i x) {
    f32 result = square_root((f32)length_sq(x));
    return result;
}

function u32 length(V2i x) {
    u32 result = round_f32_to_u32(length_f32(x));
    return result;
}

function s32 manhattan_distance(V2i a, V2i b) {
    s32 result = abs(a.x - b.x) + abs(a.y - b.y);
    return result;
}

function f32 diagonal_distance(V2i a, V2i b, f32 diagonal_cost) {
    s32 dx = abs(a.x - b.x);
    s32 dy = abs(a.y - b.y);
    f32 result = (f32)(dx + dy) + (diagonal_cost - 2.0f)*(f32)min(dx, dy);
    return result;
}

//
// NOTE: V3i
//

function V3i floor_v3_to_v3i(V3 a) {
    V3i result = v3i(floor_f32_to_s32(a.x),
                     floor_f32_to_s32(a.y),
                     floor_f32_to_s32(a.z));
    return result;
}

function V3i round_v3_to_v3i(V3 a) {
    V3i result = v3i(round_f32_to_s32(a.x),
                     round_f32_to_s32(a.y),
                     round_f32_to_s32(a.z));
    return result;
}

function u32 length_sq(V3i x) {
    u32 result = (x.x*x.x) + (x.y*x.y) + (x.z*x.z);
    return result;
}

function f32 length_f32(V3i x) {
    f32 result = square_root((f32)length_sq(x));
    return result;
}

function u32 length(V3i x) {
    u32 result = round_f32_to_u32(length_f32(x));
    return result;
}

function f32 distance_f32(V3i a, V3i b) {
    f32 result = length_f32(a - b);
    return result;
}

function s32 distance(V3i a, V3i b) {
    s32 result = round_f32_to_s32(distance_f32(a, b));
    return result;
}

function s32 manhattan_distance(V3i a, V3i b) {
    s32 result = abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z);
    return result;
}

function f32 diagonal_distance(V3i a, V3i b, f32 diagonal_cost) {
    s32 dx = abs(a.x - b.x);
    s32 dy = abs(a.y - b.y);
    s32 dz = abs(a.z - b.z);
    f32 result = (f32)(dx + dy + dz) + (diagonal_cost - 2.0f)*(f32)min(min(dx, dy), dz);
    return result;
}

function V3i min(V3i a, V3i b) {
    V3i result;
    result.x = min(a.x, b.x);
    result.y = min(a.y, b.y);
    result.z = min(a.z, b.z);
    return result;
}

function V3i max(V3i a, V3i b) {
    V3i result;
    result.x = max((f32)a.x, (f32)b.x);
    result.y = max((f32)a.y, (f32)b.y);
    result.z = max((f32)a.z, (f32)b.z);
    return result;
}

//
// NOTE: Rect2
//

/*
 I'm saying rects start bottom left, wound counter clockwise, regarding the naming of corners.
    D --------- C
    |           |
    |           |
    A --------- B
*/

function V2 get_dim(Rect2 rect) {
    V2 result = rect.max - rect.min;
    return result;
}

function V2 get_min_corner(Rect2 rect) {
    V2 result = rect.min;
    return result;
}

function V2 get_max_corner(Rect2 rect) {
    V2 result = rect.max;
    return result;
}

function V2 get_center(Rect2 rect) {
    V2 result = 0.5f * (rect.min + rect.max);
    return result;
}

function f32 get_aspect_ratio(Rect2 rect) {
    V2 dim = get_dim(rect);
    f32 aspect_ratio = dim.x/dim.y;
    return aspect_ratio;
}

function Rect2 rect_min_max(V2 min, V2 max) {
    Rect2 result;
    result.min = min;
    result.max = max;
    return result;
}

function Rect2 rect_center_half_dim(V2 center, V2 half_dim) {
    Rect2 result;
    result.min = center - half_dim;
    result.max = center + half_dim;
    return result;
}

function Rect2 rect_center_dim(V2 center, V2 dim) {
    Rect2 result;
    result.min = center - dim * 0.5f;
    result.max = center + dim * 0.5f;
    return result;
}

function Rect2 rect_min_dim(V2 min, V2 dim) {
    Rect2 result;
    result.min = min;
    result.max = min + dim;
    return result;
}

function Rect2 multiply_dimensions(Rect2 rect, V2 mul) {
    Rect2 result;
    result.min = rect.min * mul;
    result.max = rect.max * mul;
    return result;
}

function Rect2 grow_by_radius(Rect2 rect, V2 r_dim) {
    Rect2 result;
    result.min = rect.min - r_dim;
    result.max = rect.max + r_dim;
    return result;
}

function Rect2 grow_by_diameter(Rect2 rect, V2 d_dim) {
    Rect2 result;
    result.min = rect.min - d_dim * 0.5f;
    result.max = rect.max + d_dim * 0.5f;
    return result;
}

function Rect2 offset(Rect2 a, V2 offset) {
    Rect2 result;
    result.min = a.min + offset;
    result.max = a.max + offset;
    return result;
}

function b32 is_in_rect(Rect2 rect, V2 test) {
    b32 result = ((test.x >= rect.min.x && test.x < rect.max.x) &&
                  (test.y >= rect.min.y && test.y < rect.max.y));
    return result;
}

function b32 rect_contained_in_rect(Rect2 outer, Rect2 inner) {
    b32 result = ((outer.min.x <= inner.min.x && outer.max.x >= inner.max.x) &&
                  (outer.min.y <= inner.min.y && outer.max.y >= inner.max.y));
    return result;
}

function b32 rect_intersect(Rect2 a, Rect2 b) {
    b32 result = !(b.max.x <= a.min.x ||
                   b.min.x >= a.max.x ||
                   b.max.y <= a.min.y ||
                   b.min.y >= a.max.y);
    return result;
}

function Rect2 rect_sum(Rect2 a, Rect2 b) {
    Rect2 result;
    result.min = a.min + b.min;
    result.max = a.max + b.max;
    return result;
}

function V2 get_barycentric(Rect2 rect, V2 p) {
    V2 result;
    result.x = safe_ratio_0(p.x - rect.min.x, rect.max.x - rect.min.x);
    result.y = safe_ratio_0(p.y - rect.min.y, rect.max.y - rect.min.y);
    return result;
}

function Rect2 rect_union(Rect2 a, Rect2 b) {
    Rect2 result;
    result.min.x = min(a.min.x, b.min.x);
    result.min.y = min(a.min.y, b.min.y);
    result.max.x = max(a.max.x, b.max.x);
    result.max.y = max(a.max.y, b.max.y);
    return result;
}

function Rect2 intersect(Rect2 a, Rect2 b) {
    Rect2 result;
    result.min.x = max(a.min.x, b.min.x);
    result.min.y = max(a.min.y, b.min.y);
    result.max.x = min(a.max.x, b.max.x);
    result.max.y = min(a.max.y, b.max.y);
    return result;
}

function Rect2i min_width(Rect2i rect, s32 target_width) {
    Rect2i result = rect;
    s32 cur_width = (result.max.x - result.min.x);
    if (cur_width < target_width) {
        result.max.x += (target_width - cur_width);
    }
    return result;
}

function Rect2 grow_to_contain(Rect2 rect, V2 p) {
    Rect2 result = rect;
    result.min = min(result.min, p);
    result.max = max(result.max, p);
    return result;
}

function Rect2 inverted_infinity_rect2(void) {
    Rect2 result;
    result.min.x = result.min.y =  F32_MAX;
    result.max.x = result.max.y = -F32_MAX;
    return result;
}

function Rect2 correct_rect_winding(Rect2 rect) {
    Rect2 result = rect;
    if (rect.min.x > rect.max.x) {
        result.min.x = rect.max.x;
        result.max.x = rect.min.x;
    }
    if (rect.min.y > rect.max.y) {
        result.min.y = rect.max.y;
        result.max.y = rect.min.y;
    }
    return result;
}

function f32 get_area(Rect2 box) {
    V2 dim = get_dim(box);
    return dim.x*dim.y;
}

//
// NOTE: Rect2i
//

function Rect2i rect_min_max(V2i min, V2i max) {
    Rect2i result;
    result.min = min;
    result.max = max;
    return result;
}

function Rect2i rect_min_dim(V2i min, V2i dim) {
    Rect2i result;
    result.min = min;
    result.max = result.min + dim;
    return result;
}

function Rect2i rect_offset(Rect2i rect, V2i offset) {
    Rect2i result = rect;
    result.min += offset;
    result.max += offset;
    return result;
}

function Rect2i rect_union(Rect2i a, Rect2i b) {
    Rect2i result;
    result.min.x = Min(a.min.x, b.min.x);
    result.min.y = Min(a.min.y, b.min.y);
    result.max.x = Max(a.max.x, b.max.x);
    result.max.y = Max(a.max.y, b.max.y);
    return result;
}

function Rect2i rect_intersect(Rect2i a, Rect2i b) {
    Rect2i result;
    result.min.x = Max(a.min.x, b.min.x);
    result.min.y = Max(a.min.y, b.min.y);
    result.max.x = Min(a.max.x, b.max.x);
    result.max.y = Min(a.max.y, b.max.y);
    return result;
}

function Rect2i rect_grow_to_contain(Rect2i rect, V2i p) {
    Rect2i result = rect;
    result.min.x = Min(result.min.x, p.x);
    result.max.x = Max(result.max.x, p.x);
    result.min.y = Min(result.min.y, p.y);
    result.max.y = Max(result.max.y, p.y);
    return result;
}

function s32 rect_width(Rect2i rect) {
    s32 result = rect.max.x - rect.min.x;
    return result;
}

function s32 rect_height(Rect2i rect) {
    s32 result = rect.max.y - rect.min.y;
    return result;
}

function V2i get_dim(Rect2i rect) {
    V2i result;
    result.x = rect.max.x - rect.min.x;
    result.y = rect.max.y - rect.min.y;
    return result;
}

function Rect2i rect_grow_by_radius(Rect2i rect, V2i r) {
    Rect2i result;
    result.min = rect.min - r;
    result.max = rect.max + r;
    return result;
}

function s32 rect_get_area(Rect2i rect) {
    s32 w = rect.max.x - rect.min.x;
    s32 h = rect.max.y - rect.min.y;
    s32 area = 0;
    if ((w > 0) && (h > 0)) {
        area = w*h;
    }
    return area;
}

function Rect2i infinity_rect2i(void) {
    Rect2i result;
    result.max.x = result.max.y =  INT32_MAX;
    result.min.x = result.min.y = -INT32_MAX;
    return result;
}

function Rect2i inverted_infinity_rect2i(void) {
    Rect2i result;
    result.min.x = result.min.y =  INT32_MAX;
    result.max.x = result.max.y = -INT32_MAX;
    return result;
}

function b32 is_in_rect(Rect2i rect, V2i p) {
    b32 result = ((p.x >= rect.min.x && p.x < rect.max.x) &&
                  (p.y >= rect.min.y && p.y < rect.max.y));
    return result;
}

//
// NOTE: Rect3i
//

function Rect3i rect_union(Rect3i a, Rect3i b) {
    Rect3i result;
    result.min = min(a.min, b.min);
    result.max = max(a.max, b.max);
    return result;
}

function Rect3i rect_intersect(Rect3i a, Rect3i b) {
    Rect3i result;
    result.min = max(a.min, b.min);
    result.max = min(a.max, b.max);
    return result;
}

function
Rect3i rect_min_radius(V3i origin, V3i radius) {
    Rect3i result;
    result.min = origin - radius;
    result.max = origin + radius;
    return result;
}

function Rect3i grow_to_contain(Rect3i a, V3i p) {
    Rect3i result = a;
    result.min = min(a.min, p);
    result.max = max(a.max, p);
    return result;
}

function s32 width(Rect3i a) {
    s32 result = a.max.x - a.min.x;
    return result;
}

function s32 height(Rect3i a) {
    s32 result = a.max.y - a.min.y;
    return result;
}

function s32 depth(Rect3i a) {
    s32 result = a.max.z - a.min.z;
    return result;
}

function V3i dim(Rect3i a) {
    V3i result;
    result.x = width(a);
    result.y = height(a);
    result.z = depth(a);
    return result;
}

function s32 volume(Rect3i a) {
    s32 w = a.max.x - a.min.x;
    s32 h = a.max.y - a.min.y;
    s32 d = a.max.z - a.min.z;
    s32 area = 0;
    if ((w > 0) && (h > 0) && (d > 0)) {
        area = w*h*d;
    }
    return area;
}

function Rect3i infinity_rect3i(void) {
    Rect3i result;
    result.min.x = result.min.y = result.min.z = -INT32_MAX;
    result.max.x = result.max.y = result.max.z =  INT32_MAX;
    return result;
}

function Rect3i inverted_infinity_rect3i(void) {
    Rect3i result;
    result.min.x = result.min.y = result.min.z =  INT32_MAX;
    result.max.x = result.max.y = result.max.z = -INT32_MAX;
    return result;
}

function b32 is_in_rect(Rect3i rect, V3i p) {
    b32 result = ((p.x >= rect.min.x && p.x < rect.max.x) &&
                  (p.y >= rect.min.y && p.y < rect.max.y) &&
                  (p.z >= rect.min.z && p.z < rect.max.z));
    return result;
}

//
// NOTE: V3
//

function V3 square(V3 v) {
    V3 result;
    result.x = v.x*v.x;
    result.y = v.y*v.y;
    result.z = v.z*v.z;
    return result;
}

function V3 lerp(V3 a, V3 b, f32 t) {
    f32 inv_t = 1.0f - t;
    V3 result;
    result.x = inv_t*a.x + t*b.x;
    result.y = inv_t*a.y + t*b.y;
    result.z = inv_t*a.z + t*b.z;
    return result;
}

function f32 dot(V3 a, V3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

function V3 cross(V3 a, V3 b) {
    return v3(a.y*b.z - a.z*b.y,
              a.z*b.x - a.x*b.z,
              a.x*b.y - a.y*b.x);
}

function V3 reflect(V3 a, V3 b) {
    f32 dot2 = 2.0f*dot(a, b);
    V3 result;
    result.x = a.x - dot2*b.x;
    result.y = a.y - dot2*b.y;
    result.z = a.z - dot2*b.z;
    return result;
}

function f32 length_sq(V3 v) {
    f32 result = dot(v, v);
    return result;
}

function f32 length(V3 v) {
    f32 result = square_root(dot(v, v));
    return result;
}

function V3 normalize(V3 v) {
    V3 result = v / length(v);
    return result;
}

function V3 noz(V3 v) {
    V3 result = {};
    
    f32 l_sq = length_sq(v);
    if (l_sq > square(0.0001f)) {
        result = v*(1.0f / square_root(l_sq));
    }
    return result;
}

function V3 min(V3 a, V3 b) {
    V3 result;
    result.x = min(a.x, b.x);
    result.y = min(a.y, b.y);
    result.z = min(a.z, b.z);
    return result;
}

function V3 min(V3 a, f32 b) {
    V3 result;
    result.x = min(a.x, b);
    result.y = min(a.y, b);
    result.z = min(a.z, b);
    return result;
}

function V3 max(V3 a, V3 b) {
    V3 result;
    result.x = max(a.x, b.x);
    result.y = max(a.y, b.y);
    result.z = max(a.z, b.z);
    return result;
}

function V3 max(V3 a, f32 b) {
    V3 result;
    result.x = max(a.x, b);
    result.y = max(a.y, b);
    result.z = max(a.z, b);
    return result;
}

function V3 clamp(V3 n, V3 lo, V3 hi) {
    V3 result;
    result.x = max(lo.x, min(hi.x, n.x));
    result.y = max(lo.y, min(hi.y, n.y));
    result.z = max(lo.z, min(hi.z, n.z));
    return result;
}

function V3 clamp(V3 n, f32 lo, f32 hi) {
    V3 result;
    result.x = max(lo, min(hi, n.x));
    result.y = max(lo, min(hi, n.y));
    result.z = max(lo, min(hi, n.z));
    return result;
}

//
// NOTE: Rect3
//

function V3 rect_dim(Rect3 rect) {
    V3 result = rect.max - rect.min;
    return result;
}

function V3 rect_center(Rect3 rect) {
    V3 result = 0.5f*(rect.min + rect.max);
    return result;
}

function Rect3 rect_min_max(V3 min, V3 max) {
    Rect3 result;
    result.min = min;
    result.max = max;
    return result;
}

function Rect3 rect_center_half_dim(V3 center, V3 half_dim) {
    Rect3 result;
    result.min = center - half_dim;
    result.max = center + half_dim;
    return result;
}

function Rect3 rect_center_dim(V3 center, V3 dim) {
    Rect3 result;
    result.min = center + 0.5f*dim;
    result.max = center + 0.5f*dim;
    return result;
}

function Rect3 rect_min_dim(V3 min, V3 dim) {
    Rect3 result;
    result.min = min;
    result.max = min*dim;
    return result;
}

function Rect3 rect_mul_dim(Rect3 rect, V3 dim) {
    Rect3 result;
    result.min = rect.min*dim;
    result.max = rect.max*dim;
    return result;
}

function Rect3 rect_grow_by_radius(Rect3 rect, V3 r_dim) {
    Rect3 result;
    result.min = rect.min - r_dim;
    result.max = rect.max + r_dim;
    return result;
}

function Rect3 rect_grow_by_diameter(Rect3 rect, V3 d_dim) {
    Rect3 result;
    result.min = rect.min - 0.5f*d_dim;
    result.max = rect.max + 0.5f*d_dim;
    return result;
}

function Rect3 rect_offset(Rect3 a, V3 offset) {
    Rect3 result;
    result.min = a.min + offset;
    result.max = a.max + offset;
    return result;
}

function b32 is_in_rect(Rect3 rect, V3 test) {
    b32 result = ((test.x >= rect.min.x && test.x < rect.max.x) &&
                  (test.y >= rect.min.y && test.y < rect.max.y) &&
                  (test.z >= rect.min.z && test.z < rect.max.z));
    return result;
}

function b32 rect_contained_in_rect(Rect3 outer, Rect3 inner) {
    b32 result = ((outer.min.x >= inner.min.x && outer.max.x <= inner.max.x) &&
                  (outer.min.y >= inner.min.y && outer.max.y <= inner.max.y) &&
                  (outer.min.z >= inner.min.z && outer.max.z <= inner.max.z));
    return result;
}

function b32 rect_intersect(Rect3 a, Rect3 b) {
    b32 result = !(b.max.x <= a.min.x ||
                   b.min.x >= a.max.x ||
                   b.max.y <= a.min.y ||
                   b.min.y >= a.max.y ||
                   b.max.z <= a.min.z ||
                   b.min.z >= a.max.z);
    return result;
}

function V3 rect_get_barycentric(Rect3 rect, V3 p) {
    V3 result;
    result.x = safe_ratio_0(p.x - rect.min.x, rect.max.x - rect.min.x);
    result.y = safe_ratio_0(p.y - rect.min.y, rect.max.y - rect.min.y);
    result.z = safe_ratio_0(p.z - rect.min.z, rect.max.z - rect.min.z);
    return result;
}

function Rect2 rect3_to_rect2_xy(Rect3 rect) {
    Rect2 result;
    result.min = rect.min.xy;
    result.max = rect.max.xy;
    return result;
}

//
// NOTE: V4
//

function V4 neg(V4 a) {
    V4 result;
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    result.w = -a.w;
    return result;
}

function V4 square(V4 v) {
    V4 result;
    result.x = v.x*v.x;
    result.y = v.y*v.y;
    result.z = v.z*v.z;
    result.w = v.w*v.w;
    return result;
}

function V4 lerp(V4 a, V4 b, f32 t) {
    f32 inv_t = 1.0f - t;
    V4 result;
    result.x = inv_t*a.x + t*b.x;
    result.y = inv_t*a.y + t*b.y;
    result.z = inv_t*a.z + t*b.z;
    result.w = inv_t*a.w + t*b.w;
    return result;
}

function f32 dot(V4 a, V4 b) {
    f32 result = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
    return result;
}

function V4 reflect(V4 a, V4 b) {
    f32 dot2 = 2.0f*dot(a, b);
    V4 result;
    result.x = a.x - dot2*b.x;
    result.y = a.y - dot2*b.y;
    result.z = a.z - dot2*b.z;
    result.w = a.w - dot2*b.w;
    return result;
}

function f32 length_sq(V4 v) {
    f32 result = dot(v, v);
    return result;
}

function f32 length(V4 v) {
    f32 result = square_root(dot(v, v));
    return result;
}

function V4 min(V4 a, V4 b) {
    V4 result;
    result.x = min(a.x, b.x);
    result.y = min(a.y, b.y);
    result.z = min(a.z, b.z);
    result.w = min(a.w, b.w);
    return result;
}

function V4 min(V4 a, f32 b) {
    V4 result;
    result.x = min(a.x, b);
    result.y = min(a.y, b);
    result.z = min(a.z, b);
    result.w = min(a.w, b);
    return result;
}

function V4 max(V4 a, V4 b) {
    V4 result;
    result.x = max(a.x, b.x);
    result.y = max(a.y, b.y);
    result.z = max(a.z, b.z);
    result.w = max(a.w, b.w);
    return result;
}

function V4 max(V4 a, f32 b) {
    V4 result;
    result.x = max(a.x, b);
    result.y = max(a.y, b);
    result.z = max(a.z, b);
    result.w = max(a.w, b);
    return result;
}

function V4 clamp(V4 n, V4 lo, V4 hi) {
    V4 result;
    result.x = max(lo.x, min(hi.x, n.x));
    result.y = max(lo.y, min(hi.y, n.y));
    result.z = max(lo.z, min(hi.z, n.z));
    result.w = max(lo.w, min(hi.w, n.w));
    return result;
}

function V4 clamp(V4 n, f32 lo, f32 hi) {
    V4 result;
    result.x = max(lo, min(hi, n.x));
    result.y = max(lo, min(hi, n.y));
    result.z = max(lo, min(hi, n.z));
    result.w = max(lo, min(hi, n.w));
    return result;
}

//
// NOTE: M4x4
//

function M4x4 m4x4_mul(M4x4 a, M4x4 b) {
    M4x4 result;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            for (int i = 0; i < 4; ++i) {
                result.e[r][c] += a.e[r][i]*b.e[i][c];
            }
        }
    }
    return result;
}

function V4 m4x4_transform_v4(M4x4 m, V4 p) {
    V4 result;
    result.x = p.x*m.e[0][0] + p.y*m.e[0][1] + p.z*m.e[0][2] + p.w*m.e[0][3];
    result.y = p.x*m.e[1][0] + p.y*m.e[1][1] + p.z*m.e[1][2] + p.w*m.e[1][3];
    result.z = p.x*m.e[2][0] + p.y*m.e[2][1] + p.z*m.e[2][2] + p.w*m.e[2][3];
    result.w = p.x*m.e[3][0] + p.y*m.e[3][1] + p.z*m.e[3][2] + p.w*m.e[3][3];
    return result;
}

function M4x4 m4x4_identity(void) {
    M4x4 result = {
        {
            { 1, 0, 0, 0, },
            { 0, 1, 0, 0, },
            { 0, 0, 1, 0, },
            { 0, 0, 0, 1, },
        }
    };
    return result;
}

function M4x4 m4x4_x_rotation(f32 angle) {
    f32 c = cosf(angle);
    f32 s = sinf(angle);
    M4x4 result = {
        {
            { 1, 0, 0, 0, },
            { 0, c,-s, 0, },
            { 0, s, c, 0, },
            { 0, 0, 0, 1, },
        }
    };
    return result;
}

function M4x4 m4x4_y_rotation(f32 angle) {
    f32 c = cosf(angle);
    f32 s = sinf(angle);
    M4x4 result = {
        {
            { c, 0, s, 0, },
            { 0, 1, 0, 0, },
            {-s, 0, c, 0, },
            { 0, 0, 0, 1, },
        }
    };
    return result;
}

function M4x4 m4x4_z_rotation(f32 angle) {
    f32 c = cosf(angle);
    f32 s = sinf(angle);
    M4x4 result = {
        {
            { c,-s, 0, 0, },
            { s, c, 0, 0, },
            { 0, 0, 1, 0, },
            { 0, 0, 0, 1, },
        }
    };
    return result;
}

#endif
