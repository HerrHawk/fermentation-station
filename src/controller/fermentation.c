#include "fermentation.h"
#include "../drivers/bme280.h"
#include "../globals.h"
#include "../helpers.h"
#include "../logging.h"
#include "actor.h"
#include <avr/io.h>
#include <util/delay.h>

struct pid_data_holder
{
  int32_t prev_temp;
  int32_t prev_temp_error;
  int32_t integral_temp;

  uint32_t prev_hum;
  int32_t prev_hum_error;
  uint32_t integral_hum
} pid_data = { -1, -1, 0, -1, -1, 0 };

int32_t check_temp(struct recipe* current_recipe)
{
  double kP = 3;
  double kI = 0.1;
  double kD = 1.0;

  int32_t current_temp = bme280_read_temp();
  LOG_DEBUG(TEMPERATURE, "Temp %d ", current_temp);

  int32_t control_output = pid_temp_calculate(current_recipe->desired_temp, kP, kI, kD);
  LOG_DEBUG(DEFAULT, "Pid %d", control_output);

  if (control_output > 255) {
    activate_heating_pwm();
    update_heating_dutycycle(0xFF);
  } else if (control_output < 0) {
    deaktivate_heating_pwm();
    update_heating_dutycycle(0x00);
  } else {
    activate_heating_pwm();
    update_heating_dutycycle(control_output);
  }

  return bme280_read_temp();
}

uint32_t check_hum(struct recipe* current_recipe)
{
  double kP = 1;
  double kI = 0.1;
  double kD = 1.0;
  // Humidity does not affect some recipes ->
  // if the value -1 is set in the recipe humidity can be ignored
  if (current_recipe->desired_hum == -1) {
    return current_recipe->desired_hum;
  }

  int32_t control_output = pid_hum_calculate(current_recipe->desired_hum, kP, kI, kD);

  if (control_output > 255) {
    activate_humidifier();
  } else {
    deactivate_humidifier();
  }

  return bme280_read_hum();
}

int32_t pid_temp_calculate(int32_t setpoint, double kP, double kI, double kD)
{
  // fixed time interval given by counter
  int32_t time_change = 1;
  int32_t current_temp = bme280_read_temp(); // eq. input
  int32_t error = setpoint - current_temp;

  // "Do once" on initialization
  if (pid_data.prev_temp == -1) {
    pid_data.prev_temp = current_temp;
    pid_data.prev_temp_error = error;
  }

  if (-20 < error && error < 20) {
    pid_data.integral_temp += error * time_change;
  }

  int32_t gainP = kP * error;
  int32_t gainI = kI * pid_data.integral_temp;
  int32_t gainD = kD * ((error - pid_data.prev_temp_error) / time_change);

  pid_data.prev_temp = current_temp;
  pid_data.prev_temp_error = error;

  return gainP + gainI + gainD;
}

int32_t pid_hum_calculate(int32_t setpoint, double kP, double kI, double kD)
{
  int32_t current_hum = bme280_read_hum(); // eq. input
  int32_t error = setpoint - current_hum;

  // "Do once" on initialization
  if (pid_data.prev_hum == -1) {
    pid_data.prev_hum = current_hum;
    pid_data.prev_hum_error = error;
  }

  // fixed time interval given by counter
  int32_t time_change = 1;

  // Prevent Integral Wind Up by reducing the controllable range
  if (-100 < error && error < 100) {
    pid_data.integral_hum += error * time_change;
  }

  int32_t gainP = kP * error;
  int32_t gainI = kI * pid_data.integral_hum;
  int32_t gainD = kD * ((error - pid_data.prev_hum_error) / time_change);

  pid_data.prev_hum = current_hum;
  pid_data.prev_hum_error = error;

  return gainP + gainI + gainD;
}
