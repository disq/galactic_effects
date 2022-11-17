/*

MIT No Attribution

Copyright (c) 2020-2021 Mika Tuupola

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

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <hagl.h>
#include <rgb565.h>

static const uint8_t SPEED = 2;
static const uint8_t PIXEL_SIZE = 2;

static uint16_t angle;
static float sinlut[360];
static float coslut[360];

uint8_t texture_width, texture_height;
uint8_t *texture_data;

void rotozoom_init(uint8_t w, uint8_t h, const uint8_t *data888)
{
  /* Generate look up tables. */
  for (uint16_t i = 0; i < 360; i++) {
    sinlut[i] = sin(i * M_PI / 180);
    coslut[i] = cos(i * M_PI / 180);
  }

  texture_width = w;
  texture_height = h;

  // convert 888 to 0888 because of color_t being uint32_t
  size_t new_size = w*h*sizeof(color_t);
  texture_data = (uint8_t*)malloc(new_size);
  memset(texture_data, 0, new_size);

  for (int i = 0; i < w*h; i++) {
    texture_data[i*4+1] = data888[i*3];
    texture_data[i*4+2] = data888[i*3+1];
    texture_data[i*4+3] = data888[i*3+2];
  }
}

void rotozoom_close() {
  free(texture_data);
}

void rotozoom_render(hagl_backend_t const *display)
{
    float s, c, z;

//    s = sin(angle * M_PI / 180);
//    c = cos(angle * M_PI / 180);
    s = sinlut[angle];
    c = coslut[angle];
    z = s * 1.2;

    for (uint16_t x = 0; x < DISPLAY_WIDTH; x = x + PIXEL_SIZE) {
        for (uint16_t y = 0; y < DISPLAY_HEIGHT; y = y + PIXEL_SIZE) {

            /* Get a rotated pixel from the head image. */
            int16_t u = (int16_t)((x * c - y * s) * z) % texture_width;
            int16_t v = (int16_t)((x * s + y * c) * z) % texture_height;

            u = abs(u);
            if (v < 0) {
                v += texture_height;
            }

            color_t color = *(color_t*) (texture_data + texture_width * sizeof(color_t) * v + sizeof(color_t) * u);

            if (1 == PIXEL_SIZE) {
                hagl_put_pixel(display, x, y, color);
            } else {
                hagl_fill_rectangle(display, x, y, x + PIXEL_SIZE - 1, y + PIXEL_SIZE - 1, color);
            }
            // hagl_put_pixel(x, y, color);
        }
    }
}

void rotozoom_animate()
{
    angle = (angle + SPEED) % 360;
}
