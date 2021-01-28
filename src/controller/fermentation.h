#pragma once
#include <stdio.h>

/*
 * Hystherese: optimal temp is 60°
 * -> as soon as min hyst : 55° is reached -> begin heating up
 * -> as soon as max hyst : 65° is reached -> begin lowering heat
 */

struct recipe
{
  char recipe_name[50];
  // in °C (2289 => 22,89°C)
  int32_t desired_temp;
  // in %
  uint32_t desired_hum;
  // integer hystherese
  uint32_t hum_hyst;
};

uint32_t check_hum(struct recipe* recipe);
int32_t check_temp(struct recipe* recipe);
int32_t pid_temp_calculate(int32_t setpoint, double kP, double kI, double kD);
int32_t pid_hum_calculate(int32_t setpoint, double kP, double kI, double kD);
int32_t pid_calculate(struct recipe* recipe);