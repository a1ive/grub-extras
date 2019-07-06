#pragma once

#include <grub/types.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/err.h>
#include <grub/dl.h>
#include <grub/term.h>
#include <grub/efi/api.h>
#include <grub/efi/efi.h>

#include "c8fs.h"
#include "c8input.h"
#include "c8display.h"
#include "c8sound.h"
#include "c8opcodes.h"

#include "debug.h"

#define CLOCK_RATE 2000 /* Delay for CPU clock in microseconds. */
#define TIMER_RATE 10 /* CPU clocks per timer tick. */

typedef enum
{
	INT_OK = 0,
	INT_INIT_FAILED,
	INT_EXEC_FAILED,
	INT_UNKNOWN_OP
} interpreter_status_t;

typedef enum
{
	EX_NORMAL = 0,
	EX_AWAITS_KEY,
	EX_STOP
} execution_state_t;

struct interpreter_state
{
	grub_efi_uint8_t memory[4096];
	grub_efi_uint16_t stack[16];
	grub_efi_uint8_t V[16];
	grub_efi_uint16_t PC;
	grub_efi_uint16_t I;
	grub_efi_uint8_t delay_timer;
	grub_efi_uint8_t sound_timer;
	execution_state_t execution_state;
	key_event_t last_key;
	grub_efi_uint8_t X;
	grub_efi_uint8_t SP;
	grub_efi_uintn_t cycle;
	grub_efi_uintn_t timer_rate;
};

interpreter_status_t interpreter_init(char *rom_file, char *image_file, struct interpreter_state *state);
interpreter_status_t interpreter_loop(struct interpreter_state *state);