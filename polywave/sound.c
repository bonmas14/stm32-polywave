#include "sound.h"

#define STEPS_PER_SEC 48000
#define BIT_DEPTH 8
int32_t osc_saw(uint32_t step);
int32_t osc_tri(uint32_t step);
int32_t osc_sqr(uint32_t step);

int32_t osc_generate(osc_t* osc, int32_t phase_shift) {
    uint32_t current_step = ((osc->time + phase_shift) * osc->freq) % STEPS_PER_SEC;

    int32_t output = 0;

    switch (osc->osc_enum) {
        case TRIANGLE:
            output = osc_tri(current_step);
            break;

        case SAW:
            output = osc_saw(current_step);
            break;

        case SQUARE:
            output = osc_sqr(current_step);
            break;
    }

    osc->time += 1;

    return output - (1 << BIT_DEPTH) / 2;
}

int32_t osc_saw(uint32_t step) {
    return step / (STEPS_PER_SEC / ((1 << BIT_DEPTH) - 1));
}

int32_t osc_tri(uint32_t step) {
    step *= 2;
    
    if (step >= STEPS_PER_SEC) {
        step %= STEPS_PER_SEC;
        step = STEPS_PER_SEC - step;
    }

    return step / (STEPS_PER_SEC / ((1 << BIT_DEPTH) - 1));
}

int32_t osc_sqr(uint32_t step) {
    if (step < (STEPS_PER_SEC / 2))
        return (1 << BIT_DEPTH) - 1;
    return 0;
}
