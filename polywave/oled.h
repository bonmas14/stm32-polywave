#pragma once
#include <stddef.h>
#include <stdint.h>

#define OLED_ADDR 0x3c
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64
#define SSD1306_BUFFER_SIZE (SSD1306_WIDTH * SSD1306_HEIGHT / 8)

void oled_init(void);
void oled_update(void);
void oled_clear(void);

void oled_set_pixel(size_t x, size_t y, uint8_t value);
void oled_write_rle(const uint8_t* data);
