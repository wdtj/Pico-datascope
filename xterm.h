/*
 * xterm.h
 *
 *  Created on: Feb 6, 2023
 *      Author: waynej
 */

#ifndef XTERM_H_
#define XTERM_H_

#include <stdbool.h>

void xtermSaveLocation();
void xtermSetLocation(int line, int column);
void xtermRestoreLocation();
void xtermInverse(bool set);
void xtermClearScreen();
void xtermLowerLeftCorner();
int xtermGetScreenLocation(int *lines, int *columns);
int xtermSetScrollingRegion(int lines, int columns);

#endif /* XTERM_H_ */
