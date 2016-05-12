//#include <stdio.h>

#include "5kk03.h"
#include "jpeg.h"

extern unsigned int *input_buffer;
extern int vld_count;

unsigned int FGETC()
{
	//unsigned int c = ((input_buffer[vld_count / 4] << (8 * (vld_count % 4))) >> 24) & 0x00ff;
	//unsigned int c = ((input_buffer[vld_count / 4] << (8 * (3-(vld_count % 4)))) >> 24) & 0x00ff;
	unsigned int c = ((input_buffer[vld_count >> 2] << ((3-vld_count % 4)<<3)) >> 24) & 0x00ff;

	vld_count++;
	return c;
}

int FSEEK(int offset, int start)
{
	vld_count += offset + (start - start);	/* Just to use start... */
	return 0;
}

int FTELL()
{
	return vld_count;
}
