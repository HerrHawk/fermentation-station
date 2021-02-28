#include "renderer.h"
#include "../globals.h"
#include "../graphics/font.h"
#include "../graphics/images.h"
#include "../interfaces/spi.h"
#include "../logging.h"
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

/**
 * The information for this part of the program was taken mainly from three sources:
 *  - datasheet of the display
 * https://www.waveshare.com/w/upload/9/98/2.9inch-e-paper-module-user-manual-en.pdf
 *  - datasheet of the display controller
 * https://www.smart-prototyping.com/image/data/9_Modules/EinkDisplay/GDE029A1/IL3820.pdf
 *  - the official code examples by waveshare: https://github.com/waveshare/e-Paper
 */

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

// lookuptables to be used by the display for correctly accessing and modifying its memory
const unsigned char lut_full_update[] = { 0x50, 0xAA, 0x55, 0xAA, 0x11, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x1F, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

const unsigned char lut_partial_update[] = { 0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00,
                                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                             0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12,
                                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/**
 * Wait until the display has finished processing
 */
inline void display_wait_until_idle(void)
{
  while (PORTD & _BV(BUSY_PIN)) {
    _delay_ms(100);
  };
}

/**
 * Send a command to the display
 *
 * @param command the command to send
 */
void display_send_command(uint8_t command)
{
  // may not send commands while display is busy
  display_wait_until_idle();

  // make display interpret data as command
  // by setting the Data/Command pin to low
  // high pin would lead to interpretation as data
  PORTB &= ~_BV(DC_PIN);

  // send command
  spi_main_transmit(command);
}

/**
 * Send a piece of data to the display
 *
 * @param data the data to send
 */
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

/**
 * Reset the display. This is basically a soft reboot
 */
void display_reset(void)
{
  // the display may be reset by simply toggling the Reset Pin
  PORTB &= ~_BV(RST_PIN);
  // leave some time for the display to actually reset
  _delay_ms(200);
  PORTB |= _BV(RST_PIN);
  _delay_ms(200);
}

/**
 * Set the lookup table of the display. Different LUTs need to
 * be used when updating the entire display memory or only parts of it.
 *
 * @param lut the lookup table to be set on the display.
 *
 * @see lut_full_update
 * @see lut_partial_update
 */
void display_set_lookup_table(const unsigned char* lut)
{
  // different lookup tables are used when we update either
  // the entire display or only parts of it
  display_send_command(WRITE_LUT_REGISTER);
  // look up table is 30 bytes long
  for (int i = 0; i < 30; i++) {
    display_send_data(lut[i]);
  }
}

/**
 * Update only a part of the displays frame memory.
 *
 * @param image_buffer the bytearray to write to the display
 * @param x the x coordinate of the most upper left pixel
 * @param y the y coordinate of the most upper left pixel
 * @param image_width the width of the image
 * @param image_height the height of the image
 * @param stored_on_flash whether the image_buffer needs to be accessed from flash
 * @param overwrite an optional value to overwrite the values of the image buffer with
 * @param invert bitwise invert the image
 */
void display_set_partial_frame_memory(const unsigned char* image_buffer,
                                      int x,
                                      int y,
                                      int image_width,
                                      int image_height,
                                      int stored_on_flash,
                                      int overwrite,
                                      int invert)
{
  // only continue if action is valid
  if (image_buffer == NULL || x < 0 || y < 0 || image_width < 1 || image_height < 1) {
    return;
  }

  // to set only a part of the frame memory, the display needs to use the partial frame mempory
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

  // ensure that we're sending somthing that's 8bit tall and wide
  // as our display can't handle other resolutions
  if (x_end - x < 8) {
    x_end = x + 8;
  }
  if (y_end - y < 8) {
    y_end = y + 8;
  }

  // send byte by byte
  int data;
  for (int j = 0; j < y_end - y + 1; j++) {
    for (int i = 0; i < (x_end - x + 1) / 8; i++) {
      int index = i + j * (image_width / 8);
      if (stored_on_flash) {
        data = pgm_read_byte(&image_buffer[index]);
      } else if (overwrite > -1) {
        data = overwrite;
      } else {
        data = image_buffer[index];
      }
      display_send_data(invert ? ~data : data);
    }
  }
}

/**
 * Set the entire display frame memory
 *
 * @param image_buffer the bytearray to write to the display
 */
void display_set_entire_frame_memory(const unsigned char* image_buffer)
{
  // write an image buffer of the same size as the image into the frame buffer
  display_set_lookup_table(lut_full_update);

  // write over the entirety of the frame memory and start in the top left corner
  display_set_memory_area(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
  display_set_memory_pointer(0, 0);

  // write the frame buffer byte by byte
  display_send_command(WRITE_RAM);
  for (int i = 0; i < DISPLAY_WIDTH / 8 * DISPLAY_HEIGHT; i++) {
    display_send_data(pgm_read_byte(&image_buffer[i]));
  }
}

/**
 * Write a fixed value over the entire frame memory
 *
 * @param color the color to overwrite
 */
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

/**
 * Render the current frame memory onto the display.
 *
 */
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

/**
 * Tell the display onto which part of its memory we'd like to write.
 * The area is always box-shaped.
 *
 *    (x_start, y_start)  _______
 *                       |       |
 *                       |       |
 *                       |_______|  (x_end, y_end)
 *
 * @param x_start the x coordinate of the first pixel
 * @param y_start the y coordinate of the first pixel
 * @param x_end the x coordinate of the last pixel
 * @param y_end the y coordinate of the last pixel
 */
void display_set_memory_area(int x_start, int y_start, int x_end, int y_end)
{
  // in x-directionm the starting point must be a multiple of 8
  // this is a limitation of the display, which only supports
  // writing entire bytes into the memory
  display_send_command(SET_RAM_X_ADDRESS_START_END_POSITION);
  display_send_data((x_start >> 3) & 0xFF);
  display_send_data((x_end >> 3) & 0xFF);
  // in y direction, it's only important to send 8 bytes at once
  display_send_command(SET_RAM_Y_ADDRESS_START_END_POSITION);
  display_send_data(y_start & 0xFF);
  display_send_data((y_start >> 8) & 0xFF);
  display_send_data(y_end & 0xFF);
  display_send_data((y_end >> 8) & 0xFF);
}

/**
 * Move the memory pointer of the display to the desired
 * position.
 *
 * @param x x coordinate of the pointer
 * @param y y coordinate of the pointer
 */
void display_set_memory_pointer(int x, int y)
{
  display_send_command(SET_RAM_X_ADDRESS_COUNTER);
  display_send_data((x >> 3) & 0xFF);
  display_send_command(SET_RAM_Y_ADDRESS_COUNTER);
  display_send_data(y & 0xFF);
  display_send_data((y >> 8) & 0xFF);
  display_wait_until_idle();
}

/**
 * Write a piece of text to the display
 *
 * @param string the text to write
 * @param x x coordinate of the top left corner of the first letter
 * @param y y coordinate of of the top left corner of the first letter
 *
 * @see print_text for more user friendly text writing
 */
void display_write_text_block(char* string, int x, int y, int inverted)
{

  // compute required array size for the entire text
  // note that height and width are opposite to what you would expect
  // this is because the display itsel is narrow side up, while we
  // want our display to be wide side up
  int font_height = 16;
  int font_width = 24;
  int char_array_size = font_width / 8 * font_height;

  // compute the exact dimensions of the box
  // this also performs text wrapping in case the text would exceed
  // the display limits
  int string_length = strlen(string);
  int total_height = string_length * font_height;
  int box_height = total_height > y + DISPLAY_HEIGHT ? DISPLAY_HEIGHT - y : total_height;
  int box_width = ceil(total_height / box_height) * font_width;

  // allocate text box
  unsigned char text_box[(box_height / 8) * box_width];

  // write characters to textbox
  for (int char_index = 0; char_index < string_length; char_index++) {
    _delay_ms(50);
    int char_start = (string_length - 1 - char_index) * char_array_size;
    // LOG_DEBUG(DISPLAY, "Char starts at %d", char_start);

    for (int line_index = char_array_size - 1; line_index >= 0; line_index--) {
      int index = char_start + line_index;
      // LOG_DEBUG(DISPLAY, "Write line to %d", index);
      uint8_t value = pgm_read_byte(&font_16_24[string[char_index]][line_index]);
      text_box[index] = inverted ? ~value : value;
    }
  }

  // send text box to display
  display_set_partial_frame_memory(&text_box, x, y, box_width, box_height, 0, -1, 0);
}

/**
 * Userfriendly writing of text.
 * Converts x and y coordinates into their reprojected counterparts, as
 * we use the display in a flipped state
 *
 * @param string the text to write
 * @param x x coordinate of the top left corner of the first letter
 * @param y y coordinate of of the top left corner of the first letter
 *
 */
void print_text(char* string, int x, int y, int white_on_black)
{
  int block_width = strlen(string) * 16;
  int block_height = strlen(string) * 24;
  // display_write_text_block(string, DISPLAY_HEIGHT - x + block_width, y, white_on_black);
  display_write_text_block(string, y, DISPLAY_HEIGHT - x - block_width, white_on_black);
}

/**
 * Userfriendly displaying of images.
 * converts x and y coordinates into their reprojected counterparts, as
 * we use the display in a flipped state
 *
 * @param image_buffer a byte array containig the image data
 * @param x x coordinate of the top left corner of the image
 * @param y y coordinate of of the top left corner of the image
 * @param width the width of the image
 * @param height the height of the image
 *
 */
void show_image(const unsigned char* image_buffer, int x, int y, int width, int height, int invert)
{
  display_set_partial_frame_memory(
    image_buffer, y, DISPLAY_HEIGHT - x - width, height, width, 1, -1, invert);
}

/**
 * Draw a box in a specified location onto the display
 * converts x and y coordinates into their reprojected counterparts, as
 * we use the display in a flipped state
 *
 * @param color the filling color of the box (usually either 0xFF or 0x00)
 * @param x x coordinate of the top left corner of the box
 * @param y y coordinate of of the top left corner of the box
 * @param width the width of the box
 * @param height the height of the box
 *
 */
void draw_box(int color, int x, int y, int width, int height)
{
  display_set_partial_frame_memory(
    font_16_24[0], y, DISPLAY_HEIGHT - x - width, height, width, 0, color, 0);
}

/**
 * Wipe the memory of the current display by setting all bits of the memory areas to 1.
 * Due to the double memory in the display, this needs to be done multiple times.
 */
void display_wipe(void)
{
  display_set_lookup_table(lut_full_update);
  display_clear_frame_memory(0xFF);
  display_render_frame();
  _delay_ms(500);
  display_clear_frame_memory(0xFF);
  display_render_frame();
  _delay_ms(1000);
  display_set_lookup_table(lut_full_update);
  display_clear_frame_memory(0xFF);
  display_render_frame();
  _delay_ms(500);
  display_clear_frame_memory(0xFF);
  display_render_frame();
  _delay_ms(1000);
}

/// RENDERING OF THE INTERFACE

/**
 * Update a recipe name on the frame buffer
 *
 * @param recipe_name the new recipe name
 */
void update_recipe_name(char* recipe_name)
{
  draw_box(0x00, 0, 0, 296, 40);
  print_text(recipe_name, 20, 10, 1);
};

/**
 * Update provided temperature and humidity values on the frame buffer
 *
 * @param temperature the temperature to display
 * @param humidiy the humidity to display
 */
void update_temperature_and_humidity(int32_t temperature, int32_t humidity)
{
  draw_box(0xFF, 0, 40, 296, 24);
  char* temp_string[20];
  sprintf(temp_string, "Temp %dC", temperature / 100);
  print_text(temp_string, 20, 44, 0);
  if (humidity != -1) {
    char* hum_string[20];
    sprintf(hum_string, "Hum %d%s", humidity / 1000, '%');
    print_text(hum_string, 128 + 20 + 20, 44, 0);
  }
};

/**
 * Write a new recipe to the display memory
 * @param recipe_name the name of the recipe
 * @param temperature the temperature specified by the recipe
 * @param humidity the temperature specified by the recipe
 */
void update_recipe(char* recipe_name, int32_t temperature, uint32_t humidity)
{
  update_recipe_name(recipe_name);
  update_temperature_and_humidity(temperature, humidity);
}

void render_temperature_and_humidity(int32_t temperature, uint32_t humidity)
{
  update_temperature_and_humidity(temperature, humidity);
  display_render_frame();
}

/**
 * Write a new recipe to the display memory
 * @param recipe_name the name of the recipe
 * @param temperature the temperature specified by the recipe
 * @param humidity the temperature specified by the recipe
 */
void render_recipe(char* recipe_name, int32_t temperature, uint32_t humidity)
{
  update_recipe(recipe_name, temperature, humidity);
  draw_box(0xFF, 0, 80, 296, 48);
  print_text("OK to configure", 20, 92, 0);
  display_render_frame();
}

/**
 * Display a new recipe and submenu on the eink display.
 * This is the main render function for the majority of
 * our fermentation process.
 *
 * @param recipe_name the name of the recipe
 * @param temperature the temperature specified by the recipe
 * @param humidity the temperature specified by the recipe
 * @param ch_ctx the context of the render, i.e. the action that's supposed to happen
 * @param fermentation_started whether the fermenation process has previously been started
 */
void render_recipe_and_submenus(char* recipe_name,
                                int32_t temperature,
                                uint32_t humidity,
                                int8_t ch_ctx,
                                uint8_t fermentation_started)
{
  if (fermentation_started) {
    for (int i = 0; i < 2; i++) {
      update_recipe(recipe_name, temperature, humidity);
      render_menubuttons(ch_ctx);
      render_menubutton_selection(ch_ctx);
      display_render_frame();
    }
  } else {
    for (int i = 0; i < 2; i++) {
      draw_box(0xFF, 0, 40, 296, 80);
      render_ferm_start(recipe_name, ch_ctx);
      display_render_frame();
    }
  }
}

/**
 * Called once at the beginning of the fermentation process
 * , renders a selection of submenus.
 *
 * @param recipe_name the name to display
 * @param change_ctx the context of the action, in this case the position of the selection
 */
void render_ferm_start(char* recipe_name, int8_t change_ctx)
{
  draw_box(0x00, 0, 0, 296, 40);
  print_text(recipe_name, 20, 10, 1);
  draw_box(0xFF, 0, 40, 296, 80);
  print_text("OK to start", 20, 44, 0);

  render_menubuttons(change_ctx);
  render_menubutton_selection(change_ctx);
  display_render_frame();
}

/**
 * Renders the temperature selection submenu
 *
 * @param recipe_name the name of the recipe
 * @param temp a temperature value, in this case the temperature required by the recipe
 * @param submenu_change_ctx the context of the submenu
 */
void render_submenu_temp(char* recipe_name, int32_t temp, int8_t submenu_change_ctx)
{
  draw_box(0x00, 0, 0, 296, 40);
  print_text(recipe_name, 20, 10, 1);
  draw_box(0xFF, 0, 40, 296, 80);
  char* temp_string[20];
  sprintf(temp_string, "Change Temp %dC", temp / 100);
  print_text(temp_string, 20, 44, 0);

  render_submenu_buttons(submenu_change_ctx);
  display_render_frame();
}

/**
 * Renders the humidity selection submenu
 *
 * @param recipe_name the name of the recipe
 * @param temp a temperature value, in this case the temperature required by the recipe
 * @param submenu_change_ctx the context of the submenu
 */
void render_submenu_hum(char* recipe_name, uint32_t hum, int8_t submenu_change_ctx)
{
  draw_box(0x00, 0, 0, 296, 40);
  print_text(recipe_name, 20, 10, 1);
  draw_box(0xFF, 0, 40, 296, 80);
  char* hum_string[20];
  sprintf(hum_string, "Change Hum %d", hum / 100);
  print_text(hum_string, 20, 44, 0);

  render_submenu_buttons(submenu_change_ctx);
  display_render_frame();
}

/**
 * Renders the submenu buttons
 *
 * @param submenu_change_ctx the context of the submenu, ie. the current selection
 */
void render_submenu_buttons(int8_t submenu_change_ctx)
{
  draw_box(0xFF, 0, 100, 296, 28);
  switch (submenu_change_ctx) {
    case 0:
      print_text("+", 20, 100, 1);
      print_text("-", 70, 100, 0);
      print_text("Save", 120, 100, 0);
      break;
    case 1:
      print_text("+", 20, 100, 0);
      print_text("-", 70, 100, 1);
      print_text("Save", 120, 100, 0);
      break;
    case 2:
      print_text("+", 20, 100, 0);
      print_text("-", 70, 100, 0);
      print_text("Save", 120, 100, 1);
      break;
    default:
      break;
  }
  // display_render_frame();
}

/**
 * Renders a block to indicate the currently selected menu button
 *
 * @param change_ctx the number of the currently selected button
 */
void render_menubutton_selection(int8_t change_ctx)
{
  draw_box(0xFF, 0, 120, 296, 8);
  int x_pos = 0;
  switch (change_ctx) {
    case 0:
      x_pos = 54;
      break;
    case 1:
      x_pos = 114;
      break;
    case 2:
      x_pos = 174;
      break;
    case 3:
      x_pos = 234;
      break;
    default:
      break;
  }
  draw_box(0xC0, x_pos, 120, 8, 8);
}

/**
 * Renders the submenu buttons
 *
 * @param submenu_change_ctx the context of the submenu, ie. the current selection
 */
void render_menubuttons(int8_t change_ctx)
{
  draw_box(0xFF, 0, 80, 296, 48);
  show_image(start_icon, 38, 80, 40, 40, 0);
  show_image(temp_icon, 98, 80, 40, 40, 0);
  show_image(hum_icon, 158, 80, 40, 40, 0);
  show_image(back_icon, 218, 80, 40, 40, 0);
}

/**
 * Initialize the display and set all neccessary settings.
 * The settings are taken straight from the displays documentation.
 */
void display_init(void)
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

  LOG_DEBUG(DISPLAY, "DRIVER_OUTPUT_CONTROL");
  display_send_command(DRIVER_OUTPUT_CONTROL);
  display_send_data((DISPLAY_HEIGHT - 1) & 0xFF);
  display_send_data(((DISPLAY_HEIGHT - 1) >> 8) & 0xFF);
  display_send_data(0x00);

  LOG_DEBUG(DISPLAY, "BOOSTER_SOFT_START_CONTROL");
  display_send_command(BOOSTER_SOFT_START_CONTROL);
  display_send_data(0xD7);
  display_send_data(0xD6);
  display_send_data(0x9D);

  LOG_DEBUG(DISPLAY, "WRITE_VCOM_REGISTER");
  display_send_command(WRITE_VCOM_REGISTER);
  display_send_data(0xA8);

  LOG_DEBUG(DISPLAY, "SET_DUMMY_LINE_PERIOD");
  display_send_command(SET_DUMMY_LINE_PERIOD);
  display_send_data(0x1A);

  LOG_DEBUG(DISPLAY, "SET_GATE_TIME");
  display_send_command(SET_GATE_TIME);
  display_send_data(0x08);

  LOG_DEBUG(DISPLAY, "DATA_ENTRY_MODE_SETTING");
  display_send_command(DATA_ENTRY_MODE_SETTING);
  display_send_data(0x03);

  display_set_lookup_table(lut_full_update);

  display_wipe();
}