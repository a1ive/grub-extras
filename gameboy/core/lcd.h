#ifndef LCD_H
#define LCD_H

#include "lcd.h"
#include "timers.h"
#include "mmu/memory.h"
#include "memory_layout.h"
#include "interrupts.h"
#include "graphics.h"
#include "bits.h"
#include <stdint.h>
#include "cpu.h"

/* Given the elapsed cpu cycles since the last
* call to this function, updates the internal LCD
* modes, registers and if a Vertical Blank occurs redisplays
* the screen, returns amount of new cycles */
long update_graphics(long cycles);

void reset_window_line(void);

void enable_screen(void);

void disable_screen(void);

uint8_t get_interrupt_signal(void);

int get_lcd_mode(void);

void set_interrupt_signal(uint8_t s);

void check_lcd_coincidence(void);

int screen_enabled(void);

int lcd_vblank_mode(void);

int lcd_hblank_mode(void);

#endif //LCD_H
