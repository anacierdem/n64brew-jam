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


        RdpDisplayList *rdl = rdl_heap_alloc(100);

        
        rdl_push(rdl,RdpSyncPipe());
        rdl_push(rdl,RdpSetClippingI(0, 0, 640, 240));
        rdl_push(rdl,RdpSetOtherModes(SOM_CYCLE_FILL));

        rdl_push(rdl,RdpSetFillColor(RDP_COLOR16(0,0,0,0)));
        rdl_push(rdl,RdpFillRectangleI(0, 0, 640, 240));

        rdl_push(rdl,RdpSetFillColor(RDP_COLOR16(31,31,31,0)));
        rdl_push(rdl,RdpFillRectangleI(left, left, left + 100, left + 50));

        rdl_push(rdl,RdpSyncPipe());


        rdl_push(rdl,RdpSetPrimColor(cast64(RDP_COLOR32(128,0,0,100))));
        rdl_push(rdl,RdpSetBlendColor(cast64(RDP_COLOR32(0,255,0,100))));
        rdl_push(rdl,RdpSetFogColor(cast64(RDP_COLOR32(0,0,255,100))));


        rdl_push(rdl, ((cast64(0x2f)<<56) |
            (cast64(0x3) << 30) | (cast64(0x3) << 28) | (cast64(0x3) << 26) | (cast64(0x3) << 24) |
            (cast64(0x3) << 22) | (cast64(0x3) << 20) | (cast64(0x3) << 18) | (cast64(0x3) << 16) )); // | (cast64(0x80000000)) 


        rdl_push(rdl,RdpSetCombine(Comb1_Rgb(ZERO, ZERO, ZERO, PRIM), Comb1_Alpha(ZERO, ZERO, ZERO, PRIM)));


        // rdl_push(rdl,0xEF0000FF80000000);

        int16_t     YL = 50 << 2, YM = 25 << 2, YH = 0 << 2;
        int32_t     XL = 150 << 16, XH = 50 << 16, XM = 50 << 16,
                    DxLDy = ((-100) / 25) << 16,
                    DxHDy = ((0) / 1) << 16,
                    DxMDy = ((100) / 25) << 16;

        debugf("start\n");

        rdl_push(rdl, (cast64(0xC8) << 56) | (cast64(1) << 55) | (cast64(YL) << 32) | (cast64(YM) << 16) | (cast64(YH) << 0) );
        rdl_push(rdl, (cast64(XL) << 32) | (cast64(DxLDy) << 0) );
        rdl_push(rdl, (cast64(XH) << 32) | (cast64(DxHDy) << 0) );
        rdl_push(rdl, (cast64(XM) << 32) | (cast64(DxMDy) << 0) );

        // rdl_push(rdl,0xC88000C800640000);
        // rdl_push(rdl,0x00640000FFFC0000);
        // rdl_push(rdl,0x0000000000000000);
        // rdl_push(rdl,0x0000000000040000);

        rdl_flush(rdl);
        rdl_exec(rdl);

        // Present
        rdp_detach_display();
        display_show(disp);
        wait_ms(100);

        left += 1;

        while(1) {};
    }
}