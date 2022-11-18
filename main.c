/*

MIT No Attribution

Copyright (c) 2021 Mika Tuupola
Copyright (c) 2022 Kemal Hadimli

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

-cut-

SPDX-License-Identifier: MIT-0

*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <pico/stdlib.h>
#include <hardware/clocks.h>

#include <sys/time.h>

#include <hagl_hal.h>
#include <hagl.h>
#include <font6x9.h>
#include <fps.h>
#include <aps.h>
#include "galactic.h"

#include "metaballs.h"
#include "plasma.h"
#include "rotozoom.h"
#include "deform.h"
#include "head.h"

static uint8_t effect = 0;
volatile signed int switch_flag = 1;
static fps_instance_t fps;
static aps_instance_t bps;

static uint8_t *buffer;
static hagl_backend_t backend;
static hagl_backend_t *display;

static const uint64_t US_PER_FRAME_120_FPS = 1000000 / 120;
static const uint64_t US_PER_FRAME_60_FPS = 1000000 / 60;
static const uint64_t US_PER_FRAME_30_FPS = 1000000 / 30;
static const uint64_t US_PER_FRAME_25_FPS = 1000000 / 25;

const uint32_t autoswitch_timer_ms = 10000; // 10 seconds
const uint32_t debounce_ms = 300; // 300 ms
bool init_done = false;
static bool do_auto_light = true;

typedef struct {
    const char *name;
    void (*init_func)();
    void (*animate_func)();
    void (*render_func)(hagl_backend_t const *display);
    void (*close_func)();
} effects_t;

#define DEMO_COUNT 3

static void rotozoom_initwrapper() { rotozoom_init(HEAD_WIDTH, HEAD_HEIGHT, head); }
static void deform_initwrapper() { deform_init(HEAD_WIDTH, HEAD_HEIGHT, head); }
static void plasma_initwrapper() { plasma_init(display); }

static effects_t demos[DEMO_COUNT] = {
    {"Metaballs", metaballs_init, metaballs_animate, metaballs_render, metaballs_close},
    {"Plasma", plasma_initwrapper, plasma_animate, plasma_render, plasma_close},
    {"Rotozoom", rotozoom_initwrapper, rotozoom_animate, rotozoom_render, rotozoom_close},
//    {"Deform", deform_initwrapper, deform_animate, deform_render, deform_close},
};

void auto_adjust_brightness() {
  float light_level = ((float)galactic_get_light_level())/4095.0f;
  galactic_set_brightness(light_level + 0.15f);
}

bool switch_timer_callback(struct repeating_timer *t) {
    switch_flag = 1;
    return true;
}

bool light_timer_callback(struct repeating_timer *t) {
  if (do_auto_light) auto_adjust_brightness();
  return true;
}

void static inline switch_demo() {
    signed int switch_val = switch_flag;
    switch_flag = 0;

    if (init_done && effect>=0) {
      printf("Closing %s\n", demos[effect].name);
      demos[effect].close_func();
    }

    signed int nexteff = effect + switch_val;
    if (nexteff < 0) {
        nexteff = DEMO_COUNT - 1;
    } else if (nexteff >= DEMO_COUNT) {
        nexteff = 0;
    }
    effect = nexteff;
//    effect = (effect + switch_val) % DEMO_COUNT;

    printf("Starting %s\n", demos[effect].name);
    demos[effect].init_func();
    aps_init(&bps);
}

int main()
{
    size_t bytes = 0;
    struct repeating_timer switch_timer;
    struct repeating_timer light_timer;
    bool do_timer = true;

//     set_sys_clock_khz(133000, true);
//     clock_configure(
//         clk_peri,
//         0,
//         CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
//         133000 * 1000,
//         133000 * 1000
//     );

    stdio_init_all();
//    galactic_unicorn.init(); // this is done in hagl_hal_init() so no need to do here

    fps_init(&fps);
    display = hagl_init();
    hagl_clear(display);

    auto_adjust_brightness();

  /* Change demo every 10 seconds. */
    if (do_timer) add_repeating_timer_ms(autoswitch_timer_ms, switch_timer_callback, NULL, &switch_timer);

    add_repeating_timer_ms(1000, light_timer_callback, NULL, &light_timer);

    switch_demo();
    init_done = true;

    while (1) {
        uint64_t start = time_us_64();

        bool brightness_up = galactic_is_button_pressed(galactic_sw_brightness_up);
        bool brightness_down = galactic_is_button_pressed(galactic_sw_brightness_down);
        if (brightness_up && brightness_down) {
          galactic_set_brightness(0.0f);
          demos[effect].animate_func();
          demos[effect].render_func(display);
          hagl_flush(display);
          do_auto_light = true;
          sleep_ms(100);
          auto_adjust_brightness();
          printf("Auto-brightness re-enabled\n");
          demos[effect].animate_func();
          demos[effect].render_func(display);
          hagl_flush(display);
          sleep_ms(500);
          continue; // we've messed with everything this loop, restart
        } else if (brightness_up) {
          galactic_adj_brightness(+0.01f);
          do_auto_light = false;
        } else if (brightness_down) {
          galactic_adj_brightness(-0.01f);
          do_auto_light = false;
        }
        bool redo_timer = false;
        if(galactic_is_button_pressed(galactic_sw_vol_up)) {
          // switch to next
          switch_flag = 1;
          redo_timer = do_timer;
          sleep_ms(debounce_ms);
        }
        if(galactic_is_button_pressed(galactic_sw_vol_down)) {
          // switch to previous
          switch_flag = -1;
          redo_timer = do_timer;
          sleep_ms(debounce_ms);
        }
        if(galactic_is_button_pressed(galactic_sw_sleep)) {
          printf(do_timer ? "Cancelling timer\n" : "Enabling timer\n");
          do_timer = !do_timer;
          if (do_timer) redo_timer = true;
          else cancel_repeating_timer(&switch_timer);
          sleep_ms(debounce_ms);
        }
//        printf("light: %d\n", galactic_get_light_level());
        if (redo_timer) {
          // restart the timer
          cancel_repeating_timer(&switch_timer);
          add_repeating_timer_ms(autoswitch_timer_ms, switch_timer_callback, NULL, &switch_timer);
        }

        demos[effect].animate_func();
        demos[effect].render_func(display);

        /* Flush back buffer contents to display. NOP if single buffering. */
        bytes = hagl_flush(display);

        aps_update(&bps, bytes);
        fps_update(&fps);

        /* Print the message in console and switch to next demo. */
        if (switch_flag != 0) {
            printf("%s at %d fps / %d kBps\r\n", demos[effect].name, (uint32_t)fps.current, (uint32_t)(bps.current / 1024));
            switch_demo();
        }

        /* Cap the demos to 60 fps. This is mostly to accommodate to smaller */
        /* screens where plasma will run too fast. */
        busy_wait_until(start + US_PER_FRAME_120_FPS);
    };

    return 0;
}
