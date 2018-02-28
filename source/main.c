#include <string.h>
#include <stdio.h>
#include <switch.h>

#include "frame.h"

#define SCREEN_WIDTH    1280
#define SCREEN_HEIGHT   720

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

void gfxClear() {

    u32 w, h;
    u32 *dst = (u32 *) gfxGetFramebuffer(&w, &h);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x += 4) {
            *((u128 *) &dst[gfxGetFramebufferDisplayOffset(x, y)]) = 0;
        }
    }
}

enum {

    NX_TRANSFORM_DEFAULT,
    NX_TRANSFORM_ROT_90,
    NX_TRANSFORM_ROT_180,
    NX_TRANSFORM_ROT_270

};

typedef struct NXTransform {

    float with;
    float height;
    float scalex;
    float scaley;
    float rotation;
    u32 transform;
    u16 *pixels;

} NXTransform;

NXTransform transforms[4] = {

        {FRAME_WIDTH, FRAME_HEIGHT, 1, 1, 0, NX_TRANSFORM_DEFAULT, (u16 *) fba_frame},
        {FRAME_WIDTH, FRAME_HEIGHT, 1, 1, 0, NX_TRANSFORM_ROT_90,  (u16 *) fba_frame},
        {FRAME_WIDTH, FRAME_HEIGHT, 1, 1, 0, NX_TRANSFORM_ROT_180, (u16 *) fba_frame},
        {FRAME_WIDTH, FRAME_HEIGHT, 1, 1, 0, NX_TRANSFORM_ROT_270, (u16 *) fba_frame}
};

void nx_blit_transform_rgb565_to_rgba(NXTransform *transform) {

    u32 fbw, fbh;
    s32 vw = (s32) (SCREEN_WIDTH / transform->scalex);
    s32 vh = (s32) (SCREEN_HEIGHT / transform->scaley);

    switch (transform->transform) {
        case NX_TRANSFORM_ROT_90:
            gfxConfigureTransform(NATIVE_WINDOW_TRANSFORM_FLIP_V
                                  | NATIVE_WINDOW_TRANSFORM_ROT_90);
            break;
        case NX_TRANSFORM_ROT_180:
            gfxConfigureTransform(NATIVE_WINDOW_TRANSFORM_FLIP_H);
            break;
        case NX_TRANSFORM_ROT_270:
            gfxConfigureTransform(NATIVE_WINDOW_TRANSFORM_FLIP_H
                                  | NATIVE_WINDOW_TRANSFORM_ROT_90);
            break;
        default:
            gfxConfigureTransform(NATIVE_WINDOW_TRANSFORM_FLIP_V);
            break;
    }

    gfxConfigureResolution(vw, vh);
    u32 *fb_buf = (u32 *) gfxGetFramebuffer(&fbw, &fbh);

    int x, y;
    int w = (int) transform->with;
    int h = (int) transform->height;
    int cx = (fbw - w) / 2;
    int cy = (fbh - h) / 2;
    unsigned int p, r, g, b;

    printf("res:%ix%i | fb:%ix%i | tex:%ix%i | scale:%fx%f\n",
           vw, vh, fbw, fbh, (int) transform->with, (int) transform->height, transform->scalex, transform->scaley);

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {

            p = transform->pixels[y * w + x];
            r = ((p & 0xf800) >> 11) << 3;
            g = ((p & 0x07e0) >> 5) << 2;
            b = (p & 0x001f) << 3;

            fb_buf[(u32) gfxGetFramebufferDisplayOffset((u32) x + cx, (u32) y + cy)] =
                    RGBA8_MAXALPHA(r, g, b);
        }
    }
}

int main(int argc, char **argv) {

    gfxInitDefault();
    gfxSetMode(GfxMode_TiledDouble);

    consoleDebugInit(debugDevice_SVC);
    stdout = stderr; // for yuzu

    int selected = 0;

    nx_blit_transform_rgb565_to_rgba(&transforms[selected]);
    gfxFlushBuffers();
    gfxSwapBuffers();
    gfxWaitForVsync();
    nx_blit_transform_rgb565_to_rgba(&transforms[selected]);

    while (appletMainLoop()) {

        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        if (kDown) {
            selected++;
            if (selected > 3) {
                selected = 0;
            }
            gfxClear();
            nx_blit_transform_rgb565_to_rgba(&transforms[selected]);
            if (kDown & KEY_PLUS)
                break;
            svcSleepThread(1000 * 1000 * 500);
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxWaitForVsync();
    }

    gfxExit();

    return 0;
}
