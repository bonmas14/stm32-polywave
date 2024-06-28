#include <stddef.h>
#include <stdint.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#include "libopencm3/stm32/f1/gpio.h"
#include "logo.h"
#include "sound.h"
//#include "oled.h"  // we dont need oled right now (pins B8 B9)
#include "encoder.h"
#include "millis.h"
#include "utils.h"

#include "notes.h"

#define PWM_ARR 255 // 255 = ~250khz 127 = ~600khz
#define PWM_IDLE (PWM_ARR / 2)

#define SAMPLE_ARR 1500
#define SAMPLE_FREQ (72000000 / SAMPLE_ARR)

#define BUFF_SIZE 512

void init_mcu(void);
void init_timers(void);
void run_timers(void);

volatile size_t read_index = 0;
volatile bool sample_filled = false;

uint8_t output_buffer[BUFF_SIZE];
uint8_t sync_buffer[BUFF_SIZE];

osc_t car = (osc_t) { .freq = NOTE_C2, .osc_enum = OSC_TRIANGLE };
osc_t mod = (osc_t) { .freq = NOTE_C2, .osc_enum = OSC_TRIANGLE };
osc_t amp = (osc_t) { .freq = NOTE_C4, .osc_enum = OSC_TRIANGLE };

adsr_t base = (adsr_t) { .state_enum = ADSR_IDLE, .attack_time = 240, .attack_value = 24000, .decay_time = 6000, .sustain_value = 6000, .release_time = 4800 };

int main(void) {
    init_mcu();

    //oled_write_rle(logo_rle);
    //oled_update();

    int32_t offset = 0;
    int32_t gate = 0;

    while (1) {
        offset = offset + encoder_state() * 1;
        
        car.freq = NOTE_C2 + offset;
        mod.freq = car.freq * 5;
        amp.freq = car.freq * 2;

        if (sample_filled) {
            continue;
        }

        for (size_t i = 0; i < BUFF_SIZE; i++) {
            gate = (car.time % 12000) < 3000 ? 1 : 0;

            adsr_update(&car, &base, gate);

            // FM
            int16_t sample = osc_generate(&car, osc_generate(&mod, 10) / 250);
            // AM
            sample = osc_mul(sample, osc_generate(&amp, 1000));
            // ADSR
            sample = osc_mul(sample, base.current_value);

            sync_buffer[i] = max(1, min(osc_to_8bit(sample) + PWM_IDLE, 254));
        }

        sample_filled = true;
    }
}

void run_timers(void) {
    TIM2_CR1 |= TIM_CR1_CEN;
    TIM3_CR1 |= TIM_CR1_CEN; 

    nvic_enable_irq(NVIC_TIM2_IRQ);
    nvic_set_priority(NVIC_TIM2_IRQ, 0);
}

void init_mcu(void) {
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
    millis_init();

    rcc_periph_clock_enable(RCC_I2C1);
    rcc_periph_clock_enable(RCC_TIM2);
    rcc_periph_clock_enable(RCC_TIM3);

    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC); 

    rcc_periph_clock_enable(RCC_AFIO);

    init_timers();
    encoder_init();
    //oled_init();


    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

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
