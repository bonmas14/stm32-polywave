#include <stddef.h>
#include <stdint.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#include "logo.h"
#include "sound.h"
#include "oled.h"

#define PWM_ARR 255 // 255 = ~250khz 127 = ~600khz
#define PWM_IDLE (PWM_ARR / 2)

#define SAMPLE_ARR 1500
#define SAMPLE_FREQ (72000000 / SAMPLE_ARR)

#define BUFF_SIZE 512

void init_mcu(void);
void init_timers(void);
void run_timers(void);

uint64_t sample_time = 0;

size_t current_page = 0;

size_t read_index = 0;
bool sample_filled = false;

uint8_t output_buffer[BUFF_SIZE];
uint8_t sync_buffer[BUFF_SIZE];

int main(void) {
    init_mcu();

    oled_set_pixel(0, 0, 1);
    oled_set_pixel(1, 1, 1);
    oled_set_pixel(2, 2, 1);
    oled_set_pixel(3, 3, 1);
    oled_set_pixel(4, 4, 1);
    oled_set_pixel(5, 5, 1);

    oled_write_rle(logo_rle);

    oled_update();

    while (1) {
        if (sample_filled) {
            gpio_set(GPIOC, GPIO13);
            continue;
        }

        gpio_clear(GPIOC, GPIO13);

        for (size_t i = 0; i < BUFF_SIZE; i++, sample_time++) {
            sync_buffer[i] = osc_generate(SAW, 523, sample_time) / 3 + osc_generate(SAW, 659, sample_time) / 3 + osc_generate(SAW, 987, sample_time) / 3;
        }

        sample_filled = true;
    }
}

void run_timers(void) {
    TIM2_CR1 |= TIM_CR1_CEN;
    TIM3_CR1 |= TIM_CR1_CEN; 

    nvic_enable_irq(NVIC_TIM2_IRQ);
    nvic_set_priority(NVIC_TIM2_IRQ, 1);

    nvic_enable_irq(NVIC_TIM4_IRQ);
    nvic_set_priority(NVIC_TIM4_IRQ, 1);
}

void init_mcu(void) {
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);

    rcc_periph_clock_enable(RCC_TIM2);
    rcc_periph_clock_enable(RCC_TIM3);
    rcc_periph_clock_enable(RCC_AFIO);
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOC); 

    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

    oled_init();

    init_timers();
    run_timers();
}

void init_timers(void) {
    // data WE NEED DMA
    TIM2_CNT = 1;
    TIM2_PSC = 0; 
    TIM2_ARR = SAMPLE_ARR;
    TIM2_DIER |= TIM_DIER_UIE;

    // pwm
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_TIM3_CH1);

    TIM3_CR1 = TIM_CR1_CKD_CK_INT_MUL_2 | TIM_CR1_CMS_EDGE;
    TIM3_CNT = 0;
    TIM3_ARR = PWM_ARR; 
    TIM3_PSC = 0; 
    TIM3_CCR1 = PWM_IDLE;

    TIM3_EGR = TIM_EGR_UG;

    TIM3_CCMR1 |= TIM_CCMR1_OC1M_PWM1 | TIM_CCMR1_OC1PE;
    TIM3_CCER |= TIM_CCER_CC1P | TIM_CCER_CC1E;

    TIM3_CR1 |= TIM_CR1_ARPE;
    TIM3_CR1 &= ~TIM_CR1_OPM;

}

void tim2_isr(void) {
    if (read_index >= BUFF_SIZE) {
        read_index = 0;

        if (sample_filled) {
            for (size_t i = 0; i < BUFF_SIZE; i++) {
                output_buffer[i] = sync_buffer[i];
            }

            sample_filled = false;
        } else {
            TIM3_CCR1 = PWM_IDLE;
            TIM2_SR &= ~TIM_SR_UIF;
            return;
        }
    }

    TIM3_CCR1 = output_buffer[read_index++];
    TIM2_SR &= ~TIM_SR_UIF;
}
