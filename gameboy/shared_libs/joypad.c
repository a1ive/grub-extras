#include "../non_core/joypad.h"

#include "../core/mmu/mbc.h"
#include "../non_core/logger.h"

#include <grub/types.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/efi/api.h>
#include <grub/efi/efi.h>

#include <stdlib.h>

//
// Required unicode control chars
//
#define CHAR_NULL             0x0000
#define CHAR_BACKSPACE        0x0008
#define CHAR_TAB              0x0009
#define CHAR_LINEFEED         0x000A
#define CHAR_CARRIAGE_RETURN  0x000D

//
// EFI Scan codes
//
#define SCAN_NULL       0x0000
#define SCAN_UP         0x0001
#define SCAN_DOWN       0x0002
#define SCAN_RIGHT      0x0003
#define SCAN_LEFT       0x0004
#define SCAN_HOME       0x0005
#define SCAN_END        0x0006
#define SCAN_INSERT     0x0007
#define SCAN_DELETE     0x0008
#define SCAN_PAGE_UP    0x0009
#define SCAN_PAGE_DOWN  0x000A
#define SCAN_F1         0x000B
#define SCAN_F2         0x000C
#define SCAN_F3         0x000D
#define SCAN_F4         0x000E
#define SCAN_F5         0x000F
#define SCAN_F6         0x0010
#define SCAN_F7         0x0011
#define SCAN_F8         0x0012
#define SCAN_F9         0x0013
#define SCAN_F10        0x0014
#define SCAN_F11        0x0015
#define SCAN_F12        0x0016
#define SCAN_ESC        0x0017

int scale = 1;
int full_size = 0;

typedef struct {
	int scan_code;
    int key_code; // Key code on the keyboard which
                       // maps to the given gameboy button
    int state; // 1 pressed, 0 unpressed
} button_state;

enum {UP = 0, DOWN, LEFT, RIGHT, A, B, START, SELECT};

button_state buttons[8];

#define TOTAL_BUTTONS (sizeof(buttons)/sizeof(buttons[0]))

grub_efi_uint16_t button_config_scan_codes[] =
	{SCAN_UP, SCAN_DOWN, SCAN_LEFT, SCAN_RIGHT, 0, 0, 0 ,0};
grub_efi_char16_t button_config_key_codes[] =
	{0, 0, 0, 0, 0x0061 /* a */, 0x0073 /* s */, CHAR_CARRIAGE_RETURN, 0x0020 /* space */};

/*  Intialize the joypad, should be called before any other
 *  joypad functions */
void init_joypad() { 
     
    buttons[UP].state = 0;
    buttons[UP].key_code = button_config_key_codes[UP];
    buttons[UP].scan_code = button_config_scan_codes[UP];

    buttons[DOWN].state = 0;
    buttons[DOWN].key_code = button_config_key_codes[DOWN];
    buttons[DOWN].scan_code = button_config_scan_codes[DOWN];

    buttons[LEFT].state = 0;
    buttons[LEFT].key_code = button_config_key_codes[LEFT];
    buttons[LEFT].scan_code = button_config_scan_codes[LEFT];

    buttons[RIGHT].state = 0;
    buttons[RIGHT].key_code = button_config_key_codes[RIGHT];
    buttons[RIGHT].scan_code = button_config_scan_codes[RIGHT];

    buttons[A].state = 0;
    buttons[A].key_code = button_config_key_codes[A];
    buttons[A].scan_code = button_config_scan_codes[A];

    buttons[B].state = 0;
    buttons[B].key_code = button_config_key_codes[B];
    buttons[B].scan_code = button_config_scan_codes[B];

    buttons[START].state = 0;
    buttons[START].key_code = button_config_key_codes[START];
    buttons[START].scan_code = button_config_scan_codes[START];
   
    buttons[SELECT].state = 0;
    buttons[SELECT].key_code = button_config_key_codes[SELECT];
    buttons[SELECT].scan_code = button_config_scan_codes[SELECT];
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
	grub_efi_status_t status = GRUB_EFI_SUCCESS;
	grub_efi_input_key_t input_key;

	while (1)
	{
		status = efi_call_2 (grub_efi_system_table->con_in->read_key_stroke,
								grub_efi_system_table->con_in, &input_key);
		if (status == GRUB_EFI_NOT_READY)
		{
			// No more key strokes
			break;
		}
		else if (status != GRUB_EFI_SUCCESS)
		{
			break;
		}

        // Quit
        if (input_key.scan_code == SCAN_ESC) {
            return 1; 
        }

        if (input_key.unicode_char == 0x0032 /* 2 */) {
            scale++;
        }

        if (input_key.unicode_char == 0x0031 /* 1 */) {
            scale--;
            if (scale < 1) {
                scale = 1;
            } 
        }
	
		for (size_t i = 0; i < TOTAL_BUTTONS; i++) {
        	if (buttons[i].scan_code == input_key.scan_code) {
				if (input_key.scan_code != 0) {
                	buttons[i].state = 1;
					break;
				}
				if (buttons[i].key_code == input_key.unicode_char) {
					buttons[i].state = 1;
					break;
				}
			}
		}
	}

	return 0;
}
