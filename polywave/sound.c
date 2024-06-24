#include "sound.h"

#define STEPS_PER_SEC 48000
#define MIN_CLIP 1

uint16_t uint16_lerp(uint16_t a, uint16_t b, uint16_t max, uint16_t t);

uint16_t osc_saw(uint32_t step);
uint16_t osc_tri(uint32_t step);
uint16_t osc_sqr(uint32_t step);

void adsr_update(osc_t* osc, adsr_t* adsr, int8_t gate) {
    switch (adsr->state_enum) {
        case ADSR_IDLE:
            if (gate) { 
                adsr->state_enum = ADSR_ATTACK;
                adsr->state_start_time = osc->time;
                adsr->state_start_value = MIN_CLIP;
            }

            adsr->current_value = MIN_CLIP;
            break;
        case ADSR_ATTACK:
            if (!gate)
                adsr->state_enum = ADSR_RELEASE;

            if ((osc->time - adsr->state_start_time) > adsr->attack_time)
                adsr->state_enum = ADSR_DECAY;

            adsr->current_value = uint16_lerp(adsr->state_start_value, adsr->attack_value, adsr->attack_time, osc->time - adsr->state_start_time);

            if (adsr->state_enum != ADSR_ATTACK) { 
                adsr->state_start_time = osc->time;
                adsr->state_start_value = adsr->current_value;
            }
            break;
        case ADSR_DECAY:
            if (!gate)
                adsr->state_enum = ADSR_RELEASE;

            if ((osc->time - adsr->state_start_time) > adsr->decay_time)
                adsr->state_enum = ADSR_SUSTAIN;

            adsr->current_value = uint16_lerp(adsr->attack_value, adsr->sustain_value, adsr->decay_time, osc->time - adsr->state_start_time);

            if (adsr->state_enum != ADSR_DECAY) {
                adsr->state_start_time = osc->time;
                adsr->state_start_value = adsr->current_value;
            }
            break;

        case ADSR_SUSTAIN:
            if (!gate)
                adsr->state_enum = ADSR_RELEASE;

            adsr->current_value = adsr->sustain_value;

            if (adsr->state_enum != ADSR_SUSTAIN) {
                adsr->state_start_time = osc->time;
                adsr->state_start_value = adsr->current_value;
            }
            break;

        case ADSR_RELEASE:
            if (gate)
                adsr->state_enum = ADSR_ATTACK;

            if ((osc->time - adsr->state_start_time) > adsr->release_time)
                adsr->state_enum = ADSR_IDLE;

            adsr->current_value = uint16_lerp(adsr->state_start_value, MIN_CLIP, adsr->release_time, osc->time - adsr->state_start_time);

            if (adsr->state_enum != ADSR_RELEASE) {
                adsr->state_start_time = osc->time;
                adsr->state_start_value = adsr->current_value;
            }
            break;
    }
}


int8_t osc_to_8bit(int16_t sample) {
    return sample / 188;
}

int16_t osc_mul(int16_t a, int16_t b) {
    return ((int32_t)a * (int32_t)b) >> 15;
}

int16_t osc_generate(osc_t* osc, int32_t phase_shift) {
    uint32_t current_step = ((osc->time + phase_shift) * osc->freq) % STEPS_PER_SEC;

    uint16_t output = 0;

    switch (osc->osc_enum) {
        case OSC_TRIANGLE:
            output = osc_tri(current_step);
            break;

        case OSC_SAW:
            output = osc_saw(current_step);
            break;

        case OSC_SQUARE:
            output = osc_sqr(current_step);
            break;
    }

    osc->time += 1;

    return output - STEPS_PER_SEC / 2;
}

uint16_t osc_saw(uint32_t step) {
    return step;
}

uint16_t osc_tri(uint32_t step) {
    step *= 2;
    
    if (step >= STEPS_PER_SEC) {
        step %= STEPS_PER_SEC;
        step = STEPS_PER_SEC - step;
    }

    return step;
}

uint16_t osc_sqr(uint32_t step) {
    if (step < (STEPS_PER_SEC / 2))
        return STEPS_PER_SEC;

    return 0;
}

uint16_t uint16_lerp(uint16_t a, uint16_t b, uint16_t max, uint16_t t) {
    return (a * (max - t) + b * t) / max;
}
