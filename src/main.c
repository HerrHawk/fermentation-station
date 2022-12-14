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
/*
// Lactofermentierung, 25° (28° optimal), no humidity (-1 means perma off)
// Symbiotic Culture of Bacteria and Yeast, 25° (28° optimal), no hum (-1 means perma off)
// Koji, 30°, 72% humidity
*/
struct recipe recipes[] = { { "Lactobacillales", 2500, -1, 0 },
                            { "SCOBY", 2500, -1, 0 },
                            { "Asp. Orycae", 3000, 72000, 7000 },
                            { "Test1", 4500, -1, 0 },
                            { "Test2", 5000, 60000, 6000 } };

// Global variable that determines which one of the recipes is currently selected
int8_t recipe_counter;
// Global variable that determines which of the context menu buttons in the
// fermentation menu is selected
int8_t change_context;
// Global variable that determines which of the context menu buttons in the temp /
// hum sub-menus is selected
int8_t submenu_change_context;
// Global variable that tells the program whether a fermentation is currently running or not
uint8_t fermentation_started;

// Declaring functions as states for the state machine
state_fn recipe_selection;
state_fn fermentation_process;

// Setting the recipe counter in a cyclic type of list that returns
// to the beginning/end when reaching the max or min value
void set_recipe_counter(int8_t modifier)
{
  recipe_counter = recipe_counter + modifier;
  // Get the size of the array
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

// The main menu waits for user touch input then starts fermentation with a predefined recipe
void recipe_selection(struct state* state)
{
  LOG_DEBUG(CONTROL, "entering recipe selection menu");

  // initialize the main menu with the first recipe (recipes[0]) -> reset counter
  recipe_counter = 0;
  // Rendering the first recipe onto the display
  render_recipe(recipes[recipe_counter].recipe_name,
                recipes[recipe_counter].desired_temp,
                recipes[recipe_counter].desired_hum);

  for (;;) {

    LOG_DEBUG(CONTROL, "wait for user touch input");
    // Read the touch input on the register in which the touch sensor writes
    // 0b001 : Button 1
    // 0b010 : Button 2
    // 0b100 : Button 3
    uint8_t touch_input = get_touch();

    _delay_ms(10);

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

// The fermentation process regulates temp and hum and also waits for user input to start/cancel the
// ferment or alter temp or hum in a sub-menu
void fermentation_process(struct state* state)
{
  // Renders the fermentation menu overview

  if (!fermentation_started) {
    render_ferm_start(recipes[recipe_counter].recipe_name, change_context);
  }
  // Get current temp and hum
  int32_t c_temp = bme280_read_temp();
  uint32_t c_hum = recipes[recipe_counter].desired_hum != -1 ? bme280_read_hum() : -1;

  // Render recipe overview including the submenu buttons
  // Display current temp and hum
  render_recipe_and_submenus(
    recipes[recipe_counter].recipe_name, c_temp, c_hum, change_context, fermentation_started);
  LOG_DEBUG(
    CONTROL, "starting fermentation process with recipe %s ", recipes[recipe_counter].recipe_name);

  // Remember the change context (which button is highlighted/selected)
  int8_t last_context = change_context;

  for (;;) {
    // loop until a touch input has been detected
    // if nothing happens, this will just idle here and continuosly update
    // temperature and humidity
    uint8_t touch_input = get_touch();
    while (!touch_input) {
      LOG_DEBUG(DEFAULT, "no touch input received");

      // If the fermentation process is running, check the temp and hum and let the pid controller
      // control the heating and humidity control process
      if (fermentation_started && s1_triggered) {
        // Check temperature and humidity
        // if the humidity isn't part of the recipe, the returned -1 will disable
        // printing for the humidity part of the interface
        c_temp = check_temp(&recipes[recipe_counter]);
        c_hum = check_hum(&recipes[recipe_counter]);
        render_temperature_and_humidity(c_temp, c_hum);
      }
      // check whether touch input has been received by now
      touch_input = get_touch();
    }

    _delay_ms(10);

    // Interpret the touch input
    // 0b001 : Button 1
    // 0b010 : Button 2
    // 0b100 : Button 3
    switch (touch_input) {
      case 0b001: // (+) button
        // Increment the change context and get back to requesting user input
        set_change_context(1);
        return;
        break;
      case 0b010: // (-) button
        // Decrement the change context and get back to requesting user input
        set_change_context(-1);
        return;
        break;
      case 0b100: // (ok) button
        break;
      default:
        break;
    }

    // char* test[3];
    // sprintf(test, "%d", change_context);
    // print_text(test, 0, 0, 0);
    // display_render_frame();
    // _delay_ms(1500);

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
          render_ferm_start(recipes[recipe_counter].recipe_name, change_context);
          render_recipe_and_submenus(recipes[recipe_counter].recipe_name,
                                     recipes[recipe_counter].desired_temp,
                                     recipes[recipe_counter].desired_hum,
                                     change_context,
                                     fermentation_started);
        }
        return;
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
        return;
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

        // Return to recipe selection
        state->next = recipe_selection;
        return;
      default: // This should never happen, because cases are predefined from 0-3
        LOG_DEBUG(DEFAULT, "FATAL ERROR");
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
    uint8_t touch_input = get_touch();

    _delay_ms(10);

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
    uint8_t touch_input = get_touch();

    _delay_ms(10);

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

void fermentation_init(void)
{
  change_context = 0;
  fermentation_started = 0;
}

int main(void)
{
  // run setup/initialization functions
  uart_init();
  i2c_init();
  _delay_ms(10);
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
