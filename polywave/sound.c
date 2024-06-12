#include "sound.h"

#define STEPS_PER_SEC 48000
#define BIT_DEPTH 8

uint64_t time = 0;

uint8_t osc_saw(uint64_t step);
void osc_tick(void) {
    time++;
}
uint8_t osc_generate(uint32_t osc_enum, int32_t freq) {
    uint64_t current_step = (time * freq) % STEPS_PER_SEC;

    uint8_t output = 0;

    switch (osc_enum) {
        case TRIANGE:
            break;

        case SAW:
            output = osc_saw(current_step);
            break;

        case SQUARE:
            break;
    }

    return output;
}

uint8_t osc_saw(uint64_t step) {
    return step / (STEPS_PER_SEC / (1 << BIT_DEPTH));
}
