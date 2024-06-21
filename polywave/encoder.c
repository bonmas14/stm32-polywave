#include "encoder.h"
#include <stdint.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>

#include "millis.h"

#define ANTISHUTTER_TIME 10

enum {
    IDLE,
    START_LEFT,
    START_RIGHT,
};

volatile uint8_t state = IDLE;
volatile int32_t offset = 0;
volatile int32_t anti_shutter_timer = 0;

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

    // invert because they on pullups
    uint16_t a_state = (gpio_state & A_PIN) ? 0 : 1;
    uint16_t b_state = (gpio_state & B_PIN) ? 0 : 1;

    if ((millis() - (uint32_t)anti_shutter_timer) < ANTISHUTTER_TIME) {
        exti_reset_request(EXTI0);
        return;
    }

    switch (state) {
        case IDLE:
            if (a_state && !b_state) {
                anti_shutter_timer = millis();
                state = START_LEFT;
            } else if (b_state && !a_state) {
                anti_shutter_timer = millis();
                state = START_RIGHT;
            }
            break;
        case START_LEFT:
            gpio_set(GPIOC, GPIO13);

            if (b_state && a_state) {
                anti_shutter_timer = millis();
                offset--;
                state = IDLE;
            }
            break;
        case START_RIGHT:
            gpio_clear(GPIOC, GPIO13);

            if (a_state && b_state) {
                anti_shutter_timer = millis();
                offset++;
                state = IDLE;
            }
            break;
    }

    exti_reset_request(EXTI0);
}
