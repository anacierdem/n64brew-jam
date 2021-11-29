#include <stdio.h>
#include <stdlib.h>

#include <libdragon.h>

#include "rdl.h"
#include "geometry.h"
#include "game.hpp"

// Init DFS first as global C++ object use DFS
__attribute__((constructor)) void __init_dfs()
{
    dfs_init(DFS_DEFAULT_LOCATION);
}

int main(void)
{
    int seed = TICKS_READ();
    srand(seed);
    // Default scene width: 8m, heigth: 6m
    display_init(RESOLUTION_640x240, DEPTH_32_BPP, 2, GAMMA_NONE, ANTIALIAS_OFF);

    debug_init_usblog();

    rdp_init();
    timer_init();
    controller_init();
    audio_init(44100, 6);
    mixer_init(7);

    debugf("init seed: %u\n", seed);

    static display_context_t disp = 0;

    RdpDisplayList *rdl = rdl_stack_alloc(1000);

    // TODO: move to stack
    Game* testGame = new_Game(rdl);

    // Set scissor
    rdl_push(rdl,RdpSetClippingI(0, 0, 640, 240));

    rdl_push(rdl,
        RdpSetCombine(
            // We need to enable same flags for both cycles in 1 cycle mode.
            Comb0_Rgb(ZERO, ZERO, ZERO, PRIM) | Comb0_Alpha(ZERO, ZERO, ZERO, PRIM) |
            Comb1_Rgb(ZERO, ZERO, ZERO, PRIM) | Comb1_Alpha(ZERO, ZERO, ZERO, PRIM)
        )
    );

    rdl_flush(rdl);
    rdl_exec(rdl);

    while(1) {
        while( !(disp = display_lock()) );
        rdp_attach_display( disp );

        rdl_reset(rdl);

        rdl_push(rdl,RdpSyncPipe());

        update_BG(testGame);
        update_Game(testGame);

        rdl_flush(rdl);
        rdl_exec(rdl);

        rdp_detach_display();
        update_UI(testGame, disp);

        // Present
        display_show(disp);
        if (audio_can_write()) {
            short *buf = audio_write_begin();
            mixer_poll(buf, audio_get_buffer_length());
            audio_write_end();
        }
    }
}