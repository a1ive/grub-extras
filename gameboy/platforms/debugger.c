#include "../non_core/debugger.h"

#include "../core/mmu/memory.h"
#include "../core/disasm.h"
#include "../core/cpu.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Get debugger command(s) and perform a number
 * of debugger actions. Returns an integer containing set/unset
 * flags specifying the options selected. */
int get_command() {
   return 0; 
}

/* Get number of steps, 0 or below
 * if stepping not occuring */
long get_steps() {
    return 0;
}

// No longer stepping
void turn_steps_off() {
}

/* Get 16bit address of breakpoint
 * result is below 0 if no breakpoint set */
long get_breakpoint() {
    return 0;
}

//Disable breakpoint
void turn_breakpoint_off() {
}



