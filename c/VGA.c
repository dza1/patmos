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
#define RADCOLOR 48|0b10<<6 // color of radius

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define BACKGROUND 0

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




drawCircle(RAD, (DISPLAY_HEIGTH/2), RAD, RADCOLOR);
int m_x_new, m_x_old=RAD;
int m_y_new, m_y_old=DISPLAY_HEIGTH/2;

for(int i=RAD; i<=LINE_WIDTH-RAD; i++){
  m_x_new = (m_x_old + 1)%(LINE_WIDTH-2*RAD)+RAD;
  m_y_new = (m_y_old + 1)%(DISPLAY_HEIGTH-2*RAD)+RAD;
  drawChanges(m_x_old, m_y_old, m_x_new, m_y_new, RAD, RADCOLOR);
  m_x_old=m_x_new;
  m_y_old=m_y_new;
}

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


void drawChanges(int m_x_old, int m_y_old, int m_x_new, int m_y_new, int r, uint8_t color){
  for(int y = MIN(m_y_old, m_y_new)-r; y>=MAX(m_y_old, m_y_new)+r; y++){
    int* x_start_old;
    int* x_start_new;
    int* x_stop_old;
    int* x_stop_new;

    calcStartStop(m_x_old, m_y_old, y, r, x_start_old, x_stop_old);
    calcStartStop(m_x_new, m_y_new, y, r, x_start_new, x_stop_new);

    for(int x = MIN(m_x_old, m_x_new)-r; x>=MAX(m_x_old, m_x_new)+r; x++){
      if(x>=x_start_old && x<x_stop_old && !(x>=x_start_new && x<x_stop_new)){
        (*vga_arr)[y][x] = BACKGROUND;
      }else if(x>=x_start_new && x<x_stop_new){
        (*vga_arr)[y][x] = color;
      }
    }
  }
}


void calcStartStop(int m_x, int m_y, int y, int r, int* x_start, int* x_stop){
  if((m_y-r)<=y && (m_y+r)>=y){
    int y_rel=y-m_y;
    *x_start=m_x-sqrt(r*r-y_rel*y_rel);
    *x_stop=m_x+sqrt(r*r-y_rel*y_rel);
  }else{
    *x_start=0;
    *x_stop=0;
  }
}