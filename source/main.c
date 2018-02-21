#include <string.h>
#include <stdio.h>

#include <switch.h>

#include "frame.h"

#define FRAME_WIDTH     384
#define FRAME_HEIGHT    224

static void wait() {

    gfxFlushBuffers();
    gfxSwapBuffers();
    gfxWaitForVsync();

    svcSleepThread((u64) (5 * 1000 * 1000) * 1000); // 5 sec
}

void blit_scale_rgb565(const unsigned char *pixels,
                       int src_width, int src_height,
                       int dst_width, int dst_height) {

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
}

typedef struct tColorRGBA {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} tColorRGBA;

typedef struct Surface {
    int w;
    int h;
    int pitch;
    void *pixels;
} Surface;

/*!
\brief Internal 32 bit rotozoomer with optional anti-aliasing.
Rotates and zooms 32 bit RGBA/ABGR 'src' surface to 'dst' surface based on the control
parameters by scanning the destination surface and applying optionally anti-aliasing
by bilinear interpolation.
Assumes src and dst surfaces are of 32 bit depth.
Assumes dst surface was allocated with the correct dimensions.
\param src Source surface.
\param dst Destination surface.
\param cx Horizontal center coordinate.
\param cy Vertical center coordinate.
\param isin Integer version of sine of angle.
\param icos Integer version of cosine of angle.
\param flipx Flag indicating horizontal mirroring should be applied.
\param flipy Flag indicating vertical mirroring should be applied.
\param smooth Flag indicating anti-aliasing should be used.
*/
void _transformSurfaceRGBA(Surface *src, Surface *dst,
                           int cx, int cy, int isin, int icos, int flipx, int flipy, int smooth) {
    int x, y, t1, t2, dx, dy, xd, yd, sdx, sdy, ax, ay, ex, ey, sw, sh;
    tColorRGBA c00, c01, c10, c11, cswap;
    tColorRGBA *pc, *sp;
    int gap;

    /*
    * Variable setup
    */
    xd = ((src->w - dst->w) << 15);
    yd = ((src->h - dst->h) << 15);
    ax = (cx << 16) - (icos * cx);
    ay = (cy << 16) - (isin * cx);
    sw = src->w - 1;
    sh = src->h - 1;
    pc = (tColorRGBA *) dst->pixels;
    gap = dst->pitch - dst->w * 4;

    /*
    * Switch between interpolating and non-interpolating code
    */
    if (smooth) {
        for (y = 0; y < dst->h; y++) {
            dy = cy - y;
            sdx = (ax + (isin * dy)) + xd;
            sdy = (ay - (icos * dy)) + yd;
            for (x = 0; x < dst->w; x++) {
                dx = (sdx >> 16);
                dy = (sdy >> 16);
                if (flipx) dx = sw - dx;
                if (flipy) dy = sh - dy;
                if ((dx > -1) && (dy > -1) && (dx < (src->w - 1)) && (dy < (src->h - 1))) {
                    sp = (tColorRGBA *) src->pixels;;
                    sp += ((src->pitch / 4) * dy);
                    sp += dx;
                    c00 = *sp;
                    sp += 1;
                    c01 = *sp;
                    sp += (src->pitch / 4);
                    c11 = *sp;
                    sp -= 1;
                    c10 = *sp;
                    if (flipx) {
                        cswap = c00;
                        c00 = c01;
                        c01 = cswap;
                        cswap = c10;
                        c10 = c11;
                        c11 = cswap;
                    }
                    if (flipy) {
                        cswap = c00;
                        c00 = c10;
                        c10 = cswap;
                        cswap = c01;
                        c01 = c11;
                        c11 = cswap;
                    }
                    /*
                    * Interpolate colors
                    */
                    ex = (sdx & 0xffff);
                    ey = (sdy & 0xffff);
                    t1 = ((((c01.r - c00.r) * ex) >> 16) + c00.r) & 0xff;
                    t2 = ((((c11.r - c10.r) * ex) >> 16) + c10.r) & 0xff;
                    pc->r = (((t2 - t1) * ey) >> 16) + t1;
                    t1 = ((((c01.g - c00.g) * ex) >> 16) + c00.g) & 0xff;
                    t2 = ((((c11.g - c10.g) * ex) >> 16) + c10.g) & 0xff;
                    pc->g = (((t2 - t1) * ey) >> 16) + t1;
                    t1 = ((((c01.b - c00.b) * ex) >> 16) + c00.b) & 0xff;
                    t2 = ((((c11.b - c10.b) * ex) >> 16) + c10.b) & 0xff;
                    pc->b = (((t2 - t1) * ey) >> 16) + t1;
                    t1 = ((((c01.a - c00.a) * ex) >> 16) + c00.a) & 0xff;
                    t2 = ((((c11.a - c10.a) * ex) >> 16) + c10.a) & 0xff;
                    pc->a = (((t2 - t1) * ey) >> 16) + t1;
                }
                sdx += icos;
                sdy += isin;
                pc++;
            }
            pc = (tColorRGBA *) ((u8 *) pc + gap);
        }
    } else {
        for (y = 0; y < dst->h; y++) {
            dy = cy - y;
            sdx = (ax + (isin * dy)) + xd;
            sdy = (ay - (icos * dy)) + yd;
            for (x = 0; x < dst->w; x++) {
                dx = (short) (sdx >> 16);
                dy = (short) (sdy >> 16);
                if (flipx) dx = (src->w - 1) - dx;
                if (flipy) dy = (src->h - 1) - dy;
                if ((dx >= 0) && (dy >= 0) && (dx < src->w) && (dy < src->h)) {
                    sp = (tColorRGBA *) ((u8 *) src->pixels + src->pitch * dy);
                    sp += dx;
                    *pc = *sp;
                }
                sdx += icos;
                sdy += isin;
                pc++;
            }
            pc = (tColorRGBA *) ((u8 *) pc + gap);
        }
    }
}

int main(int argc, char **argv) {

    gfxInitDefault();

    consoleInit(NULL);
    consoleDebugInit(debugDevice_SVC);
    stdout = stderr;

    blit_scale_rgb565((const unsigned char *) fba_frame,
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

