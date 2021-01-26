#pragma once
#include <avr/io.h>

void display_init();
void display_send_command(uint8_t command);
void display_reset();
extern inline void display_wait_until_idle();
void display_wipe(void);
void print_text(char* string, int x, int y, int white_on_black);
void render_recipe(char* recipe_name, int temperature);
void show_image(const unsigned char* image_buffer, int x, int y, int width, int height, int invert);
void draw_box(int color, int x, int y, int width, int height);
void demo();
void display_set_entire_frame_memory(const unsigned char* image_buffer);
void display_set_partial_frame_memory(const unsigned char* image_buffer,
                                      int x,
                                      int y,
                                      int image_width,
                                      int image_height,
                                      int stored_on_flash,
                                      int overwrite,
                                      int invert);