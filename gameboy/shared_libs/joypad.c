#include "../non_core/joypad.h"

#include "../core/mmu/mbc.h"
#include "../non_core/logger.h"

#include <grub/types.h>
#include <grub/term.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/efi/api.h>
#include <grub/efi/efi.h>

#include <stdlib.h>

//
// Required unicode control chars
//
#define CHAR_CARRIAGE_RETURN  0x0D

int scale = 1;
int full_size = 0;

typedef struct {
	int key_code;
	int state; // 1 pressed, 0 unpressed
} button_state;

enum {UP = 0, DOWN, LEFT, RIGHT, A, B, START, SELECT};

button_state buttons[8];

#define TOTAL_BUTTONS (sizeof(buttons)/sizeof(buttons[0]))

int button_config_key_codes[] =
	{ GRUB_TERM_KEY_UP,
	  GRUB_TERM_KEY_DOWN, 
	  GRUB_TERM_KEY_LEFT,
	  GRUB_TERM_KEY_RIGHT,
	  'a',
	  's',
	  CHAR_CARRIAGE_RETURN,
	  ' '
	};

/*  Intialize the joypad, should be called before any other
 *  joypad functions */
void init_joypad() { 
     
    buttons[UP].state = 0;
    buttons[UP].key_code = button_config_key_codes[UP];

    buttons[DOWN].state = 0;
    buttons[DOWN].key_code = button_config_key_codes[DOWN];

    buttons[LEFT].state = 0;
    buttons[LEFT].key_code = button_config_key_codes[LEFT];

    buttons[RIGHT].state = 0;
    buttons[RIGHT].key_code = button_config_key_codes[RIGHT];

    buttons[A].state = 0;
    buttons[A].key_code = button_config_key_codes[A];

    buttons[B].state = 0;
    buttons[B].key_code = button_config_key_codes[B];

    buttons[START].state = 0;
    buttons[START].key_code = button_config_key_codes[START];
   
    buttons[SELECT].state = 0;
    buttons[SELECT].key_code = button_config_key_codes[SELECT];
}

/* Check each individual GameBoy key. Returns 1 if
 * the specified key is being held down, 0 otherwise */
int down_pressed()   { return buttons[DOWN].state;  }  
int up_pressed()     { return buttons[UP].state; }
int left_pressed()   { return buttons[LEFT].state;}
int right_pressed()  { return buttons[RIGHT].state;} 
int a_pressed()      { return buttons[A].state; }
int b_pressed()      { return buttons[B].state;}
int start_pressed()  { return buttons[START].state; }
int select_pressed() { return buttons[SELECT].state; } 

/* Returns 1 if any of the 8 GameBoy keys are being held down,
 * 0 otherwise */
int key_pressed() {

    return down_pressed() || up_pressed() || left_pressed() || right_pressed()
    || a_pressed() || b_pressed() || start_pressed() || select_pressed();
}

void unset_keys(void);
void unset_keys(void) {
    for (size_t i = 0; i < TOTAL_BUTTONS; i++) {
        buttons[i].state = 0;
    }
}

/* Update current state of GameBoy keys as well as control
 * other external actions for the emulator */
int update_keys() {

	for (size_t i = 0; i < TOTAL_BUTTONS; i++) {
		buttons[i].state = 0;
	}

	// Get all pending input events
	int input_key = 0;

	while (1)
	{
		input_key = grub_getkey_noblock ();
		if (input_key == GRUB_TERM_NO_KEY)
		{
			break;
		}

        // Quit
        if (input_key == GRUB_TERM_ESC)
		{
            return 1; 
        }

        if (input_key == '2')
		{
            scale++;
        }

        if (input_key == '1')
		{
            scale--;
            if (scale < 1)
			{
                scale = 1;
            } 
        }
	
		for (size_t i = 0; i < TOTAL_BUTTONS; i++) 
		{
        	if (buttons[i].key_code == input_key)
			{
				buttons[i].state = 1;
				break;
			}
		}
	}
	return 0;
}
