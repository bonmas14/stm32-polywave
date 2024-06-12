#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>

#include "oled.h"

uint8_t oled_buffer[SSD1306_BUFFER_SIZE];

enum ssd1306_ctlbyte {
    LongCMD = 0b00000000,
    LongDAT = 0b01000000,
    ShortCMD = 0b10000000,
    ShortDAT = 0b11000000,
};

void ssd1306_init(void);

void oled_init(void) {
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN, 
            GPIO_I2C1_RE_SCL | GPIO_I2C1_RE_SDA);

    gpio_primary_remap(0, AFIO_MAPR_I2C1_REMAP);

    i2c_peripheral_disable(I2C1);

    i2c_set_speed(I2C1, i2c_speed_fm_400k, rcc_apb1_frequency / 1000000);

    i2c_peripheral_enable(I2C1);

    ssd1306_init();
}

void ssd1306_init(void) {
    i2c_transfer7(I2C1, OLED_ADDR, (uint8_t[]){ ShortCMD, 0xAE }, 2, NULL, 0);
    i2c_transfer7(I2C1, OLED_ADDR, (uint8_t[]){ LongCMD, 0x20, 0x02 }, 3, NULL, 0);
    i2c_transfer7(I2C1, OLED_ADDR, (uint8_t[]){ ShortCMD, 0xB0 }, 2, NULL, 0);

    i2c_transfer7(I2C1, OLED_ADDR, (uint8_t[]){ LongCMD, 0xA8, 0x3F }, 3, NULL, 0);
    i2c_transfer7(I2C1, OLED_ADDR, (uint8_t[]){ LongCMD, 0xD3, 0x00 }, 3, NULL, 0);
    i2c_transfer7(I2C1, OLED_ADDR, (uint8_t[]){ ShortCMD, 0x40, ShortCMD, 0xA0, ShortCMD, 0xC0 }, 8, NULL, 0);
    i2c_transfer7(I2C1, OLED_ADDR, (uint8_t[]){ LongCMD, 0xDA, 0x12 }, 3, NULL, 0);
    i2c_transfer7(I2C1, OLED_ADDR, (uint8_t[]){ LongCMD, 0x81, 0x7F }, 3, NULL, 0);
    i2c_transfer7(I2C1, OLED_ADDR, (uint8_t[]){ ShortCMD, 0xA4, ShortCMD, 0xA6 }, 4, NULL, 0);
    i2c_transfer7(I2C1, OLED_ADDR, (uint8_t[]){ LongCMD, 0xD5, 0x80 }, 3, NULL, 0);

    i2c_transfer7(I2C1, OLED_ADDR, (uint8_t[]){ LongCMD, 0xDB, 0x20 }, 3, NULL, 0);
    i2c_transfer7(I2C1, OLED_ADDR, (uint8_t[]){ LongCMD, 0xD9, 0x22 }, 3, NULL, 0);
    i2c_transfer7(I2C1, OLED_ADDR, (uint8_t[]){ LongCMD, 0x8D, 0x14 }, 3, NULL, 0);
    i2c_transfer7(I2C1, OLED_ADDR, (uint8_t[]){ ShortCMD, 0xAF }, 2, NULL, 0);

    oled_update();
}

static void ssd1306_send_page(size_t page) {
    while ((I2C_SR2(I2C1) & I2C_SR2_BUSY)) { }

    i2c_send_start(I2C1);

    while ( !( (I2C_SR1(I2C1) & I2C_SR1_SB)
                && (I2C_SR2(I2C1) & I2C_SR2_MSL)
                && (I2C_SR2(I2C1) & I2C_SR2_BUSY) ));

    i2c_send_7bit_address(I2C1, OLED_ADDR, I2C_WRITE);
    while (!(I2C_SR1(I2C1) & I2C_SR1_ADDR));
    (void)I2C_SR2(I2C1);

    i2c_send_data(I2C1, LongDAT);

    for (size_t i = 0; i < SSD1306_WIDTH; i++) {
        i2c_send_data(I2C1, oled_buffer[i + page * SSD1306_WIDTH]);
        while (!(I2C_SR1(I2C1) & (I2C_SR1_BTF)));
    }

    i2c_send_stop(I2C1);
}

void oled_update(void) {
    i2c_transfer7(I2C1, OLED_ADDR, (uint8_t[]){ LongCMD, 0xD3, 0x00 }, 3, NULL, 0);

    for (uint8_t page = 0; page < 8; page++) {
        i2c_transfer7(I2C1, OLED_ADDR, (uint8_t[]){ ShortCMD, 0xB0 + page }, 2, NULL, 0);
        i2c_transfer7(I2C1, OLED_ADDR, (uint8_t[]){ ShortCMD, 0x00 }, 2, NULL, 0);
        i2c_transfer7(I2C1, OLED_ADDR, (uint8_t[]){ ShortCMD, 0x10 }, 2, NULL, 0);
        ssd1306_send_page(page);
    }
}

void oled_clear(void) {
    for (size_t i = 0; i < SSD1306_BUFFER_SIZE; i++) {
        oled_buffer[i] = 0x00;
    }
}

void oled_set_pixel(size_t x, size_t y, uint8_t value) {
    uint8_t output = oled_buffer[x + (y / 8) * SSD1306_WIDTH];
    uint8_t bit = 1 << (y % 8);

    if (value)
        output |= bit;
    else
        output &= ~bit;

    oled_buffer[x + (y / 8) * SSD1306_WIDTH] = output;
}

void oled_write_rle(const uint8_t *data) {
    oled_clear();

    size_t offset = 0;
    uint8_t color = 0;

    size_t counter = data[offset++];

    for (size_t y = 0; y < SSD1306_HEIGHT; y++) {
        for (size_t x = 0; x < SSD1306_WIDTH; x++) {
            if (counter == 0) {
                color = color ? 0 : 1;
                counter = data[offset++];
            }

            if (counter == 0) {
                x--;
                continue;
            }

            uint8_t temp = color << (y % 8);

            oled_buffer[x + (y / 8) * SSD1306_WIDTH] |= temp;
            counter--;
        }
    }
}
