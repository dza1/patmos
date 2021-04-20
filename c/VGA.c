/*
    This is a minimal C program executed on the FPGA version of Patmos.
    An embedded Hello World program: a blinking LED.

    Additional to the blinking LED we write to the UART '0' and '1' (if
   available).

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "include/bootable.h"
#include <machine/spm.h>

#define LOOP_DELAY 2000
#define LINE_WIDTH 640
#define DISPLAY_HEIGTH 480

#define RAD 100     // radius in pixel
#define RADCOLOUR 48|0b10<<6 // color of radius

volatile uint8_t (*vga_arr)[DISPLAY_HEIGTH][LINE_WIDTH] = 400000;
void drawCircle(int m_x, int m_y, int r,uint8_t color);
int main() {


  uint8_t color = 15;
  volatile _SPM int *led_ptr = (volatile _SPM int *)PATMOS_IO_LED;




    for (int r = 0; r < DISPLAY_HEIGTH; r++) {
      for (int c = 0; c < LINE_WIDTH; c++) {
        // if ((r >> 1) % 7 == 0) {
        if ((c >> 1) % 7 == 0) {
          (*vga_arr)[r][c] = color|0b11<<6;
        } else {
          (*vga_arr)[r][c] = (color >> 2)|0b11<<6;
        }
      }
    }


drawCircle( (LINE_WIDTH/2), (DISPLAY_HEIGTH/2), RAD,RADCOLOUR);


  int i, j;
  for (;;) {

    for (int r = 0; r < DISPLAY_HEIGTH; r++) {
      for (int c = 0; c < LINE_WIDTH; c++) {
        UART_DATA = 'A';
        for (i = LOOP_DELAY; i != 0; --i)
          for (j = LOOP_DELAY; j != 0; --j)
            *led_ptr = 1;

        UART_DATA = (*vga_arr)[r][c];
        for (i = LOOP_DELAY; i != 0; --i)
          for (j = LOOP_DELAY; j != 0; --j)
            *led_ptr = 0;
      } 
    }
  }
}

void drawCircle(int m_x, int m_y, int r,uint8_t color){

  for (int y = m_y - r; y < (m_y +r); y++) {
    int y_rel=y-m_y;
    int x_start=m_x-sqrt(r*r-y_rel*y_rel);
    int x_stop=m_x+sqrt(r*r-y_rel*y_rel);
    for (int x = x_start; x < x_stop; x++) {
      if(x >= 0 && x < LINE_WIDTH && y >= 0 && y< DISPLAY_HEIGTH){
      (*vga_arr)[y][x] = color;
      }
    }
  }
}