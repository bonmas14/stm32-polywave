#pragma once

#include <stddef.h>
#include <stdint.h>

enum {
    TRIANGLE,
    SAW,
    SQUARE
};

typedef struct {
    uint8_t osc_enum;
    int64_t time;
    int32_t freq;
} osc_t;

int32_t osc_generate(osc_t* osc, int32_t phase_shift);
