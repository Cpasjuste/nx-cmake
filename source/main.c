#include <string.h>
#include <stdio.h>
#include <switch.h>
#include <nxlink_print.h>

//#define EMU 1
#define SCREEN_WIDTH    1280
#define SCREEN_HEIGHT   720

typedef struct NXKey {
    char *name;
    int id;
} NXKey;

NXKey keys[27] =
        {
                {"KEY_A            ", 0},    ///< A
                {"KEY_B            ", 1},    ///< B
                {"KEY_X            ", 2},    ///< X
                {"KEY_Y            ", 3},    ///< Y
                {"KEY_LSTICK       ", 4},    ///< Left Stick Button
                {"KEY_RSTICK       ", 5},    ///< Right Stick Button
                {"KEY_L            ", 6},    ///< L
                {"KEY_R            ", 7},    ///< R
                {"KEY_ZL           ", 8},    ///< ZL
                {"KEY_ZR           ", 9},    ///< ZR
                {"KEY_PLUS         ", 10},   ///< Plus
                {"KEY_MINUS        ", 11},   ///< Minus
                {"KEY_DLEFT        ", 12},   ///< D-Pad Left
                {"KEY_DUP          ", 13},   ///< D-Pad Up
                {"KEY_DRIGHT       ", 14},   ///< D-Pad Right
                {"KEY_DDOWN        ", 15},   ///< D-Pad Down
                {"KEY_LSTICK_LEFT  ", 16},   ///< Left Stick Left
                {"KEY_LSTICK_UP    ", 17},   ///< Left Stick Up
                {"KEY_LSTICK_RIGHT ", 18},   ///< Left Stick Right
                {"KEY_LSTICK_DOWN  ", 19},   ///< Left Stick Down
                {"KEY_RSTICK_LEFT  ", 20},   ///< Right Stick Left
                {"KEY_RSTICK_UP    ", 21},   ///< Right Stick Up
                {"KEY_RSTICK_RIGHT ", 22},   ///< Right Stick Right
                {"KEY_RSTICK_DOWN  ", 23},   ///< Right Stick Down
                {"KEY_SL           ", 24},   ///< SL
                {"KEY_SR           ", 25},   ///< SR
                {"KEY_TOUCH       ",  26},

        };

const char *key_to_string(u64 key) {
    for (int i = 0; i < 27; i++) {
        if (BIT(keys[i].id) & key) {
            return keys[i].name;
        }
    }
    return "UNKNOW";
}


int main(int argc, char **argv) {

    u64 kDown = 0;
    Result res = 0;

    gfxInitDefault();
    gfxSetMode(GfxMode_TiledDouble);

#ifdef EMU
    consoleDebugInit(debugDevice_SVC);
    stdout = stderr; // for yuzu
#else
    nxlink_print_init();
#endif

    printf("main loop\n");

    res = hidSetNpadJoyAssignmentModeSingleByDefault(CONTROLLER_PLAYER_1);
    printf("hidSetNpadJoyAssignmentModeSingleByDefault(P1): %x\n", res);
    res = hidSetNpadJoyAssignmentModeSingleByDefault(CONTROLLER_PLAYER_2);
    printf("hidSetNpadJoyAssignmentModeSingleByDefault(P2): %x\n", res);

    while (appletMainLoop()) {

        hidScanInput();

        kDown = hidKeysHeld(CONTROLLER_PLAYER_1);
        if (kDown) {
            printf("P1: %s\n", key_to_string(kDown));
            if (kDown & KEY_B)
                break;
        }

        kDown = hidKeysHeld(CONTROLLER_PLAYER_2);
        if (kDown) {
            printf("P2: %s\n", key_to_string(kDown));
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxWaitForVsync();
    }

    res = hidSetNpadJoyAssignmentModeDual(CONTROLLER_PLAYER_1);
    printf("hidSetNpadJoyAssignmentModeDual(P1): %x\n", res);
    res = hidSetNpadJoyAssignmentModeDual(CONTROLLER_PLAYER_2);
    printf("hidSetNpadJoyAssignmentModeDual(P2): %x\n", res);

    svcSleepThread(1000 * 1000 * 1000);

    gfxExit();
#ifndef EMU
    nxlink_print_exit();
#endif

    return 0;
}
