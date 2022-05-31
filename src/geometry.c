#include <libdragon.h>

#include "./geometry.h"

static inline void __render_tri(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, int32_t DxDy3_1, int32_t DxDy2_1, int32_t DxDy3_2) {
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
            XM = x1,
            DxHDy = DxDy3_1,
            DxMDy = DxDy2_1,
            DxLDy = DxDy3_2;

    assert(YH <= YM);
    assert(YM <= YL);

    // TODO: can reuse as well for some cases of tri strip
    XL += mult_16d16(DxLDy, ceil_16d16(4 * y2) / 4 - y2 );
    XM -= mult_16d16(DxMDy, y1 - floor_16d16(y1));
    XH -= mult_16d16(DxHDy, y1 - floor_16d16(y1));

    uint8_t dir = (
        (int64_t)(x3 - x1) * (int64_t)(y2 - y1) -
        (int64_t)(y3 - y1) * (int64_t)(x2 - x1)
    ) > 0 ? 0 : 1;

    rdpq_fill_triangle(dir,
        from_16d16_to_11d2(YL),
        from_16d16_to_11d2(YM),
        from_16d16_to_11d2(YH),
        XL, DxLDy,
        XH, DxHDy,
        XM, DxMDy);

    rdpq_fill_triangle(dir,
        from_16d16_to_11d2(YL),
        from_16d16_to_11d2(YM),
        from_16d16_to_11d2(YH),
        XL, DxLDy,
        XH, DxHDy,
        XM, DxMDy);

    rdpq_fill_triangle(dir,
        from_16d16_to_11d2(YL),
        from_16d16_to_11d2(YM),
        from_16d16_to_11d2(YH),
        XL, DxLDy,
        XH, DxHDy,
        XM, DxMDy);

    // rdpq_fill_triangle(1,
    //     100 << 2,
    //     0,
    //     0,
    //     100 << 2, -4,
    //     0, 0,
    //     0, 0);

    rspq_wait();
    rsp_crash();

}


void render_tri(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3) {
    int32_t DxDy3_1 = (y3 - y1) == 0 ? 0 : div_16d16(x3 - x1, y3 - y1),
            DxDy2_1 = (y2 - y1) == 0 ? 0 : div_16d16(x2 - x1, y2 - y1),
            DxDy3_2 = (y3 - y2) == 0 ? 0 : div_16d16(x3 - x2, y3 - y2);

    __render_tri(x1, y1, x2, y2, x3, y3, DxDy3_1, DxDy2_1, DxDy3_2);
}

static int32_t last_DxDy, last_x1, last_y1, last_x2, last_y2;

void render_tri_strip(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3) {
    int32_t DxDy3_1 = (y3 - y1) == 0 ? 0 : div_16d16(x3 - x1, y3 - y1),
            DxDy2_1 = (y2 - y1) == 0 ? 0 : div_16d16(x2 - x1, y2 - y1),
            DxDy3_2 = (y3 - y2) == 0 ? 0 : div_16d16(x3 - x2, y3 - y2);

    last_DxDy = DxDy3_2;
    last_x1 = x2;
    last_y1 = y2;
    last_x2 = x3;
    last_y2 = y3;

    __render_tri(x1, y1, x2, y2, x3, y3, DxDy3_1, DxDy2_1, DxDy3_2);
}

void render_tri_strip_next(int32_t x3, int32_t y3) {
    int32_t x1 = last_x1,
            y1 = last_y1,
            x2 = last_x2,
            y2 = last_y2,
            DxDy3_1 = (y3 - y1) == 0 ? 0 : div_16d16(x3 - x1, y3 - y1),
            DxDy2_1 = last_DxDy,
            DxDy3_2 = (y3 - y2) == 0 ? 0 : div_16d16(x3 - x2, y3 - y2);

    last_DxDy = DxDy3_2;
    last_x1 = x2;
    last_y1 = y2;
    last_x2 = x3;
    last_y2 = y3;

    __render_tri(x1, y1, x2, y2, x3, y3, DxDy3_1, DxDy2_1, DxDy3_2);
}
