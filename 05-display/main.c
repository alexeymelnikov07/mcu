#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task/protocol-task.h"
#include "led-task/led-task.h"
#include "ili9341-driver.h"
#include "hardware/spi.h"
#include "ili9341-display.h"
#include "ili9341-font.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

uint32_t global_variable = 0;

//const uint LED_PIN = 25;
const uint32_t constant_variable = 42;

//              ----------display----------
static ili9341_display_t ili9341_display = {0};
static ili9341_hal_t ili9341_hal = {0};

#define ILI9341_PIN_MISO 4
#define ILI9341_PIN_CS 10
#define ILI9341_PIN_SCK 6
#define ILI9341_PIN_MOSI 7
#define ILI9341_PIN_DC 8
#define ILI9341_PIN_RESET 9
// #define PIN_LED -> 3.3V

void rp2040_spi_write(const uint8_t *data, uint32_t size)
{
	spi_write_blocking(spi0, data, size);
}

void rp2040_spi_read(uint8_t *buffer, uint32_t length)
{
	spi_read_blocking(spi0, 0, buffer, length);
}

void rp2040_gpio_cs_write(bool level)
{
	gpio_put(ILI9341_PIN_CS, level);
}

void rp2040_gpio_dc_write(bool level)
{
	gpio_put(ILI9341_PIN_DC, level);
}

void rp2040_gpio_reset_write(bool level)
{
	gpio_put(ILI9341_PIN_RESET, level);
}

void rp2040_delay_ms(uint32_t ms)
{
	sleep_ms(ms);
}
//              ---------------------------

void version_callback(const char* args)
{
	printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

void led_on_callback(const char* args){
    led_task_state_set(LED_STATE_ON);
}
void led_off_callback(const char* args){
    led_task_state_set(LED_STATE_OFF);
}
void led_blink_callback(const char* args){
    led_task_state_set(LED_STATE_BLINK);
}
void led_blink_set_period_ms_callback(const char* args){
    uint32_t period_ms = 0;
    sscanf(args, "%u", &period_ms);
    led_task_set_blink_period_ms(period_ms);
}
void disp_screen_callback(const char* args)
{
	uint32_t c = 0;
	int result = sscanf(args, "%x", &c);
	
	uint16_t color = COLOR_BLACK;
	
	if (result == 1)
	{
		color = RGB888_2_RGB565(c);
	}
	
	ili9341_fill_screen(&ili9341_display, color);
}
void disp_px_callback(const char* args){
    uint16_t x= 0;
    uint16_t y= 0;
    uint32_t c= 0;
    sscanf(args, "%hu %hu %x", &x, &y, &c);
    if (x>320 || x<0 || y>240 || y<0){
        printf("Wrong size\n");
        return;
    }
    printf("x= %u, y= %u\n",x ,y);
    uint16_t color = RGB888_2_RGB565(c);
    ili9341_draw_pixel(&ili9341_display, x, y, color);
}
void disp_line_callback(const char* args){
    uint16_t x0=0;
    uint16_t x1=0;
    uint16_t y0=0;
    uint16_t y1=0;
    uint32_t c=0;
    sscanf(args, "%hu %hu %hu %hu %x", &x0, &y0, &x1, &y1, &c);
    uint16_t color = RGB888_2_RGB565(c);
    ili9341_draw_line(&ili9341_display, x0, y0, x1, y1, color);
}
void disp_rect_callback(const char* args){
    uint16_t x0=0;
    uint16_t y0=0;
    uint16_t w=0;
    uint16_t h=0;
    uint32_t c=0;
    sscanf(args, "%hu %hu %hu %hu %x", &x0, &y0, &w, &h, &c);
    uint16_t color = RGB888_2_RGB565(c);
    ili9341_draw_rect(&ili9341_display, x0, y0, w, h, color);
}
void disp_frect_callback(const char* args){
    uint16_t x0=0;
    uint16_t y0=0;
    uint16_t w=0;
    uint16_t h=0;
    uint32_t c=0;
    sscanf(args, "%hu %hu %hu %hu %x", &x0, &y0, &w, &h, &c);
    uint16_t color = RGB888_2_RGB565(c);
    ili9341_draw_filled_rect(&ili9341_display, x0, y0, w, h, color);
}
void disp_text_callback(const char* args){
    uint16_t x=0;
    uint16_t y=0;
    uint32_t c=0;
    uint32_t bc=0;
    char text[64] ={0};
    sscanf(args, "%u %u %63s %x %x", &x, &y, &text, &c, &bc);
    uint16_t color = RGB888_2_RGB565(c);
    uint16_t bg_color = RGB888_2_RGB565(bc);
    ili9341_draw_text(&ili9341_display, x, y, &text, &jetbrains_font, color, bg_color);
}

api_t device_api[] =
{
	{"version", version_callback, "get device name and firmware version"},
    {"on",led_on_callback,"Turn on led"},
    {"off",led_off_callback,"Turn off led"},
    {"blink",led_blink_callback,"Led starts blinking"},
    {"set_period", led_blink_set_period_ms_callback, "Set period of blinking"},
    {"disp_screen", disp_screen_callback, "Fill screen in one color"},
    {"disp_px", disp_px_callback , "Draw single pixel with 'random' color"},
    {"disp_line", disp_line_callback, ""},
    {"disp_rect", disp_rect_callback, ""},
    {"disp_frect", disp_frect_callback, ""},
    {"disp_text", disp_text_callback, ""},
	{NULL, NULL, NULL},
};

int main(){
    stdio_init_all();
    stdio_task_init();
    protocol_task_init(device_api);
    led_task_init();

    // sleep_ms(10000);
    // printf("start init display\n");
    
    spi_init(spi0, 62500000);
    gpio_set_function(ILI9341_PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_SCK, GPIO_FUNC_SPI);

    gpio_init(ILI9341_PIN_CS);
    gpio_set_dir(ILI9341_PIN_CS, true);
    gpio_put(ILI9341_PIN_CS, true );
    gpio_init(ILI9341_PIN_DC);
    gpio_set_dir(ILI9341_PIN_DC, true);
    gpio_put(ILI9341_PIN_DC, false);
    gpio_init(ILI9341_PIN_RESET);
    gpio_set_dir(ILI9341_PIN_RESET, true);
    gpio_put(ILI9341_PIN_RESET, false);

    ili9341_hal.spi_write = rp2040_spi_write;
    ili9341_hal.spi_read = rp2040_spi_read;
    ili9341_hal.gpio_cs_write = rp2040_gpio_cs_write;
    ili9341_hal.gpio_dc_write = rp2040_gpio_dc_write;
    ili9341_hal.gpio_reset_write = rp2040_gpio_reset_write;
    ili9341_hal.delay_ms = rp2040_delay_ms;

    ili9341_init(&ili9341_display, &ili9341_hal);
    ili9341_set_rotation(&ili9341_display, ILI9341_ROTATION_90);

    // printf("start drawing\n");
    // //      -----drawing-----
    
    ili9341_fill_screen(&ili9341_display, COLOR_BLACK);
    sleep_ms(300);
    ili9341_draw_filled_rect(&ili9341_display, 10, 10, 100, 60, COLOR_RED);
    ili9341_draw_filled_rect(&ili9341_display, 120, 10, 100, 60, COLOR_GREEN);
    ili9341_draw_filled_rect(&ili9341_display, 230, 10, 80, 60, COLOR_BLUE);
    ili9341_draw_rect(&ili9341_display, 10, 90, 300, 80, COLOR_WHITE);
    ili9341_draw_line(&ili9341_display, 0, 0, 319, 239, COLOR_YELLOW);
    ili9341_draw_line(&ili9341_display, 319, 0, 0, 239, COLOR_CYAN);
    ili9341_draw_text(&ili9341_display, 20, 100, "Hello, ILI9341!", &jetbrains_font, COLOR_WHITE, COLOR_BLACK);
    ili9341_draw_text(&ili9341_display, 20, 116, "RP2040 / Pico SDK", &jetbrains_font, COLOR_YELLOW, COLOR_BLACK);
    printf("End");
    while(1){
        protocol_task_handle(stdio_task_handle());
        led_task_handle();
    }
}