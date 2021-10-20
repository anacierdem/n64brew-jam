#include <libdragon.h>

#include "rdl.h"

#define make_16d16_2(A, frac)       (((A) << 16) | (uint16_t)(((frac) * 65535)))
#define make_16d16_1(A)             (((A) << 16))

#define GET_3TH_ARG(arg1, arg2, arg3, ...) arg3
#define __make_16d16(...) GET_3TH_ARG(__VA_ARGS__, make_16d16_2, make_16d16_1)

#define make_16d16(...) __make_16d16(__VA_ARGS__)(__VA_ARGS__)

#define div_16d16(A, B) (((int64_t)(A) << 16) / (int64_t)(B))
#define mult_16d16(A, B) (((int64_t)(A) * (int64_t)(B)) >> 16)
#define frac_16d16(A) ((A) && 0xFFFF)
#define from_16d16_to_11d2(A) ((int16_t)((A) >> 14))

#define SWAP(X, Y) ({ int32_t t = X; X = Y; Y = t; })

// #define MICRO_ADJUSTMENTS

static void render_tri(RdpDisplayList* rdl, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3) {
    if (y1 > y2) {
        SWAP(y1, y2);
        SWAP(x1, x2);
    }

    if (y2 > y3) {
        SWAP(y2, y3);
        SWAP(x2, x3);
    }

    if (y1 > y2) {
        SWAP(y1, y2);
        SWAP(x1, x2);
    }

    int32_t YL = y3,
            YM = y2,
            YH = y1,
            XL = x2,
            XH = x1,
            XM = x1;

    assert(YH <= YM);
    assert(YM <= YL);

    int32_t DxLDy = (y3 - y2) == 0 ? 0 : div_16d16(x3 - x2, y3 - y2),
            DxHDy = (y3 - y1) == 0 ? 0 : div_16d16(x3 - x1, y3 - y1),
            DxMDy = (y2 - y1) == 0 ? 0 : div_16d16(x2 - x1, y2 - y1);

    #ifdef MICRO_ADJUSTMENTS
        int32_t subpixel_height = make_16d16(0, 0.25);
        XL += mult_16d16(DxLDy, subpixel_height - (y2 && subpixel_height) );
        XH -= mult_16d16(DxHDy, frac_16d16(y1));
        XM -= mult_16d16(DxMDy, frac_16d16(y1));
    #endif

    uint64_t dir = (
        (int64_t)(x3 - x1) * (int64_t)(y2 - y1) -
        (int64_t)(y3 - y1) * (int64_t)(x2 - x1)
    ) > 0 ? 0 : 1;

    rdl_push(rdl, (cast64(0xC8) << 56) | (cast64(dir) << 55) |
        (cast64(from_16d16_to_11d2(YL)) << 32) |
        (cast64(from_16d16_to_11d2(YM)) << 16) |
        (cast64(from_16d16_to_11d2(YH)) << 0)
    );
    rdl_push(rdl, (cast64(XL) << 32) | (cast64(DxLDy) << 0) );
    rdl_push(rdl, (cast64(XH) << 32) | (cast64(DxHDy) << 0) );
    rdl_push(rdl, (cast64(XM) << 32) | (cast64(DxMDy) << 0) );
}

static int32_t last_DxDy, last_x1, last_y1, last_x2, last_y2;

static void render_tri_strip(RdpDisplayList* rdl, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3) {
    int32_t DxDy3_2 = (y3 - y2) == 0 ? 0 : div_16d16(x3 - x2, y3 - y2),
            DxDy3_1 = (y3 - y1) == 0 ? 0 : div_16d16(x3 - x1, y3 - y1),
            DxDy2_1 = (y2 - y1) == 0 ? 0 : div_16d16(x2 - x1, y2 - y1);

    last_DxDy = DxDy3_2;
    last_x1 = x2;
    last_y1 = y2;
    last_x2 = x3;
    last_y2 = y3;

    if (y1 > y2) {
        SWAP(y1, y2);
        SWAP(x1, x2);
        SWAP(DxDy3_2, DxDy3_1);
    }

    if (y2 > y3) {
        SWAP(y2, y3);
        SWAP(x2, x3);
        SWAP(DxDy2_1, DxDy3_1);
    }

    if (y1 > y2) {
        SWAP(y1, y2);
        SWAP(x1, x2);
        SWAP(DxDy3_2, DxDy3_1);
    }

    int32_t YL = y3,
            YM = y2,
            YH = y1,
            XL = x2,
            XH = x1,
            XM = x1;

    assert(YH <= YM);
    assert(YM <= YL);

    int32_t DxLDy = DxDy3_2,
            DxHDy = DxDy3_1,
            DxMDy = DxDy2_1;

    #ifdef MICRO_ADJUSTMENTS
        int32_t subpixel_height = make_16d16(0, 0.25);
        XL += mult_16d16(DxLDy, subpixel_height - (y2 && subpixel_height) );
        XH -= mult_16d16(DxHDy, frac_16d16(y1));
        XM -= mult_16d16(DxMDy, frac_16d16(y1));
    #endif

    uint64_t dir = (
        (int64_t)(x3 - x1) * (int64_t)(y2 - y1) -
        (int64_t)(y3 - y1) * (int64_t)(x2 - x1)
    ) > 0 ? 0 : 1;

    rdl_push(rdl, (cast64(0xC8) << 56) | (cast64(dir) << 55) |
        (cast64(from_16d16_to_11d2(YL)) << 32) |
        (cast64(from_16d16_to_11d2(YM)) << 16) |
        (cast64(from_16d16_to_11d2(YH)) << 0)
    );
    rdl_push(rdl, (cast64(XL) << 32) | (cast64(DxLDy) << 0) );
    rdl_push(rdl, (cast64(XH) << 32) | (cast64(DxHDy) << 0) );
    rdl_push(rdl, (cast64(XM) << 32) | (cast64(DxMDy) << 0) );
}

static void render_tri_strip_next(RdpDisplayList* rdl, int32_t x3, int32_t y3) {
    int32_t x1 = last_x1,
            y1 = last_y1,
            x2 = last_x2,
            y2 = last_y2,
            DxDy3_2 = (y3 - x2) == 0 ? 0 : div_16d16(x3 - x2, y3 - y2),
            DxDy3_1 = (y3 - y1) == 0 ? 0 : div_16d16(x3 - x1, y3 - y1),
            DxDy2_1 = last_DxDy;

    last_DxDy = DxDy3_2;
    last_x1 = x2;
    last_y1 = y2;
    last_x2 = x3;
    last_y2 = y3;

    if (y1 > y2) {
        SWAP(y1, y2);
        SWAP(x1, x2);
        SWAP(DxDy3_2, DxDy3_1);
    }

    if (y2 > y3) {
        SWAP(y2, y3);
        SWAP(x2, x3);
        SWAP(DxDy2_1, DxDy3_1);
    }

    if (y1 > y2) {
        SWAP(y1, y2);
        SWAP(x1, x2);
        SWAP(DxDy3_2, DxDy3_1);
    }

    int32_t YL = y3,
            YM = y2,
            YH = y1,
            XL = x2,
            XH = x1,
            XM = x1;

    assert(YH <= YM);
    assert(YM <= YL);

    int32_t DxLDy = DxDy3_2,
            DxHDy = DxDy3_1,
            DxMDy = DxDy2_1;

    #ifdef MICRO_ADJUSTMENTS
        int32_t subpixel_height = make_16d16(0, 0.25);
        XL += mult_16d16(DxLDy, subpixel_height - (y2 && subpixel_height) );
        XH -= mult_16d16(DxHDy, frac_16d16(y1));
        XM -= mult_16d16(DxMDy, frac_16d16(y1));
    #endif

    uint64_t dir = (
        (int64_t)(x3 - x1) * (int64_t)(y2 - y1) -
        (int64_t)(y3 - y1) * (int64_t)(x2 - x1)
    ) > 0 ? 0 : 1;

    rdl_push(rdl, (cast64(0xC8) << 56) | (cast64(dir) << 55) |
        (cast64(from_16d16_to_11d2(YL)) << 32) |
        (cast64(from_16d16_to_11d2(YM)) << 16) |
        (cast64(from_16d16_to_11d2(YH)) << 0)
    );
    rdl_push(rdl, (cast64(XL) << 32) | (cast64(DxLDy) << 0) );
    rdl_push(rdl, (cast64(XH) << 32) | (cast64(DxHDy) << 0) );
    rdl_push(rdl, (cast64(XM) << 32) | (cast64(DxMDy) << 0) );
}
