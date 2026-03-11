#include <stdio.h>
#include <stdlib.h>

#include <libdragon.h>

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

    debug_init_isviewer();
    debug_init_usblog();

    rdp_init();
    timer_init();
    controller_init();
    audio_init(44100, 6);
    mixer_init(8);

    debugf("init seed: %u\n", seed);

    static display_context_t disp = 0;

    // TODO: move to stack
    Game* testGame = new_Game();

    // Set scissor
    rdpq_set_scissor(0, 0, 640, 240);

    rdpq_set_combiner_raw(
        RDPQ_COMBINER1((ZERO, ZERO, ZERO, PRIM), (ZERO, ZERO, ZERO, PRIM))
    );

    while(1) {
        surface_t *disp = display_get();
        rdpq_attach(disp, NULL);

        update_BG(testGame);
        update_Game(testGame);


        rdpq_detach_wait();
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