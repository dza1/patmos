/*
    This is a minimal C program executed on the FPGA version of Patmos.
    An embedded Hello World program: a blinking LED.

    Additional to the blinking LED we write to the UART '0' and '1' (if available).

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include <stdint.h>
#include <stdbool.h>

#define LOOP_DELAY 2000
#define LINE_WIDTH  640
#define DISPLAY_HEIGTH  480


volatile uint8_t (*base)[DISPLAY_HEIGTH][LINE_WIDTH] = 800000;

int main()
{


	int i, j;
	uint8_t color= 3;

	for (int r; r < DISPLAY_HEIGTH; r++)
	{
		for (int c; c < LINE_WIDTH; c++)
		{

			if ((c >> 3) % 2 == 0)
			{
				(*base)[r][c] =color;
			}
			else
			{
				(*base)[r][c] = color >> 2;
			}
		}
	}

}