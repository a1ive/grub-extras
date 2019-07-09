#ifndef DEBUGGER_H
#define DEBUGGER_H

typedef enum {NONE = 0x0, STEPS_SET = 0x1, BREAKPOINT_SET = 0x2} Command_Flag;

#define BREAKPOINT_OFF 0x0
#define STEPS_OFF 0x0

/* Get debugger command(s) and perform a number
 * of debugger actions. Returns an integer containing set/unset
 * flags specifying the options selected. */
int get_command(void);


/* Get number of steps, 0 or below
 * if stepping not occuring */
long get_steps(void);

// No longer stepping
void turn_steps_off(void);


/* Get 16bit address of breakpoint
 * result is below 0 if no breakpoint set */
long get_breakpoint(void); 

//Disable breakpoint
void turn_breakpoint_off(void);



#endif //DEBUGGER_H
