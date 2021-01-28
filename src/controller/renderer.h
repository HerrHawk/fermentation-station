#pragma once
#include <avr/io.h>

void display_init();
void display_send_command(uint8_t command);
void display_reset();
extern inline void display_wait_until_idle();
void display_wipe(void);
void print_text(char* string, int x, int y, int white_on_black);
void display_render_frame(void);
void render_recipe(char* recipe_name, int32_t temperature, uint32_t humidity);
void render_recipe_and_submenus(char* recipe_name,
                                int32_t temperature,
                                uint32_t humidity,
                                int8_t ch_ctx,
                                uint8_t fermentation_started);
void render_submenu_temp(char* recipe_name, int32_t temp, int8_t submenu_change_ctx);
void render_submenu_hum(char* recipe_name, uint32_t hum, int8_t submenu_change_ctx);
void render_menubuttons(int8_t change_ctx);
void render_submenu_buttons(int8_t submenu_change_ctx);
void render_ferm_start(char* recipe_name, int8_t change_ctx);
void show_image(const unsigned char* image_buffer, int x, int y, int width, int height, int invert);
void draw_box(int color, int x, int y, int width, int height);
// void demo();
void display_set_entire_frame_memory(const unsigned char* image_buffer);
void display_set_partial_frame_memory(const unsigned char* image_buffer,
                                      int x,
                                      int y,
                                      int image_width,
                                      int image_height,
                                      int stored_on_flash,
                                      int overwrite,
                                      int invert);