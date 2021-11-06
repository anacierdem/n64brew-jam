#include <stdio.h>
#include <libdragon.h>

#include "rdl.h"
#include "geometry.h"
#include "game.hpp"

int main(void)
{
    // Default scene width: 8m, heigth: 6m
    display_init(RESOLUTION_640x240, DEPTH_32_BPP, 2, GAMMA_NONE, ANTIALIAS_OFF);

    debug_init_usblog();

    debugf("Hello world!\n");

    rdp_init();
    timer_init();
    controller_init();

    static display_context_t disp = 0;

    RdpDisplayList *rdl = rdl_heap_alloc(300);

    Game* testGame = new_Game(rdl);
    // long long last_update = timer_ticks();

    while(1) {
        while( !(disp = display_lock()) );
        rdp_attach_display( disp );

        rdl_reset(rdl);

        rdl_push(rdl,RdpSyncPipe());

        // Set scissor
        rdl_push(rdl,RdpSetClippingI(0, 0, 640, 240));

        // Clear
        rdl_push(rdl,RdpSetOtherModes(SOM_CYCLE_FILL));

        rdl_push(rdl,RdpSetFillColor(RDP_COLOR32(0,0,0,255)));
        rdl_push(rdl,RdpFillRectangleI(0, 0, 640, 240));

        // Set modes
        rdl_push(rdl, RdpSetOtherModes(
            SOM_CYCLE_1 |
            SOM_BLENDING |
            SOM_READ_ENABLE |
            SOM_AA_ENABLE |
            SOM_COLOR_ON_COVERAGE |
            SOM_COVERAGE_DEST_WRAP |
            // (P*A + M*B)
            (cast64(0x0) << 30) | (cast64(0x0) << 28) | (cast64(0x0) << 26) | (cast64(0x0) << 24) |
            (cast64(0x1) << 22) | (cast64(0x0) << 20) | (cast64(0x0) << 18) | (cast64(0x0) << 16) ) );

        rdl_push(rdl,
            RdpSetCombine(
                // We need to enable same flags for both cycles in 1 cycle mode.
                Comb0_Rgb(ZERO, ZERO, ZERO, PRIM) | Comb0_Alpha(ZERO, ZERO, ZERO, PRIM) |
                Comb1_Rgb(ZERO, ZERO, ZERO, PRIM) | Comb1_Alpha(ZERO, ZERO, ZERO, PRIM)
            )
        );

        // float delta = (float)TIMER_MICROS(timer_ticks() - last_update) / 1000.f;
        // last_update = timer_ticks();

        update_Game(testGame);

        rdl_flush(rdl);
        rdl_exec(rdl);

        // Present
        rdp_detach_display();
        display_show(disp);
    }
}