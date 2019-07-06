/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2019  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 *  MIT License
 *  Copyright (c) 2017 Alexander Babayants

 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:

 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */


#include <grub/types.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/err.h>
#include <grub/dl.h>
#include <grub/extcmd.h>
#include <grub/file.h>
#include <grub/i18n.h>
#include <grub/term.h>
#include <grub/efi/api.h>
#include <grub/efi/efi.h>

#include "c8interpreter.h"

GRUB_MOD_LICENSE ("GPLv3+");
GRUB_MOD_DUAL_LICENSE("MIT");

static grub_err_t
grub_cmd_chip8 (grub_extcmd_context_t ctxt __attribute__ ((unused)),
		int argc, char **args)
{

	interpreter_status_t i_status;
	struct interpreter_state i_state = {0};

	if (argc < 1) 
	{
		grub_printf("CHIP-8 emulator.\nUsage: chip8 FILE [ROM]\n");
		return GRUB_ERR_NONE;
	}
	else if (argc == 1)
		i_status = interpreter_init(NULL, args[0], &i_state);
	else
		i_status = interpreter_init(args[1], args[0], &i_state);

	if (i_status != INT_OK)
	{
		grub_printf("Failed to initialize interpreter.\n");
		return GRUB_ERR_NONE;
	}

	if (input_init() != IN_OK)
	{
		grub_printf("Failed to initialize keyboard input.\n");
		return GRUB_ERR_NONE;
	}

	if (display_init() != DS_OK)
	{
		grub_printf("Failed to initialize display.\n");
		return GRUB_ERR_NONE;
	}

	i_status = interpreter_loop(&i_state);
	if (i_status != INT_OK)
		grub_printf("Failed in interpreter loop.\n");
	else
		display_clear();

	return GRUB_ERR_NONE;
}

static grub_extcmd_t cmd;

GRUB_MOD_INIT(chip8)
{
  cmd = grub_register_extcmd ("chip8", grub_cmd_chip8, 0, N_("FILE [ROM]"),
			      N_("CHIP-8 emulator."), 0);
}

GRUB_MOD_FINI(chip8)
{
  grub_unregister_extcmd (cmd);
}
