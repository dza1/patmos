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

#define RAD 5                         // radius in pixel
#define RADCOLOR 0b001100 | 0b11 << 6 // color of radius  /BRG

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define BACKGROUND 0b110000 | 0b11 << 6

volatile uint8_t (*vga_arr)[DISPLAY_HEIGTH][LINE_WIDTH] = 400000;
void drawCircle(int m_x, int m_y, int r, uint8_t color);
void drawChanges(int m_x_old, int m_y_old, int m_x_new, int m_y_new, int r,
                 uint8_t color);
void calcStartStop(int m_x, int m_y, int y, int r, int *x_start, int *x_stop);
int main() {

  uint8_t color = 15;
  volatile _SPM int *led_ptr = (volatile _SPM int *)PATMOS_IO_LED;

  // Plot a background
  for (int r = 0; r < DISPLAY_HEIGTH; r++) {
    for (int c = 0; c < LINE_WIDTH; c++) {
      if ((c >> 1) % 7 == 0) {
        (*vga_arr)[r][c] = color | 0b11 << 6;
      } else {
        (*vga_arr)[r][c] = (color >> 2) | 0b11 << 6;
      }
    }
  }

  //drawCircle(RAD, RAD, RAD, RADCOLOR);
  int m_x_new, m_x_old = RAD;
  int m_y_new, m_y_old = RAD;
  int dir_x = 1, dir_y = 1;

  for (;;) {
    if ((m_x_old + RAD) >= LINE_WIDTH || (m_x_old - RAD) < 0) {
      dir_x = -dir_x;
    }
    if ((m_y_old + RAD) >= DISPLAY_HEIGTH || (m_y_old - RAD) < 0) {
      dir_y = -dir_y;
    }
    m_x_new = (m_x_old + dir_x);
    m_y_new = (m_y_old + dir_y);
    drawChanges(m_x_old, m_y_old, m_x_new, m_y_new, RAD, RADCOLOR);
    m_x_old = m_x_new;
    m_y_old = m_y_new;
  }
}

void drawCircle(int m_x, int m_y, int r, uint8_t color) {

  for (int y = m_y - r; y < (m_y + r); y++) {
    int y_rel = y - m_y;
    int x_start = m_x - sqrt(r * r - y_rel * y_rel);
    int x_stop = m_x + sqrt(r * r - y_rel * y_rel);
    for (int x = x_start; x < x_stop; x++) {
      if (x >= 0 && x < LINE_WIDTH && y >= 0 && y < DISPLAY_HEIGTH) {
        (*vga_arr)[y][x] = color;
      }
    }
  }
}

void drawChanges(int m_x_old, int m_y_old, int m_x_new, int m_y_new, int r,
                 uint8_t color) {
  for (int y = MIN(m_y_old, m_y_new) - r; y <= MAX(m_y_old, m_y_new) + r && y < DISPLAY_HEIGTH; y++) {
    int x_start_old;
    int x_start_new;
    int x_stop_old;
    int x_stop_new;

    calcStartStop(m_x_old, m_y_old, y, r, &x_start_old, &x_stop_old);
    calcStartStop(m_x_new, m_y_new, y, r, &x_start_new, &x_stop_new);

    for (int x = MIN(m_x_old, m_x_new) - r; x <= MAX(m_x_old, m_x_new) + r && x < LINE_WIDTH; x++) {
      if (x < 0) {
        continue;
      }
      if ( x_start_old <= x && x <= x_stop_old && !(x_start_new <= x && x <= x_stop_new)) { // we need do delete the old cycle
        (*vga_arr)[y][x] = BACKGROUND;
      } else if ( !(x_start_old <= x && x <= x_stop_old) && x_start_new  <= x && x <= x_stop_new) { // wee need do write a new cycle
        (*vga_arr)[y][x] = color;
      }
    }
  }
}

void calcStartStop(int m_x, int m_y, int y, int r, int *x_start, int *x_stop) {
  if ((m_y - r) <= y && (m_y + r) >= y) {
    int y_rel = y - m_y;
    *x_start = m_x - sqrt(r * r - y_rel * y_rel);
    *x_stop = m_x + sqrt(r * r - y_rel * y_rel);
  } else {
    *x_start = 0;
    *x_stop = 0;
  }
}