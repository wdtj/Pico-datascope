/*
 * xterm.c
 *
 *  Created on: Feb 6, 2023
 *      Author: waynej
 */

#include "stdio.h"
#include "stdlib.h"
#include <stdbool.h>
#include "pico/stdio.h"
#include <ctype.h>
#include "xterm.h"

int xtermGetScreenLocation(int *lines, int *columns)
{
	int ch;
	enum
	{
		CMD_SENT, ESC_RECEIVED, LINES, COLUMNS
	} state;

	*lines = 0;
	*columns = 0;

	printf("\e[6n");
	state = CMD_SENT;

	while (true)
	{
		ch = getchar_timeout_us(30);
		if (ch < 0)
		{
			continue;
		}

		switch (state)
		{
		case CMD_SENT:
			if (ch == '\e')
			{
				state = ESC_RECEIVED;
			}
			else
			{
				return -1;
			}
			break;
		case ESC_RECEIVED:
			if (ch == '[')
			{
				state = LINES;
			}
			else
			{
				return -1;
			}
			break;
		case LINES:
			if (isdigit(ch))
			{
				*lines = *lines * 10 + ch - '0';
			}
			else if (ch == ';')
			{
				state = COLUMNS;
			}
			else
			{
				return -1;
			}
			break;
		case COLUMNS:
			if (isdigit(ch))
			{
				*columns = *columns * 10 + ch - '0';
			}
			else if (ch == 'R')
			{
				return 0;
			}
			else
			{
				return -1;
			}
			break;
		}
	}
	return -1;
}

void xtermClearScreen()
{
	printf("\e[r");
}

void xtermSetLocation(int line, int column)
{
	printf("\e[%d;%dH", line, column);
}

void xtermLowerLeftCorner()
{
	xtermSetLocation(999, 999);
}

int xtermSetScrollingRegion(int lines, int columns)
{
	// Set window lines 0 - 22
	printf("\e[%d;%dr", lines, columns);
	return lines;
}

void xtermSaveLocation()
{
	printf("\e7");
}

void xtermRestoreLocation()
{
	printf("\e8");
}

void xtermInverse(bool set)
{
	printf("\e[%cm", set ? '7' : '0');
}

