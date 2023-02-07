#include "stdio.h"
#include "stdlib.h"
#include <stdbool.h>

struct demarshallFormat
{
	char *format;
	void (*method)(void *param);
};

void atCmdId(void *ptr)
{
	printf("atCmdId ID\n");
}

void atCmdRespId(void *ptr)
{
	printf("atCmdResp ID\n");
}

struct demarshallFormat demarshallers[] =
{
{ "\x08" "%b" "ID", atCmdId },
{ "\x88" "%b" "ID" "%b" "%l", atCmdRespId },
{ NULL, NULL } };

typedef struct stream
{
	enum
	{
		FRAME_START,
		FRAME_LENGTH_MSB,
		FRAME_LENGTH_LSB,
		FRAME_DATA,
		FRAME_CHECKSUM
	} xbeeState;

	unsigned length;
	unsigned received;

	char data[512];
	char *dataPtr;

} stream_t;

stream_t streams[2];

const char *directions[] =
{ "In", "Out" };

bool xbeeDemarshallParaform(char *format, size_t *bufferLength, char **buffer)
{
	printf("format %c\n", *format);
	switch (*format)
	{
	case 'l':
	{
		unsigned long value = 0;
		printf("Long:\n");
		for (int i = 0; i < 8; ++i)
		{
			if (*format == '\0' || *bufferLength == 0)
			{
				printf("Premature end of %l");
				return false;
			}
			value = (value << 4) + **buffer;
			printf("  Value=%ld\n", value);
			(*buffer)++;
			(*bufferLength)--;
		}
		return true;
	}

	case 'b':
	{
		unsigned char value = 0;
		printf("Byte:\n");
		if (*format == '\0' || *bufferLength == '\0')
		{
			printf("Premature end of %b");
			return false;
		}
		value = **buffer;
		printf("  Value=%d\n", value);
		(*buffer)++;
		(*bufferLength)--;
	}
		return true;

	}
	return false;
}

bool xbeeDemarshallMatch(char *format, char *buffer, size_t bufferLength)
{
	while (bufferLength && *format)
	{
		printf("buffer=%x, bufferLength=%d. format=%x\n", *buffer, bufferLength, *format);
		switch (*format)
		{
		case '%':
			format++;
			if (*format && bufferLength>0)
			{
				printf("Paraform\n");
				if (!xbeeDemarshallParaform(format, &bufferLength, &buffer))
				{
					return false;
				}
				format++;
			}
			else
			{
				printf("Premature end of format or buffer\n");
				return false;
			}
			break;
		default:
			if (*buffer == *format)
			{
				printf("Ate %c(%x)\n", *buffer, *buffer);
				buffer++;
				bufferLength--;
				format++;
			}
			else
			{
				// no match, try another
				return false;
			}
			break;
		}

		printf("End of buffer=%d or format=%d\n", *buffer, *format);

		if (bufferLength==0 && *format=='\0')
		{
			printf("Match completed\n");
			return true;
		}
	}
	return false;
}

void xbeeDemarshall(char *data, size_t bufferLength)
{
	struct demarshallFormat *ptr = demarshallers;

	printf("data %s\n", data);

	while (ptr->format)
	{
		printf("Checking %s\n", ptr->format);
		if (xbeeDemarshallMatch(ptr->format, data, bufferLength))
		{
			printf("Match\n");
			(ptr->method)(NULL);
			return;
		}
		ptr++;
	}

}

void xbeeDecoderInit(int direction)
{
	stream_t *stream = &streams[direction];
	stream->xbeeState = FRAME_START;
}

void xbeeDecoder(int direction, char ch)
{
	stream_t *stream = &streams[direction];

	printf("Uart %d received %c(%x) in state %d\n", direction, ch, ch,
			stream->xbeeState);

	switch (stream->xbeeState)
	{
	case FRAME_START:
		if (ch != '\x7e')
		{
			printf("Frame Sentinel Error on %s", directions[direction]);
			return;
		}
		stream->xbeeState = FRAME_LENGTH_MSB;
		printf("Switch to state %d\n", stream->xbeeState);
		break;

	case FRAME_LENGTH_MSB:
		stream->length = ch >> 16;
		stream->xbeeState = FRAME_LENGTH_LSB;
		printf("Switch to state %d\n", stream->xbeeState);
		break;

	case FRAME_LENGTH_LSB:
		stream->length += ch;
		stream->received=0;
		stream->dataPtr = stream->data;
		stream->xbeeState = FRAME_DATA;
		printf("Switch to state %d\n", stream->xbeeState);
		break;

	case FRAME_DATA:
		if (stream->length > 0)
		{
			*(stream->dataPtr++) = ch;
			stream->length--;
			stream->received++;
			if (stream->length == 0)
			{
				stream->xbeeState = FRAME_CHECKSUM;
				printf("Switch to state %d\n", stream->xbeeState);
			}
		}
		else
		{
			printf("Frame Length Error on %s", directions[direction]);
		}
		break;

	case FRAME_CHECKSUM:
		*(stream->dataPtr++) = '\0';
		xbeeDemarshall(stream->data, stream->received);
		stream->xbeeState = FRAME_START;
		return;
		break;
	}
}

