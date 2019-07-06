#include <grub/types.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/err.h>
#include <grub/dl.h>
#include <grub/term.h>

#include "c8input.h"

input_status_t input_init()
{
	return IN_OK;
}

key_event_t input_next()
{
	int key = 0;

	key = grub_getkey_noblock ();

	switch (key)
	{
		case '0':
		return KB_0;
		
		case '1':
		return KB_1;

		case '2':
		return KB_2;

		case '3':
		return KB_3;

		case '4':
		return KB_4;

		case '5':
		return KB_5;

		case '6':
		return KB_6;

		case '7':
		return KB_7;

		case '8':
		return KB_8;

		case '9':
		return KB_9;

		case 'a':
		return KB_A;

		case 'b':
		return KB_B;

		case 'c':
		return KB_C;

		case 'd':
		return KB_D;

		case 'e':
		return KB_E;

		case 'f':
		return KB_F;

		case 'z':
		return KB_ESC;
		
		case GRUB_TERM_ESC:
		return KB_ESC;
	}

	return KB_PASS;
}