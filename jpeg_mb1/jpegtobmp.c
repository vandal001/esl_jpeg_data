//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
#include <comik.h>

/* real declaration of global variables here */
/* see jpeg.h for more info			*/

#include "jpeg.h"

/* descriptors for 3 components */
cd_t comp[3];
/* decoded DCT blocks buffer */
PBlock *MCU_buff[10];
/* components of above MCU blocks */
int MCU_valid[10];

/* quantization tables */
PBlock *QTable[4];
int QTvalid[4];

/* Video frame size     */
int x_size, y_size;
/* down-rounded Video frame size (integer MCU) */
int rx_size, ry_size;
/* MCU size in pixels   */
int MCU_sx, MCU_sy;
/* picture size in units of MCUs */
int mx_size, my_size;
/* number of components 1,3 */
int n_comp;

/* MCU after color conversion */
unsigned char *ColorBuffer;
/* complete final RGB image */
//unsigned char *FrameBuffer;
/* scratch frequency buffer */
FBlock *FBuff;
/* scratch pixel buffer */
PBlock *PBuff;
/* frame started ? current component ? */
int in_frame, curcomp;
/* current position in MCU unit */
int MCU_row, MCU_column;

/* input  File stream pointer   */
//FILE *fi;

/* stuff bytes in entropy coded segments */
int stuffers = 0;
/* bytes passed when searching markers */
int passed = 0;

int verbose = 0;

/* Extra global variables for 5kk03 */

int vld_count = 0;		/* Counter used by FGET and FSEEK in 5kk03.c */

/* End extra global variables for 5kk03 */

/*-----------------------------------------------------------------*/
/*		MAIN		MAIN		MAIN		   */
/*-----------------------------------------------------------------*/

//int JpegToBmp(char *file1, char *file2)
int JpegToBmp(unsigned int *input, volatile unsigned int *output)
{
	unsigned int aux, mark;
	int n_restarts, restart_interval, leftover;	/* RST check */
	int i, j;

	/* First find the SOI marker: */
	aux = get_next_MK(input);
	if (aux != SOI_MK)
        return 1;

	in_frame = 0;
	restart_interval = 0;
	for (i = 0; i < 4; i++)
		QTvalid[i] = 0;

	/* Now process segments as they appear: */
	do {
		mark = get_next_MK(input);

		switch (mark) {
		case SOF_MK:
			in_frame = 1;
			get_size(input);	/* header size, don't care */

			/* load basic image parameters */
			FGETC();	/* precision, 8bit, don't care */
			y_size = get_size(input);

			x_size = get_size(input);


			n_comp = FGETC();	/* # of components */

			for (i = 0; i < n_comp; i++) {
				/* component specifiers */
				comp[i].CID = FGETC();
				aux = FGETC();
				comp[i].HS = first_quad(aux);
				comp[i].VS = second_quad(aux);
				comp[i].QT = FGETC();
			}

			if (init_MCU() == -1)
                return 1;

			/* dimension scan buffer for YUV->RGB conversion */
			//FrameBuffer = (unsigned char *)mk_malloc((size_t) x_size * y_size * n_comp);
			ColorBuffer = (unsigned char *)mk_malloc((size_t) MCU_sx * MCU_sy * n_comp);
			FBuff = (FBlock *) mk_malloc(sizeof(FBlock));
			PBuff = (PBlock *) mk_malloc(sizeof(PBlock));

			if (/*(FrameBuffer == NULL) ||*/ (ColorBuffer == NULL) || (FBuff == NULL) || (PBuff == NULL)) {
                return 1;
			}
			break;

		case DHT_MK:
			if (load_huff_tables(input) == -1)
                return 1;
			break;

		case DQT_MK:
			if (load_quant_tables(input) == -1)
                return 1;
			break;

		case DRI_MK:

			get_size(input);	/* skip size */
			restart_interval = get_size(input);
			break;

		case SOS_MK:	/* lots of things to do here */

			//if (verbose)
				//fprintf(stderr, "%ld:\tINFO:\tFound the SOS marker!\n", FTELL(input));
			get_size(input);	/* don't care */
			aux = FGETC();
			if (aux != (unsigned int)n_comp) {
                return 1;
			}

			for (i = 0; i < n_comp; i++) {
				aux = FGETC();
				if (aux != comp[i].CID) {
                return 1;
				}
				aux = FGETC();
				comp[i].DC_HT = first_quad(aux);
				comp[i].AC_HT = second_quad(aux);
			}
			get_size(input);
			FGETC();	/* skip things */

			MCU_column = 0;
			MCU_row = 0;
			clear_bits();
			reset_prediction();

			/* main MCU processing loop here */
			if (restart_interval) {
				n_restarts = ceil_div(mx_size * my_size, restart_interval) - 1;
				leftover = mx_size * my_size - n_restarts * restart_interval;
				/* final interval may be incomplete */

				for (i = 0; i < n_restarts; i++) {
					for (j = 0; j < restart_interval; j++)
						process_MCU(input);
					/* proc till all EOB met */

					aux = get_next_MK(input);
					if (!RST_MK(aux)) {
                        return 1;
					}	
					reset_prediction();
					clear_bits();
				}	/* intra-interval loop */
			} else
				leftover = mx_size * my_size;

			/* process till end of row without restarts */
			for (i = 0; i < leftover; i++)
				process_MCU(input);

			in_frame = 0;
			break;

		case EOI_MK:
			if (in_frame)
				//aborted_stream(input);
                return 1;

			free_structures();
			return 0;
			break;

		case COM_MK:
			skip_segment(input);
			break;

		case (const int)EOF:
			return 1;

		default:
			if ((mark & MK_MSK) == APP_MK) {
				skip_segment(input);
				break;
			}
			if (RST_MK(mark)) {
				reset_prediction();
				break;
			}
			return 1;
			break;
		}		/* end switch */
	} while (1);

	return 0;
}

/*
void write_bmp(volatile unsigned int *output)
{
    int x, y;

    for(y = 0; y < y_size; y++)
    {
        for(x = 0; x < 3*x_size; x = x + 3)
        {
            unsigned int temp = 0;
            temp = FrameBuffer[y*3*x_size + x];
            temp = temp << 8;
            temp = temp | FrameBuffer[y*3*x_size + x + 1];
            temp = temp << 8;
            temp = temp | FrameBuffer[y*3*x_size + x + 2];
            output[y*1024 + x/3] = temp;
        }
    }
}

*/
