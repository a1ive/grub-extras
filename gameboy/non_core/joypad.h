#ifndef JOYPAD_H
#define JOYPAD_H

//Virtual Button Positions for Mobile Devices
#define SQUARE_SIZE (current.w / 25)
#define DPAD_SIZE (SQUARE_SIZE * 2)
#define H_BORDER (SQUARE_SIZE + DPAD_SIZE)
#define W_BORDER SQUARE_SIZE

#define DPAD_LEFT_X W_BORDER
#define DPAD_LEFT_Y(height) (height - H_BORDER - DPAD_SIZE) 
#define DPAD_LEFT_W DPAD_SIZE
#define DPAD_LEFT_H DPAD_SIZE

#define DPAD_DOWN_X (W_BORDER + DPAD_SIZE)
#define DPAD_DOWN_Y(height) (height - H_BORDER)
#define DPAD_DOWN_W DPAD_SIZE
#define DPAD_DOWN_H DPAD_SIZE

#define DPAD_RIGHT_X (DPAD_LEFT_X + DPAD_LEFT_W + DPAD_DOWN_W)
#define DPAD_RIGHT_Y(height) (DPAD_LEFT_Y(height))
#define DPAD_RIGHT_W DPAD_SIZE
#define DPAD_RIGHT_H DPAD_SIZE

#define DPAD_UP_X DPAD_DOWN_X
#define DPAD_UP_Y(height) (DPAD_LEFT_Y(height) - DPAD_LEFT_H)
#define DPAD_UP_W DPAD_SIZE
#define DPAD_UP_H DPAD_SIZE

#define SELECT_X (DPAD_RIGHT_X + DPAD_RIGHT_W + DPAD_SIZE)
#define SELECT_Y(height) (DPAD_DOWN_Y(height))
#define SELECT_W ((DPAD_SIZE * 3) / 2)
#define SELECT_H ((DPAD_SIZE * 2)/ 3)

#define START_X (SELECT_X + SELECT_W + (DPAD_SIZE / 2))
#define START_Y(height) (DPAD_DOWN_Y(height))
#define START_W ((DPAD_SIZE * 3) / 2)
#define START_H ((DPAD_SIZE * 2) / 3)

#define A_X(width) (width - W_BORDER - DPAD_SIZE)
#define A_Y(height) (DPAD_RIGHT_Y(height))
#define A_W DPAD_SIZE
#define A_H DPAD_SIZE

#define B_X(width) (A_X(width) - A_W - (DPAD_SIZE/2))
#define B_Y(height) (DPAD_RIGHT_Y(height))
#define B_W DPAD_SIZE
#define B_H DPAD_SIZE

/*  Initialize the joypad, should be called before
 *  any other joypad functions */
void init_joypad(void);

/* Update current state of GameBoy keys as well as control
 * other external actions for the emulator, returns 1 if
 * quitting, 0 otherwise */
int update_keys(void);

/* Return state of one of the 8 GameBoy keys
 * 0 for unpressed, 1 for pressed */
int down_pressed(void);  
int up_pressed(void);
int left_pressed(void);
int right_pressed(void);
int a_pressed(void);
int b_pressed(void);
int start_pressed(void);
int select_pressed(void);


/* Returns 1 if any of the 8 GameBoy keys 
 * are pressed, 0 otherwise */
int key_pressed(void);

#endif //JOYPAD_H

