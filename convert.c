#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <hagl.h>
#include "convert.h"

uint8_t* convert_888(uint8_t w, uint8_t h, const uint8_t *data888)
{
  // convert 888 to 0888 because of color_t being uint32_t
  size_t new_size = w*h*sizeof(color_t);
  uint8_t *new_data = (uint8_t*)malloc(new_size);
  memset(new_data, 0, new_size);

  for (int i = 0; i < w*h; i++) {
    new_data[i*4+2] = data888[i*3];
    new_data[i*4+1] = data888[i*3+1];
    new_data[i*4+3] = data888[i*3+2];
  }
  return new_data;
}
