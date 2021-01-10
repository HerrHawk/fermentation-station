#include "renderer.h"
#include "../globals.h"
#include "../graphics/images.h"
#include "../interfaces/spi.h"
#include "../logging.h"
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <util/delay.h>

// PORTS on the display
#define RST_PIN PB0
#define DC_PIN PB1
#define CS_PIN PB2
#define BUSY_PIN PD6

#define BAUD 9600

// define display commands p. 12 - 16
#define DRIVER_OUTPUT_CONTROL 0x01
#define BOOSTER_SOFT_START_CONTROL 0x0C
#define GATE_SCAN_START_POSITION 0x0F
#define DEEP_SLEEP_MODE 0x10
#define DATA_ENTRY_MODE_SETTING 0x11
#define SW_RESET 0x12
#define TEMPERATURE_SENSOR_CONTROL 0x1A
#define MASTER_ACTIVATION 0x20
#define DISPLAY_UPDATE_CONTROL_1 0x21
#define DISPLAY_UPDATE_CONTROL_2 0x22
#define WRITE_RAM 0x24
#define WRITE_VCOM_REGISTER 0x2C
#define WRITE_LUT_REGISTER 0x32
#define SET_DUMMY_LINE_PERIOD 0x3A
#define SET_GATE_TIME 0x3B
#define BORDER_WAVEFORM_CONTROL 0x3C
#define SET_RAM_X_ADDRESS_START_END_POSITION 0x44
#define SET_RAM_Y_ADDRESS_START_END_POSITION 0x45
#define SET_RAM_X_ADDRESS_COUNTER 0x4E
#define SET_RAM_Y_ADDRESS_COUNTER 0x4F
#define TERMINATE_FRAME_READ_WRITE 0xFF

unsigned char image[1024];

// required for waveform
const unsigned char lut_full_update[] = { 0x50, 0xAA, 0x55, 0xAA, 0x11, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x1F, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

const unsigned char lut_partial_update[] = { 0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00,
                                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                             0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12,
                                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

inline void display_wait_until_idle()
{
  while (PORTD & _BV(BUSY_PIN)) {
    // TODO: remove this once we are certain that it works!
    LOG_DEBUG(DISPLAY, "display busy");
    _delay_ms(100);
  };
}

void display_send_command(uint8_t command)
{
  // may not send commands while display is busy
  display_wait_until_idle();

  // make display interpret data as command
  // by setting the Data/Command pin to low
  // high pin would lead to interpretation as data
  PORTB &= ~_BV(DC_PIN);

  // send command
  LOG_DEBUG(DISPLAY, "send command %x to display", command);
  spi_main_transmit(command);
}

void display_send_data(uint8_t data)
{
  // may not send data while display is busy
  display_wait_until_idle();

  // make display interpret data as command
  // by setting the Data/Command pin to low
  // low pin would lead to interpretation as command
  PORTB |= _BV(DC_PIN);

  // send data
  spi_main_transmit(data);
}

void display_reset(void)
{
  PORTB &= ~_BV(RST_PIN);
  _delay_ms(200);
  PORTB |= _BV(RST_PIN);
  _delay_ms(200);
}

void display_set_lookup_table(const unsigned char* lut)
{
  display_send_command(WRITE_LUT_REGISTER);
  // look up table is 30 bytes long
  for (int i = 0; i < 30; i++) {
    display_send_data(lut[i]);
  }
}

void display_set_partial_frame_memory(const unsigned char* image_buffer,
                                      int x,
                                      int y,
                                      int image_width,
                                      int image_height)
{
  // only continue if action is valid
  if (image_buffer == NULL || x < 0 || y < 0 || image_width < 1 || image_height < 1) {
    return;
  }
  display_set_lookup_table(lut_partial_update);
  // x must be the multiple of 8 or the last 3 bits will be ignored
  x &= 0xF8;
  image_width &= 0xF8;

  // determine the end index of the image
  int x_end;
  int y_end;
  if (x + image_width >= DISPLAY_WIDTH) {
    x_end = DISPLAY_WIDTH - 1;
  } else {
    x_end = x + image_width - 1;
  }
  if (y + image_height >= DISPLAY_HEIGHT) {
    y_end = DISPLAY_HEIGHT - 1;
  } else {
    y_end = y + image_height - 1;
  }

  // prepare send to display
  display_set_memory_area(x, y, x_end, y_end);
  display_set_memory_pointer(x, y);
  display_send_command(WRITE_RAM);

  // send byte by byte
  for (int j = 0; j < y_end - y + 1; j++) {
    for (int i = 0; i < (x_end - x + 1) / 8; i++) {
      int index = i + j * (image_width / 8);
      int data = pgm_read_byte(&image_buffer[index]);
      display_send_data(data);
    }
  }
}

void display_set_entire_frame_memory(const unsigned char* image_buffer)
{
  display_set_lookup_table(lut_full_update);
  display_set_memory_area(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
  display_set_memory_pointer(0, 0);
  display_send_command(WRITE_RAM);
  for (int i = 0; i < DISPLAY_WIDTH / 8 * DISPLAY_HEIGHT; i++) {
    display_send_data(pgm_read_byte(&image_buffer[i]));
  }
}

void display_clear_frame_memory(unsigned char color)
{
  // set memory area
  display_set_memory_area(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
  // set memory pointer
  display_set_memory_pointer(0, 0);
  // send command write ram
  display_send_command(WRITE_RAM);
  // send color to display
  for (int i = 0; i < DISPLAY_WIDTH / 8 * DISPLAY_HEIGHT; i++) {
    display_send_data(color);
  }
}

void display_render_frame(void)
{
  display_send_command(DISPLAY_UPDATE_CONTROL_2);
  /*
  // These are the parameters that work
  0xC4 => 11000100 (waveshare)
  0xC7 => 11000111 (github)

  // Bit Meanings
  7 - enable Clock Signal
  6 - enable CP (CP = Charge Pump)
  5 - ??? (likely load temperature)
  4 - ??? (likely load lut)
  3 - initial display
  2 - display pattern
  1 - disable CP (CP = Charge Pump)
  0 - disable clock signal
  */
  display_send_data(0xC4);
  display_send_command(MASTER_ACTIVATION);
  display_send_command(TERMINATE_FRAME_READ_WRITE);
  display_wait_until_idle();
}

void display_set_memory_area(int x_start, int y_start, int x_end, int y_end)
{
  LOG_DEBUG(DISPLAY, "set display area to x(%d - %d) / y(%d - %d)", x_start, x_end, y_start, y_end);
  display_send_command(SET_RAM_X_ADDRESS_START_END_POSITION);
  // TODO: multiple of 8??
  display_send_data((x_start >> 3) & 0xFF);
  display_send_data((x_end >> 3) & 0xFF);
  display_send_command(SET_RAM_Y_ADDRESS_START_END_POSITION);
  display_send_data(y_start & 0xFF);
  display_send_data((y_start >> 8) & 0xFF);
  display_send_data(y_end & 0xFF);
  display_send_data((y_end >> 8) & 0xFF);
}

void display_set_memory_pointer(int x, int y)
{
  display_send_command(SET_RAM_X_ADDRESS_COUNTER);
  display_send_data((x >> 3) & 0xFF);
  display_send_command(SET_RAM_Y_ADDRESS_COUNTER);
  display_send_data(y & 0xFF);
  display_send_data((y >> 8) & 0xFF);
  display_wait_until_idle();
}

void display_wipe(void)
{
  display_clear_frame_memory(0xFF);
  display_render_frame();
  display_clear_frame_memory(0xFF);
  display_render_frame();
  display_set_entire_frame_memory(dickbutt);
  display_render_frame();
  display_set_entire_frame_memory(dickbutt);
  display_render_frame();
  _delay_ms(3000);
}

void display_sleep(void) {}

void render_image(unsigned char image[]) {}

void clear_display() {}

void display_init()
{
  // Set CS, DC and RST Pins to OUTPUT
  DDRB |= _BV(CS_PIN) | _BV(DC_PIN);
  DDRB |= _BV(RST_PIN);
  // Set BUSY Pin to INPUT
  DDRD &= ~_BV(BUSY_PIN);

  // Initialize SPI Connection
  spi_init();

  // reset display to get a clean start
  display_reset();

  //
  LOG_DEBUG(DISPLAY, "DRIVER_OUTPUT_CONTROL");
  display_send_command(DRIVER_OUTPUT_CONTROL);
  display_send_data((DISPLAY_HEIGHT - 1) & 0xFF);
  display_send_data(((DISPLAY_HEIGHT - 1) >> 8) & 0xFF);
  display_send_data(0x00);

  //
  LOG_DEBUG(DISPLAY, "BOOSTER_SOFT_START_CONTROL");
  display_send_command(BOOSTER_SOFT_START_CONTROL);
  display_send_data(0xD7);
  display_send_data(0xD6);
  display_send_data(0x9D);

  //
  LOG_DEBUG(DISPLAY, "WRITE_VCOM_REGISTER");
  display_send_command(WRITE_VCOM_REGISTER);
  display_send_data(0xA8);

  //
  LOG_DEBUG(DISPLAY, "SET_DUMMY_LINE_PERIOD");
  display_send_command(SET_DUMMY_LINE_PERIOD);
  display_send_data(0x1A);

  //
  LOG_DEBUG(DISPLAY, "SET_GATE_TIME");
  display_send_command(SET_GATE_TIME);
  display_send_data(0x08);

  //
  LOG_DEBUG(DISPLAY, "DATA_ENTRY_MODE_SETTING");
  display_send_command(DATA_ENTRY_MODE_SETTING);
  display_send_data(0x03);

  //
  display_set_lookup_table(lut_full_update);

  //
  display_wipe();
}