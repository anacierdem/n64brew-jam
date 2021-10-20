#include <stdio.h>

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

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

// #define MICRO_ADJUSTMENTS
#define SWAP(X, Y) ({ int32_t t = X; X = Y; Y = t; })

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
// static int32_t is_even = 1;

static void render_tri_strip(RdpDisplayList* rdl, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3) {
    int32_t DxDy3_2 = (y3 - y2) == 0 ? 0 : div_16d16(x3 - x2, y3 - y2),
            DxDy3_1 = (y3 - y1) == 0 ? 0 : div_16d16(x3 - x1, y3 - y1),
            DxDy2_1 = (y2 - y1) == 0 ? 0 : div_16d16(x2 - x1, y2 - y1);

    last_DxDy = DxDy3_2;
    last_x1 = x2;
    last_y1 = y2;
    last_x2 = x3;
    last_y2 = y3;
    // is_even = 0;

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
    // is_even = 0;

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


int main(void)
{
    // effective height: 240 * 4 = 960
    // int32_t height = 240 * 4;
    // effective width: 640 * 65535 = 41942400
    // int32_t width = 640 * 65535;

    display_init(RESOLUTION_640x240, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE);

    debug_init_usblog();

    debugf("Hello world!\n");

    rdp_init();



    static display_context_t disp = 0;

    int32_t left = 0;

    RdpDisplayList *rdl = rdl_heap_alloc(100);

    test_func();

    while(1) {

        while( !(disp = display_lock()) );

        rdp_attach_display( disp );

        rdl_reset(rdl);

        rdl_push(rdl,RdpSyncPipe());
        rdl_push(rdl,RdpSetClippingI(0, 0, 640, 240));
        rdl_push(rdl,RdpSetOtherModes(SOM_CYCLE_FILL));

        rdl_push(rdl,RdpSetFillColor(RDP_COLOR16(31,31,31,1)));
        rdl_push(rdl,RdpFillRectangleI(0, 0, 640, 240));

        // rdl_push(rdl,RdpSetFillColor(RDP_COLOR16(31,0,0,1)));
        // rdl_push(rdl,RdpFillRectangleI(0, 0, 0 + 100, 0 + 50));

        rdl_push(rdl,RdpSyncPipe());


        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(255, 255, 0, 128)));

        #define BLEND_ENABLE            (1 << 14)
        #define READ_ENABLE             (1 << 6)
        #define AA_ENABLE               (1 << 3)
        #define COVERAGE_DEST_CLAMP     (0 << 8)
        #define COLOR_ONCOVERAGE        (1 << 7)

        rdl_push(rdl,
            RdpSetCombine(
                // We need to enable same flags for both cycles in 1 cycle mode.
                Comb0_Rgb(ZERO, ZERO, ZERO , PRIM) | Comb0_Alpha(ZERO, ZERO, ZERO , PRIM) |
                Comb1_Rgb(ZERO, ZERO, ZERO , PRIM) | Comb1_Alpha(ZERO, ZERO, ZERO , PRIM)
            )
        );



        // 30, 26, 22, 18 are P, A, M, B cycle 0

        /* Configure (FOG_RGB * FOG_A) + (MEM_RGB * (1-FOG_A)). For some reason,
        there seems to be no way to use the blend register's alpha, so we
        need to use the fog register for that. */
        // #define BLEND_ENABLE   (1<<14)
        // #define BLEND_MODE_FOG ((3<<30) | (1<<26) | (1<<22))


        // (P*A + M*B)
        // in the case above P=3=FOG_RGB, A=1=FOG_A, M=1=MEM_RGB, B=0=1-A
        rdl_push(rdl, RdpSetOtherModes( BLEND_ENABLE | AA_ENABLE | READ_ENABLE |
            (cast64(0x0) << 30) | (cast64(0x3) << 28) | (cast64(0x0) << 26) | (cast64(0x1) << 24) |
            (cast64(0x1) << 22) | (cast64(0x1) << 20) | (cast64(0x0) << 18) | (cast64(0x0) << 16) ) ); // | (cast64(0x80000000))


        render_tri_strip(rdl,
            make_16d16(50),   make_16d16(50),
            make_16d16(50),   make_16d16(0),
            make_16d16(100),  make_16d16(30)
        );
        render_tri_strip_next(rdl, make_16d16(120), make_16d16(10));

        render_tri_strip_next(rdl, make_16d16(180), make_16d16(50));
        render_tri_strip_next(rdl, make_16d16(200), make_16d16(0));

        render_tri_strip_next(rdl, make_16d16(250), make_16d16(45));
        render_tri_strip_next(rdl, make_16d16(300), make_16d16(10));

        // rdl_push(rdl,RdpSetFogColor(cast64(RDP_COLOR32(255,0,255,50))));

        // render_tri(rdl,
        //     make_16d16(50),    make_16d16(0),
        //     make_16d16(250),   make_16d16(10),
        //     make_16d16(150),   make_16d16(25)
        // );


        rdl_flush(rdl);
        rdl_exec(rdl);

        // Present
        rdp_detach_display();
        display_show(disp);
        wait_ms(50);

        left += make_16d16(0, 0.25);

        // while(1) {};
    }
}