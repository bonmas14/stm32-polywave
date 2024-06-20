#include "sound.h"

#define STEPS_PER_SEC 48000
#define BIT_DEPTH 8


uint32_t osc_saw(uint64_t step);


uint32_t osc_generate(osc_t* osc) {


    uint64_t current_step = ((osc->time + osc->phase) * osc->freq) % STEPS_PER_SEC;

    uint32_t output = 0;

    switch (osc->osc_enum) {
        case TRIANGE:
            break;

        case SAW:
            output = osc_saw(current_step);
            break;

        case SQUARE:
            break;
    }

    osc->time += 1;

    return output;
}

uint32_t osc_saw(uint64_t step) {
    return step / (STEPS_PER_SEC / (1 << BIT_DEPTH));
}
