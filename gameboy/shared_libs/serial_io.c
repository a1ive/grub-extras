#include <stdint.h>
#include <string.h>

#include "../non_core/serial_io_transfer.h"
#include "../non_core/logger.h"

/* Setup TCP Client, and attempt to connect
 * to the server */
int setup_client(unsigned port __attribute__ ((unused))) {
    return 0;
}

/*  Setup TCP Server, and wait for a single
 *  client to connect */
int setup_server(unsigned port __attribute__ ((unused))) { 
    return 0;
}

/*  Send and Recieved byte 
int transfer(uint8_t data __attribute__ ((unused)), uint8_t *recv __attribute__ ((unused)), int ext __attribute__ ((unused))) {
    return 0;
}


void quit_io(void) {
}*/


// Transfer when current GB is using external clock
// returns 1 if there is data to be recieved, 0 otherwise
int transfer_ext(uint8_t data __attribute__ ((unused)), uint8_t *recv __attribute__ ((unused))) {
    return 0;
}

// Transfer when current GB is using internal clock
// returns 0xFF if no external GB found
uint8_t transfer_int(uint8_t data __attribute__ ((unused))) {
    return 0xFF;
}
