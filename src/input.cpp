#include <cstdint>
#include <vpad/input.h>

uint32_t GetInput(uint32_t mask) {
    VPADStatus input;
    VPADReadError error;
    VPADRead(VPAD_CHAN_0, &input, 1, &error);
    return input.trigger & mask;
}

uint32_t WaitInput(uint32_t mask) {
    VPADStatus input;
    VPADReadError error;
    while (true) {
        VPADRead(VPAD_CHAN_0, &input, 1, &error);
        if (input.trigger & mask) {
            return input.trigger & mask;
        }
    }
}
