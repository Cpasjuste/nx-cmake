#include <string.h>
#include <stdio.h>
#include <switch.h>
#include <nxlink_print.h>

//#define EMU 1
#define SCREEN_WIDTH    1280
#define SCREEN_HEIGHT   720

int main(int argc, char **argv) {

    u64 kDown = 0;

    gfxInitDefault();
    gfxSetMode(GfxMode_TiledDouble);

#ifdef EMU
    consoleDebugInit(debugDevice_SVC);
    stdout = stderr; // for yuzu
#else
    nxlink_print_init();
#endif

    printf("main loop\n");

    hidSetNpadJoyAssignmentModeSingleByDefault(CONTROLLER_PLAYER_1);
    hidSetNpadJoyAssignmentModeSingleByDefault(CONTROLLER_PLAYER_2);

    while (appletMainLoop()) {

        hidScanInput();

        kDown = hidKeysDown(CONTROLLER_PLAYER_1);
        if (kDown) {
            printf("P1: %lx\n", kDown);
            if (kDown & KEY_B)
                break;
        }

        kDown = hidKeysDown(CONTROLLER_PLAYER_2);
        if (kDown) {
            printf("P2: %lx\n", kDown);
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxWaitForVsync();
    }

    hidSetNpadJoyAssignmentModeDual(CONTROLLER_PLAYER_1);
    hidSetNpadJoyAssignmentModeDual(CONTROLLER_PLAYER_2);

    gfxExit();
#ifndef EMU
    nxlink_print_exit();
#endif

    return 0;
}
