/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "stdio.h"
#include "stdlib.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include <ctype.h>

#include "xbee.h"
#include "xterm.h"

#define UART0_TX_PIN 0
#define UART0_RX_PIN 1
#define UART1_TX_PIN 4
#define UART1_RX_PIN 5

#define BAUD_RATE 115200
static const char msg2[] = "\x7E\x00\x04\x08\x4C\x49\x44\x1E";
static const char msg1[] =
		"\x7E\x00\x0D\x88\x4C\x49\x44\x00\x00\x00\x00\x00\x00\x00\x27\x01\x76";

bool connected = false;

int lines, columns;
bool fullDuplex = true;
bool xdecode = true;

int baudRates[] =
{ 110, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 38400, 57600, 115200,
		128000, 256000 };
int baudIndex = 6;
int baudIndexLength = sizeof(baudRates) / sizeof(int);

void updateConfigDisplay()
{
	xtermSaveLocation();

	xtermSetLocation(lines, 1);

	printf("d)");
	printf(fullDuplex ? "FDX" : "HDX");

	xtermSetLocation(lines, 7);
	printf("b)");
	printf("%-6d", baudRates[baudIndex]);

	xtermRestoreLocation();
}

int charCount = 0;

#ifdef trash
typedef struct stream
{
	enum
	{
		FRAME_START,
		FRAME_LENGTH_MSB,
		FRAME_LENGTH_LSB,
		FRAME_TYPE,
		AT_COMMAND = 0x08,
		AT_COMMAND_QUEUE = 0x09,
		TRANSMIT_REQUEST = 0x10,
		EXPLICIT_ADDRESSING_COMMAND_FRAME = 0x11,
		REMOTE_AT_COMMAND_REQUEST = 0x17,
		CREATE_SOURCE_ROUTE = 0x21,
		REGISTER_JOINING_DEVICE = 0x24,
		AT_COMMAND_RESPONSE = 0x88,
		MODEM_STATUS = 0x8a,
		TRANSMIT_STATUS = 0x8B,
		RECEIVE_PACKET = 0x90,
		EXPLICIT_RX_INDICATOR = 0x91,
		IO_DATA_SAMPLE_RX_INDICATOR = 0x92,
		XBEE_SENSOR_READ_INDICATOR = 0x94,
		NODE_IDENTIFICATION_INDICATOR = 0x95,
		REMOTE_AT_COMMAND_RESPONSE = 0x97,
		EXTENDED_MODEM_STATUS = 0x98,
		OVER_THE_AIR_FIRMWARE_UPDATE_STATUS = 0xA0,
		ROUTER_RECORD_INDICATOR = 0xA1,
		MANY_TO_ONE_ROUTE_REQUEST_INDICATOR = 0xA3,
		JOIN_NOTIFICATION_STATUS = 0xA5

	} xbeeState;

	unsigned length;

	enum
	{
		CMD1, CMD2
	} at_cmd;

	char cmd1;
	char cmd2;
} stream_t;

stream_t streams[2];

const char *directions[] =
{ "In", "Out" };

int xbeeATCommand(stream_t *stream, char ch)
{
	switch (stream->at_cmd)
	{
	case CMD1:
		stream->cmd1 = ch;
		break;
	case CMD2:
		stream->cmd2 = ch;
		if (stream->cmd1 == 'I' && stream->cmd2 == 'D' )
		{

		}
			break;

	}
	return AT_COMMAND;
}

int xbeeAtCommandQueue(stream_t *stream, char ch)
{
	return AT_COMMAND_QUEUE;
}

void xbeeDecoder(int direction, char ch)
{
	stream_t *stream = &streams[direction];

	switch (stream->xbeeState)
	{
	case FRAME_START:
		if (ch != '\x7e')
		{
			printf("Frame Error on %s", directions[direction]);
			return;
		}
		stream->xbeeState = FRAME_LENGTH_MSB;
		break;

	case FRAME_LENGTH_MSB:
		stream->length = ch >> 16;
		stream->xbeeState = FRAME_LENGTH_LSB;
		break;
	case FRAME_LENGTH_LSB:
		stream->length += ch;
		stream->xbeeState = FRAME_TYPE;
		break;

	case FRAME_TYPE:
		stream->xbeeState = ch;
		break;

	case AT_COMMAND:
		stream->xbeeState = xbeeATCommand(stream, ch);
		break;
	case AT_COMMAND_QUEUE:
		stream->xbeeState = xbeeAtCommandQueue(stream, ch);
		break;
	case TRANSMIT_REQUEST:
		stream->xbeeState = xbeeATCommand(stream, ch);
		break;
	case EXPLICIT_ADDRESSING_COMMAND_FRAME:
		stream->xbeeState = xbeeATCommand(stream, ch);
		break;
	case AT_COMMAND_RESPONSE:
		stream->xbeeState = xbeeATCommand(stream, ch);
		break;
	case REMOTE_AT_COMMAND_REQUEST:
		stream->xbeeState = xbeeATCommand(stream, ch);
		break;
	case CREATE_SOURCE_ROUTE:
		stream->xbeeState = xbeeATCommand(stream, ch);
		break;
	case REGISTER_JOINING_DEVICE:
		stream->xbeeState = xbeeATCommand(stream, ch);
		break;
	}

}
#endif

void displayChar(int direction, char ch)
{
	xtermInverse(direction);
	if (xdecode)
	{
		xbeeDecoder(direction, ch);
	}
	else
	{
		printf("%c", ch);
	}
	xtermInverse(false);
}

void send()
{
	uart_write_blocking(uart0, (uint8_t*) &msg1, sizeof msg1); // Send on uart0, receive on uart1
	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
	sleep_ms(500);

	uart_write_blocking(uart1, (uint8_t*) &msg2, sizeof msg2); // Send on uart0, receive on uart1
	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
	sleep_ms(500);
}

int main()
{
	stdio_init_all();

	if (cyw43_arch_init())
	{
		printf("WiFi init failed");
		return -1;
	}

	gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);
	gpio_set_function(UART1_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(UART1_RX_PIN, GPIO_FUNC_UART);

	uart_init(uart0, BAUD_RATE);
	uart_init(uart1, BAUD_RATE);

	while (true)
	{
		if (stdio_usb_connected())
		{
			if (connected == false)
			{
				printf("Started\n");

				xbeeDecoderInit(0);
				xbeeDecoderInit(1);

				// state transition.  We're now connected
				xtermClearScreen();
				xtermLowerLeftCorner();
				xtermGetScreenLocation(&lines, &columns);
				xtermSetScrollingRegion(1, lines - 2);

				updateConfigDisplay();

				connected = true;
			}
		}
		else
		{
			connected = false;
		}

		while (uart_is_readable(uart0) || uart_is_readable(uart0))
		{
			while (uart_is_readable(uart0))
			{
				char ch = uart_getc(uart0);
				displayChar(0, ch);
			}

			while (uart_is_readable(uart1))
			{
				char ch = uart_getc(uart1);
				displayChar(1, ch);
			}
		}

// Process any command keys hit on the console
		int ch = getchar_timeout_us(0);
		if (ch != PICO_ERROR_TIMEOUT)
		{
			// d - duplex display
			if (tolower(ch) == 'd')
			{
				fullDuplex = !fullDuplex;
			}
			else

			// b - baud rate rotation
			if (tolower(ch) == 'b')
			{
				++baudIndex;
				baudIndex %= baudIndexLength;
			}
			else if (tolower(ch) == 'x')
			{
				xdecode = true;
			}

			else if (tolower(ch) == 't')
			{
				printf("Sending\n");
				send();
			}

			// After we process commands, update the config display
			updateConfigDisplay();
		}

	}
}
