#include <stddef.h>
#include <stdint.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

void init_mcu(void);
void init_carrier(void);
void init_sampler(void);

int main(void) {
    init_mcu();

    size_t j = 0;
    int8_t d = 1;

    while (1) {
        if (j == 256)
            d = -1;
        else if (j == 0)
            d = 1;

        j += d;

        TIM3_CCR1 = j;

        gpio_toggle(GPIOC, GPIO13);

        for (size_t i = 0; i < 180000; i++) {
            __asm__("nop");
        }
    }

}

void init_mcu(void) {
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);

    rcc_periph_clock_enable(RCC_TIM3);
    rcc_periph_clock_enable(RCC_AFIO);
    rcc_periph_clock_enable(RCC_GPIOA); 
    rcc_periph_clock_enable(RCC_GPIOC);

    init_carrier();

    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
    
}


void init_carrier(void) {
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_TIM3_CH1);

    TIM3_CR1 = TIM_CR1_CKD_CK_INT | TIM_CR1_CMS_EDGE;
    TIM3_ARR = (1 << 8) - 1;

    TIM3_PSC = 0;
    TIM3_EGR = TIM_EGR_UG;

    TIM3_CCMR1 |= TIM_CCMR1_OC1M_PWM1 | TIM_CCMR1_OC1PE;
    TIM3_CCER |= TIM_CCER_CC1P | TIM_CCER_CC1E;

    TIM3_CCR1 = 0;

    TIM3_CR1 |= TIM_CR1_ARPE;
    TIM3_CR1 |= TIM_CR1_CEN;
}

void init_sampler(void) {

}


// 1 таймер - генерация несущего сигнала
// 2 таймер - генерация и микширование "аналоговых" сигналов
//  прерывание на таймер 48кГц
//
// +midi event
