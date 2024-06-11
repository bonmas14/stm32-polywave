#include <stddef.h>
#include <stdint.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#define FREQ_ARR 127

#define BUFF_SIZE 512

void init_mcu(void);
void init_timers(void);

size_t buff_index = 0;
uint8_t buffer[BUFF_SIZE];


int main(void) {
    for (size_t i = 0; i < BUFF_SIZE; i++) {
        buffer[i] = i / 4;
    }

    init_mcu();

    TIM2_CR1 |= TIM_CR1_CEN;
    while (1) {
        //TIM3_CCR1 = buffer[buff_index++];

        //if (buff_index >= BUFF_SIZE)
         //   buff_index = 0;
    }
}

void init_mcu(void) {
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);

    rcc_periph_clock_enable(RCC_TIM2);
    rcc_periph_clock_enable(RCC_TIM3);
    rcc_periph_clock_enable(RCC_AFIO);
    rcc_periph_clock_enable(RCC_GPIOA); 

    nvic_enable_irq(NVIC_TIM2_IRQ);
    nvic_set_priority(NVIC_TIM2_IRQ, 1);

    init_timers();
}

void init_timers(void) {
    // data WE NEED DMA
    TIM2_CNT = 1;
    TIM2_PSC = 0; 
    TIM2_ARR = 1500;
    TIM2_DIER |= TIM_DIER_UIE;

    // pwm
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_TIM3_CH1);

    TIM3_CR1 = TIM_CR1_CKD_CK_INT_MUL_2 | TIM_CR1_CMS_EDGE;
    TIM3_CNT = 0;
    TIM3_ARR = FREQ_ARR; 
    TIM3_PSC = 0; 
    TIM3_CCR1 = 63;

    TIM3_EGR = TIM_EGR_UG;

    TIM3_CCMR1 |= TIM_CCMR1_OC1M_PWM1 | TIM_CCMR1_OC1PE;
    TIM3_CCER |= TIM_CCER_CC1P | TIM_CCER_CC1E;

    TIM3_CR1 |= TIM_CR1_ARPE;
    TIM3_CR1 &= ~TIM_CR1_OPM;

    TIM3_CR1 |= TIM_CR1_CEN; 
}

void tim2_isr(void) {
    TIM3_CCR1 = buffer[buff_index++] / 2;

    if (buff_index >= BUFF_SIZE)
        buff_index = 0;

    TIM2_SR &= ~TIM_SR_UIF;
}
