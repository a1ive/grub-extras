#ifndef EMU_H
#define EMU_H

#include "serial_io.h"

/* Intialize emulator with given ROM file, and
 * specify whether or not debug mode is active
 * (0 for OFF, any other value is on) 
 *
 * returns 1 if successfully initialized, 0
 * otherwise */
int init_emu(const char *file_path, int debugger, int dmg_mode, ClientOrServer cs);

// Free up all resources, preparing to exit
void finalize_emu(void);

// Execute until a single frame has been rendered
void run_one_frame(void);

//Main Fetch-Decode-Execute loop
void run(void);


void add_current_cycles(unsigned cycles);

#endif
