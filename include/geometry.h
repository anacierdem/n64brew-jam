#ifndef __GEOMETRY_H
#define __GEOMETRY_H

#include "rdl.h"

#define make_16d16_2(A, frac)       (((A) << 16) + (uint16_t)(((frac) * 65536)))
#define make_16d16_1(A)             (((A) << 16))

#define GET_3TH_ARG(arg1, arg2, arg3, ...) arg3
#define __make_16d16(...) GET_3TH_ARG(__VA_ARGS__, make_16d16_2, make_16d16_1)

#define make_16d16(...) __make_16d16(__VA_ARGS__)(__VA_ARGS__)

#define div_16d16(A, B) (((int64_t)(A) << 16) / (int64_t)(B))
#define mult_16d16(A, B) (((int64_t)(A) * (int64_t)(B)) >> 16)

#define floor_16d16(A) (((A) >> 16) << 16)
#define ceil_16d16(A) (floor_16d16((A) + 0xFFFF))

#define from_16d16_to_11d2(A) ((uint16_t)((A) >> 14))

#define SWAP(X, Y) ({ int32_t t = X; X = Y; Y = t; })

void render_tri(RdpDisplayList* rdl, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3);
void render_tri_strip(RdpDisplayList* rdl, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3);
void render_tri_strip_next(RdpDisplayList* rdl, int32_t x3, int32_t y3);

#endif /* __GEOMETRY_H */