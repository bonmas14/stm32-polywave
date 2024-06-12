#include "encoder.h"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <stdint.h>

volatile uint8_t last_state = 0;

volatile int32_t offset = 0;

void encoder_init(void) {
    gpio_set_mode(ENCODER_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, A_PIN | B_PIN);

    gpio_set(ENCODER_PORT, A_PIN | B_PIN);
}

int32_t encoder_state(void) {
    return offset;
}

void exti0_isr(void) {
    uint16_t state = (gpio_port_read(GPIOB) & A_PIN) ? 1 : 0;

    gpio_toggle(GPIOC, GPIO13);

    if (state != last_state) {
        uint8_t b_state = (state | B_PIN) ? 1 : 0;

        offset += (b_state != last_state) ? -1 : 1;

        last_state = state;
    }

    exti_reset_request(EXTI0);
}
