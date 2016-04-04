#include <3ds.h>

#include "inputs.h"

static circlePosition circle_pad;
static u32 kDown;
static u32 kHeld;

int read_inputs()
{
    hidScanInput();

    // Respond to user input
    kDown = hidKeysDown();
    if (kDown & KEY_START)
    {
        return 0; // in order to return to hbmenu
    }

    kHeld = hidKeysHeld();

    // Read the CirclePad position
    hidCircleRead(&circle_pad);
    return 1;
}

int keypressed(uint32_t key_code)
{
    return ((kDown & key_code) || (kHeld & key_code))
        ? 1 : 0;
}

float stick_dx()
{
    return (float)circle_pad.dx / 160;
}

float stick_dy()
{
    return (float)circle_pad.dy / 160;
}