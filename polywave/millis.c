#include "millis.h"

#include "libopencm3/cm3/nvic.h"
#include <libopencm3/cm3/systick.h>

volatile uint32_t counter;

void millis_init(void) {
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
    systick_set_reload(8999);
    systick_interrupt_enable();
    systick_counter_enable();
}

void sys_tick_handler(void) {
    counter++;
}

uint32_t millis(void) {
    return counter;
}
