#pragma once

#include <stddef.h>
#include <stdint.h>

#include <libopencm3/stm32/gpio.h>


#define ENCODER_PORT GPIOB
#define A_PIN GPIO0
#define B_PIN GPIO1

void encoder_init(void); // we can change it to accept struct like encoder_t so we can have multiple encoder

int32_t encoder_state(void); // outputs direction of change
