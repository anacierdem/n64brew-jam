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

// Given vertices should be in correct order
static void render_tri(RdpDisplayList* rdl, int32_t v1x, int32_t v1y, int32_t v2x, int32_t v2y, int32_t v3x, int32_t v3y) {
    int32_t YL = v3y,
            YM = v2y,
            YH = v1y,
            XL = v2x,
            XH = v1x,
            XM = v1x,
            DxLDy = (v3y - v2y) == 0 ? 0 : div_16d16(v3x - v2x, v3y - v2y),
            DxHDy = (v3y - v1y) == 0 ? 0 : div_16d16(v3x - v1x, v3y - v1y),
            DxMDy = (v2y - v1y) == 0 ? 0 : div_16d16(v2x - v1x, v2y - v1y);

    #ifdef MICRO_ADJUSTMENTS
        int32_t subpixel_height = make_16d16(0, 0.25);
        XL += mult_16d16(DxLDy, subpixel_height - (v2y && subpixel_height) );
        XH -= mult_16d16(DxHDy, frac_16d16(v1y));
        XM -= mult_16d16(DxMDy, frac_16d16(v1y));
    #endif

    rdl_push(rdl, (cast64(0xC8) << 56) | (cast64(1) << 55) |
        (cast64(from_16d16_to_11d2(YL)) << 32) |
        (cast64(from_16d16_to_11d2(YM)) << 16) |
        (cast64(from_16d16_to_11d2(YH)) << 0)
    );
    rdl_push(rdl, (cast64(XL) << 32) | (cast64(DxLDy) << 0) );
    rdl_push(rdl, (cast64(XH) << 32) | (cast64(DxHDy) << 0) );
    rdl_push(rdl, (cast64(XM) << 32) | (cast64(DxMDy) << 0) );
}


#define SWAP(X, Y) ({ int32_t t = X; X = Y; Y = t; })

static int32_t last_DxDy, last_v1x, last_v1y, last_v2x, last_v2y;
static int32_t is_even = 1;
// Start from top left
static void render_tri_strip(RdpDisplayList* rdl, int32_t v1x, int32_t v1y, int32_t v2x, int32_t v2y, int32_t v3x, int32_t v3y) {
    int32_t YL = v3y,
            YM = v2y,
            YH = v1y,
            XL = (v3y < v2y) ? v3x : v2x,
            XH = v1x,
            XM = v1x,
            DxLDy = (v3y - v2y) == 0 ? 0 : div_16d16(v3x - v2x, v3y - v2y),
            DxHDy = (v3y - v1y) == 0 ? 0 : div_16d16(v3x - v1x, v3y - v1y),
            DxMDy = (v2y - v1y) == 0 ? 0 : div_16d16(v2x - v1x, v2y - v1y);

    last_DxDy = DxHDy;
    last_v1x = v1x;
    last_v1y = v1y;
    last_v2x = v3x;
    last_v2y = v3y;
    is_even = 0;

    if (v3y < v2y) {
        SWAP(YL, YM);
        SWAP(DxHDy, DxMDy);
    }

    #ifdef MICRO_ADJUSTMENTS
        int32_t subpixel_height = make_16d16(0, 0.25);
        XL += mult_16d16(DxLDy, subpixel_height - (v2y && subpixel_height) );
        XH -= mult_16d16(DxHDy, frac_16d16(v1y));
        XM -= mult_16d16(DxMDy, frac_16d16(v1y));
    #endif

    rdl_push(rdl, (cast64(0xC8) << 56) | (cast64((v3y < v2y) ? 1 : 0) << 55) |
        (cast64(from_16d16_to_11d2(YL)) << 32) |
        (cast64(from_16d16_to_11d2(YM)) << 16) |
        (cast64(from_16d16_to_11d2(YH)) << 0)
    );
    rdl_push(rdl, (cast64(XL) << 32) | (cast64(DxLDy) << 0) );
    rdl_push(rdl, (cast64(XH) << 32) | (cast64(DxHDy) << 0) );
    rdl_push(rdl, (cast64(XM) << 32) | (cast64(DxMDy) << 0) );
}


static void render_tri_strip_next(RdpDisplayList* rdl, int32_t v1x, int32_t v1y) {
    int32_t YL = v1y,
            YM = last_v2y,
            YH = last_v1y,
            XL = (v1y < last_v2y) ? v1x : last_v2x,
            XH = last_v1x,
            XM = last_v1x,
            DxLDy = (v1y - last_v2y) == 0 ? 0 : div_16d16(v1x - last_v2x, v1y - last_v2y),
            DxHDy = (v1y - last_v1y) == 0 ? 0 : div_16d16(v1x - last_v1x, v1y - last_v1y),
            DxMDy = last_DxDy;

    last_DxDy = DxHDy;
    is_even = is_even ^ 1;

    if (is_even) {
        last_v1x = v1x;
        last_v1y = v1y;
    } else {
        last_v2x = v1x;
        last_v2y = v1y;
    }

    if (v1y < last_v2y) {
        SWAP(YL, YM);
        SWAP(DxHDy, DxMDy);
    }

    rdl_push(rdl, (cast64(0xC8) << 56) | (cast64((v1y < last_v2y) ? 1 : 0) << 55) |
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
            make_16d16(50),   make_16d16(0),
            make_16d16(50),   make_16d16(50),
            make_16d16(150),  make_16d16(50)
        );
        render_tri_strip_next(rdl, make_16d16(150), make_16d16(0));
    
        render_tri_strip_next(rdl, make_16d16(250), make_16d16(50));
        render_tri_strip_next(rdl, make_16d16(250), make_16d16(0));
    
        render_tri_strip_next(rdl, make_16d16(300), make_16d16(50));
        render_tri_strip_next(rdl, make_16d16(300), make_16d16(0));
    
        // render_tri_strip_next(rdl, make_16d16(350), make_16d16(0));


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