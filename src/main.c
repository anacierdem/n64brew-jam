#include <stdio.h>

#include <libdragon.h>

#include "rdl.h"
#include "geometry.h"
#include "b2d.hpp"


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
    timer_init();

    static display_context_t disp = 0;

    int32_t left = 0;

    RdpDisplayList *rdl = rdl_heap_alloc(100);

    // testPhysics;
    Physics* testPhysics = new_Physics();
    long long last_update = timer_ticks();

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



        rdl_push(rdl,RdpSyncPipe());

        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(0, 0, 255, 128)));

        render_tri(rdl,
            make_16d16(50),   make_16d16(150),
            make_16d16(50),   make_16d16(100),
            make_16d16(100),  make_16d16(130)
        );

        // rdl_push(rdl,RdpSetFogColor(cast64(RDP_COLOR32(255,0,255,50))));

        // render_tri(rdl,
        //     make_16d16(50),    make_16d16(0),
        //     make_16d16(250),   make_16d16(10),
        //     make_16d16(150),   make_16d16(25)
        // );

        float delta = (float)TIMER_MICROS(timer_ticks() - last_update) / 1000.f;
        last_update = timer_ticks();

        rdl_flush(rdl);
        rdl_exec(rdl);

        // Present
        rdp_detach_display();
        display_show(disp);

        left += make_16d16(0, 0.25);

        debugf("delta time: %f.2\n", delta);

        update_Physics(testPhysics, delta);
    }
}