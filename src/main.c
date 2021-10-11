#include <stdio.h>

#include <libdragon.h>

#include "rdl.h"

int main(void)
{
    init_interrupts();

    display_init(RESOLUTION_640x240, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE);

    debug_init_usblog();

    debugf("Hello world!\n");

    rdp_init();



    static display_context_t disp = 0;

    int left = 0;

    while(1) {

        while( !(disp = display_lock()) );

        rdp_attach_display( disp );


        RdpDisplayList *rdl = rdl_heap_alloc(8);

        rdl_push(rdl,
            RdpSyncPipe(),
            RdpSetClippingI(0, 0, 640, 240),
            RdpSetOtherModes(SOM_CYCLE_FILL),

            RdpSetFillColor(RDP_COLOR16(0,0,0,0)),
            RdpFillRectangleI(0, 0, 320, 240),

            RdpSetFillColor(RDP_COLOR16(31,31,31,0)),
            RdpFillRectangleI(left, left, left + 100, left + 50)
        );


        rdl_flush(rdl);
        rdl_exec(rdl);

        // Present
        rdp_detach_display();
        display_show(disp);
        wait_ms(17);

        left += 1;
    }
}