#include "c8interpreter.h"

__attribute__ ((always_inline)) static inline grub_efi_uint16_t l2b(grub_efi_uint16_t l)
{
	return (l >> 8) | (l << 8);
}

__attribute__ ((always_inline)) static inline grub_efi_uint16_t b2l(grub_efi_uint16_t b)
{
	return (b >> 8) | (b << 8);
}

#define OPCODE_HANDLER(OPCODE) \
	__attribute__((always_inline)) \
static inline void interpret_##OPCODE(struct interpreter_state *state)

OPCODE_HANDLER(00E0)
{
	display_clear();
	state->PC += 2;
}

OPCODE_HANDLER(00EE)
{
	state->PC = state->stack[--state->SP];
}

OPCODE_HANDLER(0NNN)
{
	/* Shouldn't appear in normal program. */
	state->execution_state = EX_STOP;
}

OPCODE_HANDLER(1NNN)
{
	grub_efi_uint16_t NNN = b2l(*((grub_efi_uint16_t*) (state->memory + state->PC))) & 0x0FFF;
	state->PC = NNN;
}

OPCODE_HANDLER(2NNN)
{
	grub_efi_uint16_t NNN = b2l(*((grub_efi_uint16_t*) (state->memory + state->PC))) & 0x0FFF;
	state->stack[state->SP++] = state->PC + 2;
	state->PC = NNN;
}

OPCODE_HANDLER(3XNN)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	grub_efi_uint8_t NN = state->memory[state->PC + 1];
	if (state->V[X] == NN)
		state->PC += 4;
	else
		state->PC += 2;
}

OPCODE_HANDLER(4XNN)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	grub_efi_uint8_t NN = state->memory[state->PC + 1];
	if (state->V[X] != NN)
		state->PC += 4;
	else
		state->PC += 2;
}

OPCODE_HANDLER(5XY0)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	grub_efi_uint8_t Y = state->memory[state->PC + 1] >> 4;
	if (state->V[X] == state->V[Y])
		state->PC += 4;
	else
		state->PC += 2;
}

OPCODE_HANDLER(9XY0)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	grub_efi_uint8_t Y = state->memory[state->PC + 1] >> 4;
	if (state->V[X] != state->V[Y])
		state->PC += 4;
	else
		state->PC += 2;
}

OPCODE_HANDLER(6XNN)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	grub_efi_uint8_t NN = state->memory[state->PC + 1];
	state->V[X] = NN;
	state->PC += 2;
}

OPCODE_HANDLER(7XNN)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	grub_efi_uint8_t NN = state->memory[state->PC + 1];
	state->V[X] += NN;
	state->PC += 2;
}

OPCODE_HANDLER(8XY0)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	grub_efi_uint8_t Y = state->memory[state->PC + 1] >> 4;
	state->V[X] = state->V[Y];
	state->PC += 2;
}

OPCODE_HANDLER(8XY1)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	grub_efi_uint8_t Y = state->memory[state->PC + 1] >> 4;
	state->V[X] |= state->V[Y];
	state->PC += 2;
}

OPCODE_HANDLER(8XY2)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	grub_efi_uint8_t Y = state->memory[state->PC + 1] >> 4;
	state->V[X] &= state->V[Y];
	state->PC += 2;
}

OPCODE_HANDLER(8XY3)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	grub_efi_uint8_t Y = state->memory[state->PC + 1] >> 4;
	state->V[X] ^= state->V[Y];
	state->PC += 2;
}

OPCODE_HANDLER(8XY4)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	grub_efi_uint8_t Y = state->memory[state->PC + 1] >> 4;
	if ((grub_efi_uint16_t) state->V[X] + state->V[Y] > 255)
		state->V[0xF] = 1;
	else
		state->V[0xF] = 0;
	state->V[X] += state->V[Y];
	state->PC += 2;
}

OPCODE_HANDLER(8XY5)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	grub_efi_uint8_t Y = state->memory[state->PC + 1] >> 4;
	if (state->V[X] < state->V[Y])
		state->V[0xF] = 0;
	else
		state->V[0xF] = 1;
	state->V[X] -= state->V[Y];
	state->PC += 2;
}

OPCODE_HANDLER(8XY6)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	/* grub_efi_uint8_t Y = state->memory[state->PC + 1] >> 4; */
	/* Actually this is not a defined CHIP-8 behaviour, this is a dirty hack to make
	   Space Invaders by David Winter working. */
	state->V[0xF] = state->V[X] & 0x01;
	state->V[X] = state->V[X] >> 1;
	state->PC += 2;
}

OPCODE_HANDLER(8XY7)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	grub_efi_uint8_t Y = state->memory[state->PC + 1] >> 4;
	if (state->V[Y] < state->V[X])
		state->V[0xF] = 0;
	else
		state->V[0xF] = 1;
	state->V[X] = state->V[Y] - state->V[X];
	state->PC += 2;
}

OPCODE_HANDLER(8XYE)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	grub_efi_uint8_t Y = state->memory[state->PC + 1] >> 4;
	state->V[0xF] = state->V[Y] >> 7;
	state->V[X] = state->V[Y] << 1;
	state->PC += 2;
}

OPCODE_HANDLER(ANNN)
{
	grub_efi_uint16_t NNN = b2l(*((grub_efi_uint16_t*) (state->memory + state->PC))) & 0x0FFF;
	state->I = NNN;
	state->PC += 2;
}

OPCODE_HANDLER(BNNN)
{
	grub_efi_uint16_t NNN = b2l(*((grub_efi_uint16_t*) (state->memory + state->PC))) & 0x0FFF;
	state->PC = NNN + state->V[0];
}

OPCODE_HANDLER(CXNN)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	grub_efi_uint8_t NN = state->memory[state->PC + 1];
	/* So clever, much random, wow! */
	state->V[X] = (grub_efi_uint8_t) (state->cycle / 100) & NN;
	state->PC += 2;
}

OPCODE_HANDLER(DXYN)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	grub_efi_uint8_t Y = state->memory[state->PC + 1] >> 4;
	grub_efi_uint8_t N = state->memory[state->PC + 1] & 0x0F;

	state->V[0xF] = 0;

	for (int y_c = 0; y_c < N; y_c++)
	{
		for (int x_c = 0; x_c < 8; x_c++)
		{
			if (((state->memory[state->I + y_c] >> x_c) & 0x01) == 1)
			{
				if (display_px_xor(state->V[X] + (7 - x_c), state->V[Y] + y_c, PX_FILLED) == PX_DIRTY)
					state->V[0xF] = 1;
				else
					state->V[0xF] = 0;
			}
		}
	}

	state->PC += 2;
}

OPCODE_HANDLER(EX9E)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	if (state->last_key == state->V[X])
	{
		state->PC += 4;
		state->last_key = KB_PASS;
	}
	else
		state->PC += 2;
}

OPCODE_HANDLER(EXA1)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	if (state->last_key != state->V[X])
		state->PC += 4;
	else
	{
		state->PC += 2;
		state->last_key = KB_PASS;
	}
}

OPCODE_HANDLER(FX07)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	state->V[X] = state->delay_timer;
	state->PC += 2;
}

OPCODE_HANDLER(FX0A)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	state->X = X;
	state->execution_state = EX_AWAITS_KEY;
	state->PC += 2;
}

OPCODE_HANDLER(FX15)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	state->delay_timer = state->V[X];
	state->PC += 2;
}

OPCODE_HANDLER(FX18)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	state->sound_timer = state->V[X];
	state->PC += 2;
}

OPCODE_HANDLER(FX1E)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	state->I += state->V[X];
	state->PC += 2;
}

OPCODE_HANDLER(FX29)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	state->I = 0x0 + state->V[X] * 5;
	state->PC += 2;
}

OPCODE_HANDLER(FX33)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;
	state->memory[state->I] = state->V[X] / 100;
	state->memory[state->I + 1] = (state->V[X] % 100) / 10;
	state->memory[state->I + 2] = state->V[X] % 10;
	state->PC += 2;
}

OPCODE_HANDLER(FX55)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;

	for (int c = 0; c <= X; c++)
	{
		state->memory[state->I + c] = state->V[c];
	}

	state->I = state->I + X + 1;
	state->PC += 2;
}

OPCODE_HANDLER(FX65)
{
	grub_efi_uint8_t X = state->memory[state->PC] & 0x0F;

	for (int c = 0; c <= X; c++)
	{
		state->V[c] = state->memory[state->I + c];
	}

	state->I = state->I + X + 1;
	state->PC += 2;
}

#undef OPCODE_HANDLER

interpreter_status_t interpreter_init(char *rom_file, char *image_file, struct interpreter_state *state)
{
	state->PC = 512;
	state->I = 512;
	state->delay_timer = 0;
	state->sound_timer = 0;
	state->execution_state = EX_NORMAL;
	state->X = 0;
	state->SP = 0;
	state->cycle = 0;
	state->timer_rate = TIMER_RATE;

	grub_ssize_t rom_sz = 512;
	grub_ssize_t image_sz = 3584;

	grub_efi_status_t status;
	status = read_file(rom_file, &rom_sz, (void *) state->memory);
	if (status != GRUB_EFI_SUCCESS)
		return INT_INIT_FAILED;
	status = read_file(image_file, &image_sz, (void *) (state->memory + 512));
	if (status != GRUB_EFI_SUCCESS)
		return INT_INIT_FAILED;

	return INT_OK;
}

interpreter_status_t interpreter_loop(struct interpreter_state *state)
{
	key_event_t k_ev;
	grub_efi_boot_services_t *b;
	b = grub_efi_system_table->boot_services;

	while (1)
	{
		efi_call_1 (b->stall, CLOCK_RATE);

		state->cycle++;

		k_ev = input_next();
		if (k_ev == KB_ESC)
			return INT_OK;
		else if (k_ev == KB_ERROR)
			return INT_EXEC_FAILED;
		else if (k_ev != KB_PASS)
			state->last_key = k_ev;

		if (state->execution_state == EX_STOP)
		{
			LOG("Stopped at %x : opcode %x\n", state->PC, *((grub_efi_uint16_t*) (state->memory + state->PC)));
			return INT_EXEC_FAILED;
		}

		if (state->sound_timer != 0)
			beep();

		if (state->cycle % state->timer_rate == 0)
		{
			if (state->sound_timer > 0)
				state->sound_timer--;
			if (state->delay_timer > 0)
				state->delay_timer--;
		}

		if (state->execution_state == EX_AWAITS_KEY)
		{
			if (state->last_key == KB_PASS)
				continue;
			else
			{
				state->V[state->X] = state->last_key;
				state->execution_state = EX_NORMAL;
				continue;
			}
		}

		#define INTERPRETER_CASE(opcode, descr, mask) \
			if ((*((grub_efi_uint16_t*) (state->memory + state->PC)) & l2b(mask)) == *((grub_efi_uint16_t*) (state->memory + state->PC))) \
			{ \
				interpret_##opcode(state); \
				continue; \
			}

			FOR_OPCODES(INTERPRETER_CASE)
		#undef INTERPRETER_CASE

		/* Unknown opcode: */
		state->execution_state = EX_STOP;
	}

	return INT_OK;
}