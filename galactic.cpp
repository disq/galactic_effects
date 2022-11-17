#include "galactic_unicorn.hpp"

using namespace pimoroni;
extern GalacticUnicorn galactic_unicorn;

#ifdef __cplusplus
extern "C" {
#endif

// couldn't get these to work, see galactic.h
//const uint8_t galactic_sw_brightness_up = galactic_unicorn.SWITCH_BRIGHTNESS_UP;
//const uint8_t galactic_sw_brightness_down = galactic_unicorn.SWITCH_BRIGHTNESS_DOWN;
//const uint8_t galactic_sw_vol_up = galactic_unicorn.SWITCH_VOLUME_UP;
//const uint8_t galactic_sw_vol_down = galactic_unicorn.SWITCH_VOLUME_DOWN;
//const uint8_t galactic_sw_sleep = galactic_unicorn.SWITCH_SLEEP;

bool galactic_is_button_pressed(uint8_t button) {
  return galactic_unicorn.is_pressed(button);
}

void galactic_set_brightness(float value) {
  galactic_unicorn.set_brightness(value);
}

void galactic_adj_brightness(float diff) {
  galactic_unicorn.adjust_brightness(diff);
}

#ifdef __cplusplus
}
#endif
