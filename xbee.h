/*
 * pico_xbee.h
 *
 *  Created on: Feb 6, 2023
 *      Author: waynej
 */

#ifndef XBEE_H_
#define XBEE_H_

void xbeeDecoderInit(int direction);
void xbeeDecoder(int direction, char ch);

#endif /* XBEE_H_ */
