#include <3ds.h>

#include "input.h"

static circlePosition circle_pad;
static u32 kDown;
static u32 kUp;
static u32 kHeld;

int read_inputs()
{
    hidScanInput();

    kDown = hidKeysDown();
    if (kDown & KEY_START)
    {
        return 0;
    }

    kHeld = hidKeysHeld();
    kUp = hidKeysUp();

    hidCircleRead(&circle_pad);
    return 1;
}

int pressed(uint32_t key_code)
{
    return (kDown & key_code)
        ? 1 : 0;
}

int held(uint32_t key_code)
{
    return (kHeld & key_code)
        ? 1 : 0;
}

int released(uint32_t key_code)
{
    return (kUp & key_code)
        ? 1 : 0;
}

float stick_dx()
{
    return (float)circle_pad.dx / 160;
}

float stick_dy()
{
    // invert y-axis
    return -1 * (float)circle_pad.dy / 160;
}