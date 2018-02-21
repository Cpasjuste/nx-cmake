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

void nx_blit_scale_rgb565(const unsigned char *pixels,
                          int src_width, int src_height,
                          int dst_width, int dst_height) {

    double scaleWidth = (double) dst_width / (double) src_width;
    double scaleHeight = (double) dst_height / (double) src_height;

    //printf("scale: %f x %f\n", scaleWidth, scaleHeight);

    int dst_bpp = 4;
    int src_bpp = 2;

    u32 *dst = (u32 *) gfxGetFramebuffer(NULL, NULL);
    unsigned short *src = (unsigned short *) pixels;

    unsigned int v, r, g, b;

    for (int cy = 0; cy < dst_height; cy++) {

        for (int cx = 0; cx < dst_width; cx++) {

            v = src[(((int) (cy / scaleHeight) * (src_width * src_bpp)) +
                     ((int) (cx / scaleWidth) * src_bpp))];
            r = (v & 0xf800) >> 11;
            g = (v & 0x07e0) >> 5;
            b = v & 0x001f;
            u32 pixel = RGBA8_MAXALPHA(r << 3, g << 2, b << 3);

            /*
            u32 pixel = RGBA8_MAXALPHA(((nearestMatch & 0xf800) >> 11) << 3,
                                             ((nearestMatch & 0x07e0) >> 5) << 2,
                                             (nearestMatch & 0x001f) << 3);
            */
            dst[(u32) gfxGetFramebufferDisplayOffset((u32) cx, (u32) cy)] = pixel;
        }
    }

    /*
    unsigned short *q;
    unsigned int v, r, g, b;
    int x, y, w, h;
    u32 pixel;
    unsigned subx, suby;
    int tgtw, tgth, centerx, centery;

    printf("blit_scale_rgb565: src: %ix%i | dst: %ix%i\n",
           src_width, src_height, dst_width, dst_height);

    w = src_width;
    h = src_height;
    int xsf = dst_width / w;
    int ysf = dst_height / h;

    int sf = xsf;
    if (ysf < sf)
        sf = ysf;
    tgtw = w * sf;
    tgth = h * sf;
    centerx = (1280 - tgtw) / 2;
    centery = (720 - tgth) / 2;

    q = (unsigned short *) pixels;
    u32 *buffer = (u32 *) gfxGetFramebuffer(NULL, NULL);

    for (x = 0; x < w; x++) {
        for (y = 0; y < h; y++) {

            v = q[y * w + x];
            r = (v & 0xf800) >> 11;
            g = (v & 0x07e0) >> 5;
            b = v & 0x001f;
            pixel = RGBA8_MAXALPHA(r << 3, g << 2, b << 3);

            for (subx = 0; subx < xsf; subx++) {
                for (suby = 0; suby < ysf; suby++) {
                    buffer[(u32) gfxGetFramebufferDisplayOffset(
                            (u32) ((x * sf) + subx + centerx),
                            (u32) ((y * sf) + suby + centery))] = pixel;
                }
            }
        }
    }
    */

    /*
    unsigned short *q;
    unsigned int v;
    int x, y, w, h;
    unsigned subx, suby;
    int tgtw, tgth, centerx, centery;

    printf("blit_scale_rgb565: src: %ix%i | dst: %ix%i\n",
           src_width, src_height, dst_width, dst_height);

    w = src_width;
    h = src_height;
    int xsf = dst_width / w;
    int ysf = dst_height / h;

    int sf = xsf;
    if (ysf < sf)
        sf = ysf;
    tgtw = w * sf;
    tgth = h * sf;
    centerx = (1280 - tgtw) / 2;
    centery = (720 - tgth) / 2;

    q = (unsigned short *) pixels;
    u32 *buffer = (u32 *) gfxGetFramebuffer(NULL, NULL);

    for (x = 0; x < w; x++) {
        for (y = 0; y < h; y++) {

            v = q[y * w + x];

            for (subx = 0; subx < xsf; subx++) {
                for (suby = 0; suby < ysf; suby++) {
                    buffer[(u32) gfxGetFramebufferDisplayOffset(
                            (u32) ((x * sf) + subx + centerx),
                            (u32) ((y * sf) + suby + centery))] =
                            RGBA8_MAXALPHA(((v & 0xf800) >> 11) << 3,
                                           ((v & 0x07e0) >> 5) << 2,
                                           (v & 0x001f) << 3);
                }
            }
        }
    }
    */
}

int main(int argc, char **argv) {

    gfxInitDefault();
    gfxSetMode(GfxMode_TiledSingle);

    //consoleInit(NULL);
    consoleDebugInit(debugDevice_SVC);
    stdout = stderr; // for yuzu

    nx_blit_scale_rgb565((const unsigned char *) fba_frame,
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

