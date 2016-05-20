#include <comik.h>
#include <global_memmap.h>
#include <5kk03-utils.h>
#include <hw_tifu.h>
//#include "jpeg.h"
//#include "surfer.jpg.h"

int main (void)
{
  //  volatile unsigned int *ddr_mem = (unsigned int*) shared_pt_REMOTEADDR;

	// Sync with the monitor.
	mk_mon_sync();
	// Enable stack checking.
	start_stack_check();
	// Paint it our 'group' color so we can identify it.
	/*
	TIME t1,t2, t3;
	t1 = hw_tifu_systimer_get();
    mk_mon_debug_info(1100);
	for ( int i = 0 ; i < (768*1024);i++ ){
		ddr_mem[i] = 0x00771468;
	}
	t2 = hw_tifu_systimer_get();
	
	// Start decoding the JPEG.
	if( JpegToBmp((unsigned int*)(&(input_buffer[0])), ddr_mem) )
        mk_mon_debug_info(1101);
    else
        mk_mon_debug_info(1102);
    t3 = hw_tifu_systimer_get();
    TIME diff = t2 - t1;
    TIME diff2 = t3 - t2;
    TIME diff3 = t3 - t1;
	//mk_mon_debug_info(LO_64(diff)); // color_the 1024x768 block
	//mk_mon_debug_info(LO_64(diff2)); // Jpeg decoding
	//mk_mon_debug_info(LO_64(diff3)); // Full run (decoding + coloring) 
	// Signal the monitor we are done.
	*/
	mk_mon_debug_tile_finished();
	return 0;
}
