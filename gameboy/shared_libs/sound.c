#include "../core/sound.h"

// UEFI doesn't have any sound support, just stub everything out


void init_apu() {
}
    
void sound_add_cycles(unsigned c __attribute__ ((unused))) {
}

void write_apu(uint16_t addr __attribute__ ((unused)), uint8_t val __attribute__ ((unused))) {
}

uint8_t read_apu(uint16_t addr __attribute__ ((unused))) {
    return 0xFF;
}

void end_frame() {
}                           
