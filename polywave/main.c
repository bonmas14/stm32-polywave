#include <stddef.h>
#include <stdint.h>

// todo: add tim2 as irq instead of tim3 and sync them
// not working because isr is too long?
// 

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#define FREQ_ARR (300 - 1)
#define OUT_OFFSET ((FREQ_ARR - 255) / 2)

#define BUFF_SIZE 512

void init_mcu(void);
void init_timers(void);

size_t buff_index = 0;
uint8_t buffer[BUFF_SIZE];

int main(void) {
    for (size_t i = 0; i < BUFF_SIZE; i++) {
        buffer[i] = i / 2;
    }

    init_mcu();

    while (1) {
        for (size_t i = 0; i < (18000000) / (255); i++) {
            __asm__("nop");
        }

        TIM3_CCR1 = buffer[buff_index++] + OUT_OFFSET;  
        if (buff_index >= BUFF_SIZE)
            buff_index = 0;
        gpio_toggle(GPIOC, GPIO13);
    }
}

void init_mcu(void) {
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);

    rcc_periph_clock_enable(RCC_TIM3);
    rcc_periph_clock_enable(RCC_AFIO);
    rcc_periph_clock_enable(RCC_GPIOA); 
    rcc_periph_clock_enable(RCC_GPIOC);

    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

    //nvic_enable_irq(NVIC_TIM3_IRQ);
    //nvic_set_priority(NVIC_TIM3_IRQ, 1);

    init_timers();
}

void init_timers(void) {
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_TIM3_CH1);

    TIM3_CR1 = TIM_CR1_CKD_CK_INT | TIM_CR1_CMS_EDGE;
    TIM3_CNT = 0;
    TIM3_ARR = FREQ_ARR; 
    TIM3_PSC = 4; // 4 = 48khz 9 = 24khz (5, 10)
    TIM3_CCR1 = 127 + OUT_OFFSET;

    TIM3_EGR = TIM_EGR_UG;
    TIM3_CCMR1 |= TIM_CCMR1_OC1M_PWM1 | TIM_CCMR1_OC1PE;
    TIM3_CCER |= TIM_CCER_CC1P | TIM_CCER_CC1E;

    TIM3_CR1 |= TIM_CR1_ARPE;
    TIM3_CR1 &= ~TIM_CR1_OPM;

    //TIM3_DIER |= TIM_DIER_UIE; // do this with TIM2 and sync them togethe
    
    TIM3_CR1 |= TIM_CR1_CEN;
}

