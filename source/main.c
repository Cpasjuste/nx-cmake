#include <string.h>
#include <stdio.h>
#include <switch.h>
#include <nxlink_print.h>

#define EMU 1
#define SCREEN_WIDTH    1280
#define SCREEN_HEIGHT   720

#include <GL/gl.h>
#include <GL/osmesa.h>

static u8 *nx_fb;

int main(int argc, char **argv) {

    gfxInitDefault();
    gfxSetMode(GfxMode_LinearDouble);

#ifdef EMU
    consoleDebugInit(debugDevice_SVC);
    stdout = stderr;
#else
    nxlink_print_init();
#endif

    OSMesaContext mesa_ctx = OSMesaCreateContextExt(OSMESA_RGBA, 0, 0, 0, NULL);
    if (!mesa_ctx) {
        printf("OSMesaCreateContextExt() failed!\n");
        return -1;
    }

    u32 w, h;
    nx_fb = gfxGetFramebuffer(&w, &h);
    if (!OSMesaMakeCurrent(mesa_ctx, nx_fb,
                           GL_UNSIGNED_BYTE, (GLsizei) w, (GLsizei) h)) {
        printf("OSMesaMakeCurrent (8 bits/channel) failed!\n");
        OSMesaDestroyContext(mesa_ctx);
        return -1;
    }

    OSMesaPixelStore(OSMESA_Y_UP, 0);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, w, h, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);

    printf("main loop\n");

    bool flip = 0;

    while (appletMainLoop()) {

        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        if (kDown & KEY_PLUS) {
            break;
        }


        if(flip) {
            glClearColor(0, 1, 0, 1);
        } else {
            glClearColor(1, 0, 0, 1);
        }
        flip = !flip;
        glClear(GL_COLOR_BUFFER_BIT);
        //svcSleepThread(1000 * 1000 * 1000);

        glFinish();

        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxWaitForVsync();

        nx_fb = gfxGetFramebuffer(&w, &h);

        printf("flip\n");
    }

    gfxExit();
#ifndef EMU
    nxlink_print_exit();
#endif

    return 0;
}
