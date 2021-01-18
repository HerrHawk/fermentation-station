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
  // integer hystherese
  int32_t temp_hyst;
  // in %
  uint32_t desired_hum;
  // integer hystherese
  uint32_t hum_hyst;
};

void check_hum(struct recipe* recipe);
void check_temp(struct recipe* recipe);
