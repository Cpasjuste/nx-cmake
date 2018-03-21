#include <string.h>
#include <stdio.h>
#include <switch.h>
#include <nxlink_print.h>

//#define EMU 1
#define SCREEN_WIDTH    1280
#define SCREEN_HEIGHT   720

int main(int argc, char **argv) {

    gfxInitDefault();
    gfxSetMode(GfxMode_TiledDouble);

#ifdef EMU
    consoleDebugInit(debugDevice_SVC);
    stdout = stderr; // for yuzu
#else
    nxlink_print_init();
#endif

    printf("main loop\n");

    while (appletMainLoop()) {

        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        if (kDown & KEY_PLUS) {
            break;
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxWaitForVsync();
    }

    gfxExit();
#ifndef EMU
    nxlink_print_exit();
#endif

    return 0;
}
