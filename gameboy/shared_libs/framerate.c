#include "../non_core/framerate.h"

#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <grub/efi/api.h>
#include <grub/efi/efi.h>

static uint64_t last_ticks;
static int framerate;
static int count;
static uint64_t current_ticks = 0;

static uint64_t get_timestamp_micro(void) {
	return 0;
}

//Assign Framerate in FPS and start counter
void start_framerate(int f) {
    last_ticks = get_timestamp_micro();
    framerate = f;
    count = 0;
}

/* Check time elapsed after one frame, hold up
 * the program if not enough tim has elapsed */
void adjust_to_framerate() {
    current_ticks = get_timestamp_micro();
    uint64_t ticks_elapsed = current_ticks - last_ticks;

    // Running on Bare Metal, just delay for the required cycles 
	grub_efi_boot_services_t *b;
	b = grub_efi_system_table->boot_services;
	efi_call_1 (b->stall, (grub_efi_uintn_t)(1000000/framerate - ticks_elapsed));

    current_ticks = get_timestamp_micro();
    
    count = (count + 1) % 60;

    last_ticks = current_ticks;
}
