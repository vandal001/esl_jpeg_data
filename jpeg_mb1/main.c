#include <comik.h>
#include <global_memmap.h>
#include <5kk03-utils.h>

#include "jpeg.h"
#include "surfer.jpg.h"

int main (void)
{
    volatile unsigned int *ddr_mem = (unsigned int*) shared_pt_REMOTEADDR;

	// Sync with the monitor.
	mk_mon_sync();
	// Enable stack checking.
	start_stack_check();
	// Paint it our 'group' color so we can identify it.
	for ( int i = 0 ; i < (768*1024);i++ ){
		ddr_mem[i] = 0x00771468;
	}

	// Start decoding the JPEG.
    mk_mon_debug_info(1100);
	if( JpegToBmp((unsigned int*)(&(input_buffer[0])), ddr_mem) )
        mk_mon_debug_info(1101);
    else
        mk_mon_debug_info(1102);

	// Signal the monitor we are done.
	mk_mon_debug_tile_finished();
	return 0;
}
