#pragma once

#include <stddef.h>
#include <stdint.h>

enum {
    OSC_TRIANGLE,
    OSC_SAW,
    OSC_SQUARE
};

enum {
    ADSR_IDLE = 0,
    ADSR_ATTACK,
    ADSR_DECAY,
    ADSR_SUSTAIN,
    ADSR_RELEASE
};

typedef struct {
    uint8_t osc_enum;
    int64_t time;
    int32_t freq;
} osc_t;

typedef struct {
    int8_t state_enum;
    int64_t state_start_time;
    int16_t state_start_value;
    int16_t current_value;

    //config
    int16_t attack_time;
    int16_t attack_value;
    int16_t decay_time;
    int16_t sustain_value;
    int16_t release_time;
} adsr_t;

int16_t osc_mul(int16_t a, int16_t b);
int16_t osc_generate(osc_t* osc, int32_t phase_shift);

int8_t osc_to_8bit(int16_t sample);

void adsr_update(osc_t* osc, adsr_t* adsr, int8_t gate);
