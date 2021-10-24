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

    display_init(RESOLUTION_640x240, DEPTH_32_BPP, 2, GAMMA_NONE, ANTIALIAS_OFF);

    debug_init_usblog();

    debugf("Hello world!\n");

    rdp_init();
    timer_init();
    controller_init();

    static display_context_t disp = 0;

    RdpDisplayList *rdl = rdl_heap_alloc(100);

    Game* testGame = new_Game(rdl);
    long long last_update = timer_ticks();

    while(1) {
        controller_scan();
        struct controller_data keys = get_keys_down();

        while( !(disp = display_lock()) );

        rdp_attach_display( disp );

        rdl_reset(rdl);

        rdl_push(rdl,RdpSyncPipe());
        rdl_push(rdl,RdpSetClippingI(0, 0, 640, 240));
        rdl_push(rdl,RdpSetOtherModes(SOM_CYCLE_FILL));

        rdl_push(rdl,RdpSetFillColor(RDP_COLOR32(0,0,0,255)));
        rdl_push(rdl,RdpFillRectangleI(0, 0, 640, 240));

        // rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(255, 255, 0, 128)));

        // 30, 26, 22, 18 are P, A, M, B cycle 0

        /* Configure (FOG_RGB * FOG_A) + (MEM_RGB * (1-FOG_A)). For some reason,
        there seems to be no way to use the blend register's alpha, so we
        need to use the fog register for that. */
        // #define BLEND_ENABLE   (1<<14)
        // #define BLEND_MODE_FOG ((3<<30) | (1<<26) | (1<<22))

        // (P*A + M*B)
        // in the case above P=3=FOG_RGB, A=1=FOG_A, M=1=MEM_RGB, B=0=1-A
        rdl_push(rdl, RdpSetOtherModes(
            BLEND_ENABLE |
            READ_ENABLE |
            AA_ENABLE |
            // (cast64(0x1) << 12) | // cvg times alpha
            // (cast64(0x1) << 13) | // use cvg for alpha
            // (cast64(0x1) << 4) | // z compare
            // (cast64(0x1) << 1) | // z source prim
            (cast64(0x1) << 7) | // Color on coverage
            (cast64(0x1) << 8) | // CVG dest wrap
            // (cast64(0x3) << 8) | // CVG dest save
            // (cast64(0x0) << 8) | // CVG dest clamp
            // (cast64(0x1) << 1) | // Alpha compare
            (cast64(0x3) << 30) | (cast64(0x0) << 28) | (cast64(0x1) << 26) | (cast64(0x0) << 24) |
            (cast64(0x1) << 22) | (cast64(0x0) << 20) | (cast64(0x0) << 18) | (cast64(0x0) << 16) ) ); // | (cast64(0x80000000))

        rdl_push(rdl,
            RdpSetCombine(
                // We need to enable same flags for both cycles in 1 cycle mode.
                Comb0_Rgb(ZERO, ZERO, ZERO , ONE) | Comb0_Alpha(ZERO, ZERO, ZERO , ZERO) |
                Comb1_Rgb(ZERO, ZERO, ZERO , ONE) | Comb1_Alpha(ZERO, ZERO, ZERO , ZERO)
            )
        );

        rdl_push(rdl,RdpSetFogColor(RDP_COLOR32(166, 0, 255, 100)));

        render_tri_strip(rdl,
            make_16d16(0),   make_16d16(180),
            make_16d16(0),  make_16d16(240),
            make_16d16(640),   make_16d16(180)
        );
        render_tri_strip_next(rdl, make_16d16(640), make_16d16(240));

        rdl_push(rdl,RdpSyncPipe());

        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(0, 0, 0, 0)));
        rdl_push(rdl,RdpSetFogColor(RDP_COLOR32(255, 255, 255, 128)));

        float delta = (float)TIMER_MICROS(timer_ticks() - last_update) / 1000.f;
        last_update = timer_ticks();

        update_Game(testGame, keys);

        rdl_flush(rdl);
        rdl_exec(rdl);

        // Present
        rdp_detach_display();
        display_show(disp);
    }
}