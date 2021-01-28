#include "controller/fermentation.h"
#include "controller/renderer.h"
#include "drivers/bme280.h"
#include "drivers/mpr121.h"
#include "globals.h"
#include "helpers.h"
#include "interfaces/i2c.h"
#include "logging.h"
#include "timer.h"
#include <avr/io.h>
#include <util/delay.h>

// Types needed for state machine program architecture
struct state;
typedef void state_fn(struct state*);
struct state
{
  state_fn* next;
  int pass_data;
};

// Our pre-defined recipes to be selected for a fermentation process
struct recipe recipes[] = {
  { "Lactobacillales",
    2500,
    -1,
    0 }, // Lactofermentierung, 25° (28° optimal), no humidity (-1 means perma off)
  { "SCOBY",
    2500,
    -1,
    0 }, // Symbiotic Culture of Bacteria and Yeast, 25° (28° optimal), no hum (-1 means perma off)
  { "Asp. Orycae", 3000, 300, 72000, 7000 } // Koji, 30°, 72% humidity
};

// Global variable that determines which one of the recipes is currently selected
int8_t recipe_counter;
// Global variable that determines which of the context menu buttons in the
// fermentation menu is selected
int8_t change_context;
// Global variable that determines which of the context menu buttons in the temp /
// hum sub-menus is selected
int8_t submenu_change_context;

// Declaring functions as states for the state machine
state_fn recipe_selection;
state_fn fermentation_process;

// Setting the recipe counter in a cyclic type of list that returns
// to the beginning/end when reaching the max or min value
void set_recipe_counter(int8_t modifier)
{
  recipe_counter = recipe_counter + modifier;
  int8_t size = (sizeof(recipes) / sizeof(recipes[0]));

  if (recipe_counter == -1) {
    // If the recipe counter is at -1 -> set counter to last element in array
    recipe_counter = size - 1;
  } else if (recipe_counter == size) {
    // If the recipe counter exceeds the maximum range return to the first element
    recipe_counter = 0;
  }

  LOG_DEBUG(DEFAULT, "recipe counter is now %d", recipe_counter);
}

// Setting the fermentation menu change context in a cyclic type of list that returns
// to the beginning/end when reaching the max or min value
void set_change_context(int8_t modifier)
{
  // Change context :
  // 0: idle
  // 1: change temperature
  // 2: change humidity
  // 3: exit
  change_context = change_context + modifier;

  if (change_context == -1) {
    change_context = 3;
  } else if (change_context == 4) {
    change_context = 0;
  }

  LOG_DEBUG(DEFAULT, "change_context is now %d", change_context);
}

// Setting the temp/hum sub-menu change context in a cyclic type of list that returns
// to the beginning/end when reaching the max or min value
void set_submenu_change_context(int8_t modifier)
{
  // Submenu Change context :
  // 0: plus
  // 1: minus
  // 2: Exit
  submenu_change_context = submenu_change_context + modifier;

  if (submenu_change_context == -1) {
    submenu_change_context = 2;
  } else if (submenu_change_context == 3) {
    submenu_change_context = 0;
  }

  LOG_DEBUG(DEFAULT, "submenu change_context is now %d", submenu_change_context);
}

// The main menu waits for user touch input then starts fermentation with a custom or predefined
// recipe
void recipe_selection(struct state* state)
{
  LOG_DEBUG(CONTROL, "entering recipe selection menu");

  // initialize the main menu with the first recipe (recipes[0])
  recipe_counter = 0;
  // Rendering the first recipe onto the display
  render_recipe(recipes[recipe_counter].recipe_name,
                recipes[recipe_counter].desired_temp,
                recipes[recipe_counter].desired_hum);

  for (;;) {

    LOG_DEBUG(CONTROL, "wait for user touch input");
    // Read the touch input on the touch sensor
    // 0b001 : Button 1
    // 0b010 : Button 2
    // 0b100 : Button 3
    uint8_t touch_input = mpr121_read_byte(0x00);

    // TODO: Remove delay once implementation is complete
    _delay_ms(1000);

    switch (touch_input) {
      case 0b001: // (+) button pressed
        // Increment the recipe counter
        set_recipe_counter(1);
        // Re-render the display with the the next recipe
        render_recipe(recipes[recipe_counter].recipe_name,
                      recipes[recipe_counter].desired_temp,
                      recipes[recipe_counter].desired_hum);
        break;

      case 0b010: // (-) button pressed
        // Decrement the recipe counter
        set_recipe_counter(-1);
        // Re-render the display with the previous recipe
        render_recipe(recipes[recipe_counter].recipe_name,
                      recipes[recipe_counter].desired_temp,
                      recipes[recipe_counter].desired_hum);
        break;

      case 0b100: // (ok) button pressed
        LOG_DEBUG(DEFAULT, "switching state to fermentation");
        // Switch the state machine over to fermentation process
        state->next = fermentation_process;
        return;

      default:
        // No touch input received -> Do nothing, next iteration of loop
        break;
    }
  }
}

// The fermentation process regulates temp and hum and also waits for user input to cancel or alter
// temp or hum in a sub-menu
void fermentation_process(struct state* state)
{
  // Initialize the menu context counter and fermentation started variable to zero
  change_context = 0;
  uint8_t fermentation_started = 0;

  // Has to be called twice to fully overwrite the "alternating memories"
  render_ferm_start(recipes[recipe_counter].recipe_name, change_context);
  render_ferm_start(recipes[recipe_counter].recipe_name, change_context);

  LOG_DEBUG(
    CONTROL, "starting fermentation process with recipe %s ", recipes[recipe_counter].recipe_name);

  for (;;) {
    // Read the touch input on the touch sensor
    // 0b001 : Button 1
    // 0b010 : Button 2
    // 0b100 : Button 3
    uint8_t touch_input = mpr121_read_byte(0x00);

    // TODO: Delete once implementation is complete
    _delay_ms(1000);

    switch (touch_input) {
      case 0b001: // (+) button
        // Increment the change context
        set_change_context(1);
        // Re-render the display and its submenu-bar (the current menupoint will be highlighted)
        render_recipe_and_submenus(recipes[recipe_counter].recipe_name,
                                   recipes[recipe_counter].desired_temp,
                                   recipes[recipe_counter].desired_hum,
                                   change_context,
                                   fermentation_started);
        break;
      case 0b010: // (-) button
        // Decrement the change context
        set_change_context(-1);
        // Re-render the display and its submenu-bar (the current menupoint will be highlighted)
        render_recipe_and_submenus(recipes[recipe_counter].recipe_name,
                                   recipes[recipe_counter].desired_temp,
                                   recipes[recipe_counter].desired_hum,
                                   change_context,
                                   fermentation_started);
        break;
      case 0b100: // (ok) button

        // Change context :
        // 0: idle                  --> switch fermentation on or off
        // 1: change temperature    --> switch to submenu to change temp of the recipe
        // 2: change humidity       --> switch to submenu to change hum of the recipe
        // 3: exit                  --> exit and go back to recipe selection menu

        switch (change_context) {
          case 0: // (idle context btn) -> start/stop fermentation
            // Fermentation start/stop
            if (fermentation_started == 0) {
              LOG_DEBUG(CONTROL, "fermentation has been started");
              fermentation_started = 1;
            } else {
              LOG_DEBUG(CONTROL, "fermentation has been stopped");
              fermentation_started = 0;
              change_context = 0;
              // Render the fermentation starting menu and its buttons
              // (twice to overwrite "alternating memories")
              render_ferm_start(recipes[recipe_counter].recipe_name, change_context);
              render_ferm_start(recipes[recipe_counter].recipe_name, change_context);
            }
            break;
          case 1: // change temp context
            // Switch to sub-menu for temperature change
            sub_menu_temperature_change();
            // After changing the temp, re-render the display with the new required temperature
            render_recipe_and_submenus(recipes[recipe_counter].recipe_name,
                                       recipes[recipe_counter].desired_temp,
                                       recipes[recipe_counter].desired_hum,
                                       change_context,
                                       fermentation_started);
            LOG_DEBUG(DEFAULT, "implement me: temp change");
            break;
          case 2: // change hum context
            LOG_DEBUG(DEFAULT, "implement me: hum change");

            if (recipes[recipe_counter].desired_hum != -1) {
              // Switch to sub-menu for humidity change
              sub_menu_humidity_change();
              // After changing the hum, re-render the display with the new required humidity
              render_recipe_and_submenus(recipes[recipe_counter].recipe_name,
                                         recipes[recipe_counter].desired_temp,
                                         recipes[recipe_counter].desired_hum,
                                         change_context,
                                         fermentation_started);
            }
            break;
          case 3: //(exit context btn)

            // Stop actors
            // TODO
            // heating & nebuliz0r stop

            // Return to recipe selection
            state->next = recipe_selection;
            return;
          default: // This should never happen, because cases are predefined from 0-3
            LOG_DEBUG(DEFAULT, "FATAL ERROR");
            break;
        }

      default:
        LOG_DEBUG(DEFAULT, "no touch input received");

        // If the fermentation process is running, check the temp and hum and let the pid controller
        // control the heating and humidity control process
        if (fermentation_started) {
          // Check temperature
          int32_t c_temp = check_temp(&recipes[recipe_counter]);
          // Check humidity
          uint32_t c_hum = check_hum(&recipes[recipe_counter]);

          // Update the display to show the current temp + hum
          render_recipe_and_submenus(recipes[recipe_counter].recipe_name,
                                     c_temp,
                                     c_hum,
                                     change_context,
                                     fermentation_started);
        }
        break;
    }
  }
}

// nested loop method in fermentation_process
// "invisible nested state"
void sub_menu_temperature_change()
{
  submenu_change_context = 0;
  // Render the submenu-view for temperature change
  // with the recipe's base data for temperature
  render_submenu_temp(recipes[recipe_counter].recipe_name,
                      recipes[recipe_counter].desired_temp,
                      submenu_change_context);

  for (;;) {
    // Read the touch input on the touch sensor
    // 0b001 : Button 1
    // 0b010 : Button 2
    // 0b100 : Button 3
    uint8_t touch_input = mpr121_read_byte(0x00);

    // TODO: Remove once implementation is complete
    _delay_ms(1000);

    switch (touch_input) {
      case 0b001: // (+) button
        // Increment sub menu selection context
        set_submenu_change_context(1);
        // Re-render the submenu with the new selected btn
        render_submenu_temp(recipes[recipe_counter].recipe_name,
                            recipes[recipe_counter].desired_temp,
                            submenu_change_context);
        break;
      case 0b010: // (-) button
        // Decrement sub menu selection context
        set_submenu_change_context(-1);
        // Re-render the submenu with the new selected btn
        render_submenu_temp(recipes[recipe_counter].recipe_name,
                            recipes[recipe_counter].desired_temp,
                            submenu_change_context);
        break;
      case 0b100: // (OK) button

        // Submenu Change context :
        // 0: plus      --> Increment temperature
        // 1: minus     --> Decrement temperature
        // 2: Exit      --> Exit (and save but that happens implicitly)
        switch (submenu_change_context) {
          case 0: // Increment temp
            // update temp value in recipe + 1 °C
            recipes[recipe_counter].desired_temp += 100;
            // Re-render with new temp
            render_submenu_temp(recipes[recipe_counter].recipe_name,
                                recipes[recipe_counter].desired_temp,
                                submenu_change_context);
            break;
          case 1: // Decrement temp
            // update temp value in recipe - 1 °C
            recipes[recipe_counter].desired_temp -= 100;
            // Re-render with new temp
            render_submenu_temp(recipes[recipe_counter].recipe_name,
                                recipes[recipe_counter].desired_temp,
                                submenu_change_context);
            break;
          case 2: // Exit submenu
            return;

          default:
            break;
        }
        break;
      default:
        break;
    }
  }
}

// nested loop method in fermentation_process
// "invisible nested state"
void sub_menu_humidity_change()
{
  submenu_change_context = 0;
  // Render the submenu-view for humdity change
  // with the recipe's base data for humidity
  render_submenu_hum(recipes[recipe_counter].recipe_name,
                     recipes[recipe_counter].desired_hum,
                     submenu_change_context);

  for (;;) {
    // Read the touch input on the touch sensor
    // 0b001 : Button 1
    // 0b010 : Button 2
    // 0b100 : Button 3
    uint8_t touch_input = mpr121_read_byte(0x00);

    // TODO: Remove once implementation is complete
    _delay_ms(1000);

    switch (touch_input) {
      case 0b001: // (+) button
        // Increment sub menu selection context
        set_submenu_change_context(1);
        // Re-render the submenu with the new selected btn
        render_submenu_hum(recipes[recipe_counter].recipe_name,
                           recipes[recipe_counter].desired_hum,
                           submenu_change_context);
        break;
      case 0b010: // (-) button
        // Decrement sub menu selection context
        set_submenu_change_context(-1);
        // Re-render the submenu with the new selected btn
        render_submenu_hum(recipes[recipe_counter].recipe_name,
                           recipes[recipe_counter].desired_hum,
                           submenu_change_context);
        break;
      case 0b100: // (OK) button

        // Submenu Change context :
        // 0: plus      --> Increment temperature
        // 1: minus     --> Decrement temperature
        // 2: Exit      --> Exit (and save but that happens implicitly)
        switch (submenu_change_context) {
          case 0: // Increment hum
            // update hum value in recipe + 1 %
            recipes[recipe_counter].desired_hum += 1000;
            // Re-render with new hum
            render_submenu_hum(recipes[recipe_counter].recipe_name,
                               recipes[recipe_counter].desired_hum,
                               submenu_change_context);
            break;
          case 1: // Decrement hum
            // update temp value in recipe - 1 %
            recipes[recipe_counter].desired_hum -= 1000;
            // Re-render with new hum
            render_submenu_hum(recipes[recipe_counter].recipe_name,
                               recipes[recipe_counter].desired_hum,
                               submenu_change_context);
            break;
          case 2: // Exit submenu
            return;

          default:
            break;
        }
        break;
      default:
        break;
    }
  }
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
  setup_heating_element();
  setup_timer_s1();

  display_init();

  // Initialize state machine with initial state "wait for touch"
  // (waiting for user input after init)
  struct state state = { recipe_selection, 0 };
  display_wipe();

  while (1) {
    state.next(&state);
  }
  return 0;
}
