#include "encoder.h"

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>

volatile uint8_t last_state = 0;
volatile uint8_t turn_flag = 0;
volatile int32_t offset = 0;

void encoder_init(void) {
    gpio_set_mode(ENCODER_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, A_PIN | B_PIN | ENC_BUTTON);
    gpio_set(ENCODER_PORT, A_PIN | B_PIN | ENC_BUTTON);

    exti_select_source(EXTI0, ENCODER_PORT);
    exti_set_trigger(EXTI0, EXTI_TRIGGER_BOTH);
    exti_enable_request(EXTI0);

    nvic_enable_irq(NVIC_EXTI0_IRQ);
    nvic_set_priority(NVIC_EXTI0_IRQ, 0);
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

    if (a_state != last_state) {
        turn_flag = !turn_flag;
        if (turn_flag) // i actually can tell if it needs turn flag
            offset += (b_state != last_state) ? -1 : 1;
        last_state = a_state;
    }

    exti_reset_request(EXTI0);
}
