/* Ross Meikleham 2014*/                  
#ifndef CPU_H
#define CPU_H

#include <stdint.h>

extern int halted;
extern int stopped;

/*  Call interrupt handler code */
void restart(uint8_t addr);


void update_all_cycles(long cycles);

/*  Check if master interrupts are enabled */
int master_interrupts_enabled(void);
void master_interrupts_disable(void);
void master_interrupts_enable(void);


void reset_cpu(void);


/*  Executes current instruction and returns
 *  the number of machine cycles it took */
int exec_opcode(int skip_bug);


void print_regs(void);


#endif
