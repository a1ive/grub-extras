#include "timers.h"
#include "interrupts.h"
#include "bits.h"
#include "mmu/mbc.h"

//Possible timer increment timer_frequencies in hz
#define TIMER_FREQUENCIES_LEN sizeof (timer_frequencies) / sizeof (long)
static const long timer_frequencies[] = {1024, 16, 64, 256}; 

static long timer_frequency = -1;
static long timer_counter = 0;
static uint64_t clocks = 0;

/* Change the timer frequency to another of the possible
 * frequencies, resets the timer_counter 
 * If frequency number selected isn't valid then nothing happens*/
void set_timer_frequency(unsigned int n) {
    if (n < TIMER_FREQUENCIES_LEN) {
        timer_frequency = timer_frequencies[n];
    }
}

static long get_timer_frequency(void) {
    return cgb_speed ? timer_frequency / 2 : timer_frequency;
}

/*  Increments the TIMA register
 *  if causes overflow, timer interrupt is raised*/
void increment_tima(void) {

    uint8_t tima = io_mem[TIMA_REG] + 1;

    if (tima == 0) { //Overflow
        tima = io_mem[TMA_REG];
        raise_interrupt(TIMER_INT);
    }
    io_mem[TIMA_REG] = tima;
    
}


/*  Increment DIV register 
 *  should be incremented at a frequency of 16382
 *  (once every 256 clock cycles)*/
void increment_div(void) {
    io_mem[DIV_REG] += 1;
}


static void update_divider_reg(long cycles) {
    
    static long divider_counter = 0;

	divider_counter += cycles;
	long div_cycles = cgb_speed ? 128 : 256;
	while (divider_counter >= div_cycles) {
		increment_div();
		divider_counter -= div_cycles;
	}
}


/* Update internal timers given the cycles executed since
* the last time this function was called. */
void update_timers(long cycles) {
    clocks += cycles;
    // Inc MBC3 RTC seconds
    if (clocks >= 4 * 1024 * 1024) {
        inc_sec_mbc3();         
        clocks -= 4 * 1024 * 1024;
    }

	uint8_t timer_control = io_mem[TAC_REG];

	update_divider_reg(cycles);
	//Clock enabled
	if ((timer_control & BIT_2) != 0) {
		if (timer_frequency == -1) { // If timer not set
			set_timer_frequency(timer_control & 3);
		}
		timer_counter += cycles;
		/* Once timer incremented, check for changes in timer frequency,
		* and reset timer */
		while (timer_counter >= get_timer_frequency()) {
			timer_counter -= get_timer_frequency();
			increment_tima();
			set_timer_frequency(timer_control & 3);
		}
	}
}

