//extern const uint8_t galactic_sw_brightness_up;
//extern const uint8_t galactic_sw_brightness_down;
//extern const uint8_t galactic_sw_vol_up;
//extern const uint8_t galactic_sw_vol_down;
//extern const uint8_t galactic_sw_sleep;
// could not get the extern consts to work (see galactic.cpp)

const uint8_t galactic_sw_brightness_up = 21;
const uint8_t galactic_sw_brightness_down = 26;
const uint8_t galactic_sw_vol_up = 7;
const uint8_t galactic_sw_vol_down = 8;
const uint8_t galactic_sw_sleep = 27;

extern bool galactic_is_button_pressed(uint8_t button);
extern void galactic_set_brightness(float value);
extern void galactic_adj_brightness(float diff);
