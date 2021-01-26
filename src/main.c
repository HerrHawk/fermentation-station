#include "controller/fermentation.h"
#include "controller/renderer.h"
#include "drivers/bme280.h"
#include "drivers/mpr121.h"
#include "globals.h"
#include "helpers.h"
#include "interfaces/i2c.h"
#include "logging.h"
#include <avr/io.h>
#include <util/delay.h>

struct state;
typedef void state_fn(struct state*);

struct state
{
  state_fn* next;
  int pass_data;
};

// TODO: dynamic array?
struct recipe recipes[] = {
  { "Lactobacillales",
    2500,
    300,
    -1,
    0 }, // Lactofermentierung, 25° (28° optimal), no humidity (-1 means perma off)
  { "SCOBY",
    2500,
    300,
    -1,
    0 }, // Symbiotic Culture of Bacteria and Yeast, 25° (28° optimal), no hum (-1)
  { "Aspergillus Orycae", 3000, 300, 72000, 7000 } // Koji, 30°, 72% humidity
};

uint8_t recipe_counter;
uint8_t change_context;

// int r = rand() % 20;

/*
 * SW States:
 *  - Waiting for user (touch) input
 *  - In Fermentation process (waiting for user input to either modify or cancel)
 */

state_fn main_menu;
state_fn fermentation_process;
state_fn error;
state_fn fatalerror;

// TODO: Move to helpers? - what about the recipe and recipecounter globals? Move to globals
// they will be only used in main loop afaik...
void set_recipe_counter(uint8_t modifier)
{
  recipe_counter = recipe_counter + modifier;
  uint8_t size = (sizeof(recipes) / sizeof(recipes[0]));

  if (recipe_counter > size) {
    // If the recipe counter is at -1 -> set counter to last element in array
    recipe_counter = size - 1;
  } else if (recipe_counter == size) {
    recipe_counter = 0;
  }

  LOG_DEBUG(DEFAULT, "recipe counter is now %d", recipe_counter);
}

// TODO: Move to helpers? - what about the recipe and recipecounter globals? Move to globals
// they will be only used in main loop afaik...
void set_change_context(uint8_t modifier)
{

  // Change context :
  // 0: idle
  // 1: change temperature
  // 2: change humidity
  // 3: exit
  change_context = change_context + modifier;

  // If the uint8 "overflows"
  if (change_context > 4) {
    change_context = 3;
  }
  // TODO: Does idle state (0) get re-selected?
  else if (change_context == 4) {
    change_context = 0;
  }

  LOG_DEBUG(DEFAULT, "change_context is now %d", change_context);
}

// The main menu waits for user touch input then starts fermentation with a custom or predefined
// recipe
void main_menu(struct state* state)
{
  LOG_DEBUG(CONTROL, "entering main menu");

  // TODO
  // display.render_main_menu()
  recipe_counter = 0;
  render_recipe(recipes[recipe_counter].recipe_name, recipes[recipe_counter].desired_temp);

  for (;;) {
    LOG_DEBUG(CONTROL, "wait for user touch input");
    uint8_t touch_input = mpr121_read_byte(0x00);
    // TODO: Remove delay once implementation is complete
    _delay_ms(2000);

    switch (touch_input) {
      case 0b001: // (+) button pressed
        set_recipe_counter(1);
        render_recipe(recipes[recipe_counter].recipe_name, recipes[recipe_counter].desired_temp);
        break;

      case 0b010: // (-) button pressed
        set_recipe_counter(-1);
        render_recipe(recipes[recipe_counter].recipe_name, recipes[recipe_counter].desired_temp);
        break;

      case 0b100: // (ok) button pressed
        LOG_DEBUG(DEFAULT, "switching state to fermentation");
        state->next = fermentation_process;
        return;

      default:
        // Do nothing, next iteration of loop
        break;
    }
  }
}

// The fermentation process regulates temp and hum and also waits for user input to cancel or alter
// temp or hum
void fermentation_process(struct state* state)
{

  // TODO
  // display.render_fermentation_view()
  // display.renderRecipe(recipe_counter)

  LOG_DEBUG(
    CONTROL, "starting fermentation process with recipe %s ", recipes[recipe_counter].recipe_name);
  change_context = 0;

  uint8_t fermentation_started = 0;

  // TODO
  // initialize_temp_ctrl (hystherese from recipe data)
  // initialize_temp_ctrl (hystherese from recipe data)

  for (;;) {
    // Get touch input
    uint8_t touch_input = mpr121_read_byte(0x00);

    _delay_ms(2000);

    switch (touch_input) {
      case 0b001: // (+) button
        set_change_context(1);
        // display.update();
        break;
      case 0b010: // (-) button
        set_change_context(-1);
        // display.update();
        break;
      case 0b100: // (ok) button
        // TODO: Switch to submenu context
        // switch_to_submenu_and_return();
        // display.update();

        switch (change_context) {
          case 0: // (idle context btn) -> start/stop fermentation
            // Fermentation start/stop
            if (fermentation_started == 0) {
              LOG_DEBUG(CONTROL, "fermentation has been started");
              fermentation_started = 1;
            } else {
              LOG_DEBUG(CONTROL, "fermentation has been stopped");
              fermentation_started = 0;
            }
            break;
          case 1: // change temp context
            // Call sub menu for temp change
            LOG_DEBUG(DEFAULT, "implement me: temp change");
            break;
          case 2: // change hum context
            // Call sub menu for hum change
            LOG_DEBUG(DEFAULT, "implement me: hum change");
            break;
          case 3: //(exit context btn)
            // Stop actors
            // TODO
            // heating & nebuliz0r stop

            // Return to main menu
            state->next = main_menu;
            return;
          default: // This should never happen, because cases are predefined from 0-3
            LOG_DEBUG(DEFAULT, "FATAL ERROR");
            break;
        }

      default:
        LOG_DEBUG(DEFAULT, "no touch input received");

        if (fermentation_started) {
          // Check temperature
          check_temp(&recipes[recipe_counter]);
          // Check humidity
          check_hum(&recipes[recipe_counter]);
        }
        break;
    }
  }

  LOG_DEBUG(DEFAULT, "begin fermentation");
  _delay_ms(5000);

  LOG_DEBUG(DEFAULT, "fermentation complete!");
  state->next = main_menu;
}

// nested loop method in fermentation_process
// "invisible nested state"
void sub_menu_temperature_change()
{
  // update_display()

  // for(;;)
  //    get_user_input()
  //    switch()
  //      increment_temp()
  //      decrement_temp()
  //      exit_sub_menu() -> return
}

// nested loop method in fermentation_process
// "invisible nested state"
void sub_menu_humidity_change()
{
  // update_display()

  // for(;;)
  //    get_user_input()
  //    switch()
  //      increment_hum()
  //      decrement_hum()
  //      exit_sub_menu() -> return
}

// Dummy method for occurring errors
void error_function()
{
  LOG_DEBUG(DEFAULT, "error occurred, writing to log");
  _delay_ms(2500);
}

int main(void)
{
  // run setup/initialization functions

  uart_init();
  I2CInit();
  _delay_ms(100);
  mpr121_init();
  bme280_init();

  display_init();

  // Initialize state machine with initial state "wait for touch"
  // (waiting for user input after init)
  struct state state = { main_menu, 0 };
  display_wipe();

  while (1) {

    state.next(&state);
    // setBit(PORTB, PB5);
    // _delay_ms(200);
    // clearBit(PORTB, PB5);
    // _delay_ms(200);
    // toggleBit(PORTB, PB5);
    // _delay_ms(200);
    // toggleBit(PORTB, PB5);
    // LOG_DEBUG(DEFAULT, "Main Test Routine");
    // _delay_ms(1000);
    // LOG_DEBUG(DEFAULT, "Start BME280 Test");
    // int32_t temp = bme280_read_temp();
    // uint32_t hum = bme280_read_hum();
    // LOG_DEBUG(DEFAULT, "Temperature: %ld", temp);
    // LOG_DEBUG(DEFAULT, "Humidity: %ld", hum);
    // LOG_DEBUG(DEFAULT, "End BME280 Test");

    // _delay_ms(2000);

    // uint8_t rd = mpr121_read_byte(0x00);
    // LOG_DEBUG(DEFAULT, "touch: %x", rd);
    // LOG_DEBUG(DEFAULT, "====================");

    // rd = mpr121_read_byte(0x1E);
    // LOG_DEBUG(DEFAULT, "bl: %x", rd);

    // _delay_ms(150);
  }

  // can never be reached
  return 0;
}
