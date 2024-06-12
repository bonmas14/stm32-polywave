#include <stdint.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>

#include "encoder.h"

enum {
    IDLE,
    START_LEFT,
    START_RIGHT,
    STOP
};

volatile uint8_t state = STOP;
volatile uint8_t last_a_state = 0;
volatile uint8_t last_b_state = 0;
volatile uint16_t previous_check = 0;

volatile int32_t offset = 0;

void encoder_init(void) {
    gpio_set_mode(ENCODER_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, A_PIN | B_PIN);

    gpio_set(ENCODER_PORT, A_PIN | B_PIN);
}

int32_t encoder_state(void) {
    int32_t temp = offset;
    offset = 0;
    return temp;
}

void exti0_isr(void) {
    uint16_t gpio_state = gpio_port_read(GPIOB);
    
    uint16_t a_state = (gpio_state & A_PIN) ? 1 : 0;
    uint16_t b_state = (gpio_state & B_PIN) ? 1 : 0;

    switch (state) {
        case IDLE:
            if (!a_state && b_state) {
                state = START_LEFT;
            } else if (!b_state && a_state) {
                last_a_state = a_state;
                state = START_RIGHT;
            }
            break;
        case START_LEFT:
            gpio_set(GPIOC, GPIO13);

            if (a_state && !b_state) {
                offset--;
                state = STOP;
            }
            break;
        case START_RIGHT:
            gpio_clear(GPIOC, GPIO13);

            if (b_state && !a_state) {
                offset++;
                state = STOP;
            }
            break;
        case STOP:
            if (a_state && b_state) {
                state = IDLE;
            }
            break;
    }

    exti_reset_request(EXTI0);
}
