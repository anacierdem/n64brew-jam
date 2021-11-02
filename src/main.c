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
        int controllers = get_controllers_present();
        struct controller_data keys = get_keys_pressed();

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
        // #define SOM_BLEND_ENABLE   (1<<14)
        // #define BLEND_MODE_FOG ((3<<30) | (1<<26) | (1<<22))

        // (P*A + M*B)
        // in the case above P=3=FOG_RGB, A=1=FOG_A, M=1=MEM_RGB, B=0=1-A
        rdl_push(rdl, RdpSetOtherModes(
            SOM_CYCLE_1 |
            SOM_BLENDING |
            SOM_READ_ENABLE |
            SOM_AA_ENABLE |
            SOM_COLOR_ON_COVERAGE |
            SOM_COVERAGE_DEST_WRAP |
            (cast64(0x0) << 30) | (cast64(0x0) << 28) | (cast64(0x0) << 26) | (cast64(0x0) << 24) |
            (cast64(0x1) << 22) | (cast64(0x0) << 20) | (cast64(0x0) << 18) | (cast64(0x0) << 16) ) );

        rdl_push(rdl,
            RdpSetCombine(
                // We need to enable same flags for both cycles in 1 cycle mode.
                Comb0_Rgb(ZERO, ZERO, ZERO , PRIM) | Comb0_Alpha(ZERO, ZERO, ZERO , PRIM) |
                Comb1_Rgb(ZERO, ZERO, ZERO , PRIM) | Comb1_Alpha(ZERO, ZERO, ZERO , PRIM)
            )
        );

        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(166, 0, 255, 100)));
        rdl_push(rdl,RdpSetFogColor(RDP_COLOR32(166, 0, 255, 100)));

        render_tri_strip(rdl,
            make_16d16(0),   make_16d16(180),
            make_16d16(0),  make_16d16(240),
            make_16d16(640),   make_16d16(180)
        );
        render_tri_strip_next(rdl, make_16d16(640), make_16d16(240));

        rdl_push(rdl,RdpSyncPipe());

        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(255, 255, 255, 128)));
        rdl_push(rdl,RdpSetFogColor(RDP_COLOR32(255, 255, 255, 128)));

        float delta = (float)TIMER_MICROS(timer_ticks() - last_update) / 1000.f;
        last_update = timer_ticks();

        update_Game(testGame, controllers, keys);

        rdl_flush(rdl);
        rdl_exec(rdl);

        // Present
        rdp_detach_display();
        display_show(disp);
    }
}