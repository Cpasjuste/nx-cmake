#include <string.h>
#include <stdio.h>
#include <switch.h>

#include "frame.h"

#define FRAME_WIDTH     384
#define FRAME_HEIGHT    224

/*
static void wait() {

    gfxFlushBuffers();
    gfxSwapBuffers();
    gfxWaitForVsync();

    svcSleepThread((u64) (5 * 1000 * 1000) * 1000); // 5 sec
}
*/

void nx_blit_scale_rgb565_to_rgba(const unsigned char *pixels,
                                  int src_width, int src_height,
                                  int dst_width, int dst_height) {

    double sw = (double) dst_width / (double) src_width;
    double sh = (double) dst_height / (double) src_height;
    int cx = (1280 - dst_width) / 2;
    int cy = (720 - dst_height) / 2;

    u32 *dst = (u32 *) gfxGetFramebuffer(NULL, NULL);
    unsigned short *src = (unsigned short *) pixels;
    unsigned int p, r, g, b;

    for (int y = 0; y < dst_height; y++) {
        for (int x = 0; x < dst_width; x++) {

            p = src[((int) (y / sh) * src_width) + (int) (x / sw)];
            r = ((p & 0xf800) >> 11);
            g = ((p & 0x07e0) >> 5);
            b = (p & 0x001f);

            dst[(u32) gfxGetFramebufferDisplayOffset((u32) x + cx, (u32) y + cy)] =
                    RGBA8_MAXALPHA(r << 3, g << 2, b << 3);
        }
    }
}

int main(int argc, char **argv) {

    gfxInitDefault();
    gfxSetMode(GfxMode_TiledSingle);

    //consoleInit(NULL);
    consoleDebugInit(debugDevice_SVC);
    stdout = stderr; // for yuzu

    nx_blit_scale_rgb565_to_rgba((const unsigned char *) fba_frame,
                                 FRAME_WIDTH, FRAME_HEIGHT,
                                 (int) (FRAME_WIDTH * 1.9f), (int) (FRAME_HEIGHT * 1.9f));

    while (appletMainLoop()) {

        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxWaitForVsync();
    }

    /*
	//Move the cursor to row 16 and column 20 and then prints "Hello World!"
	//To move the cursor you have to print "\x1b[r;cH", where r and c are respectively
	//the row and column where you want your cursor to move
	//printf("\x1b[16;20HHello World!");

	while(appletMainLoop())
	{
        consoleClear();
		//Scan all the inputs. This should be done once for each frame
		hidScanInput();

		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        printf("%ld\n", svcGetSystemTick());



		if (kDown & KEY_PLUS) break; // break in order to return to hbmenu

		gfxFlushBuffers();
		gfxSwapBuffers();
		gfxWaitForVsync();
	}
    */

    gfxExit();

    return 0;
}

