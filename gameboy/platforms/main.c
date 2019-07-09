/** @file
	UEFI Plutoboy entry point
**/

#include "../non_core/logger.h"
#include "../core/mmu/memory.h"
#include "../core/emu.h"

#include "../core/serial_io.h"

#include <grub/types.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/err.h>
#include <grub/dl.h>
#include <grub/extcmd.h>
#include <grub/file.h>
#include <grub/i18n.h>
#include <grub/term.h>
#include <grub/time.h>
#include <grub/efi/api.h>
#include <grub/efi/efi.h>
#include <grub/efi/graphics_output.h>

GRUB_MOD_LICENSE ("GPLv3+");

static grub_err_t
grub_cmd_gameboy (grub_extcmd_context_t ctxt __attribute__ ((unused)),
		int argc, char **args)
{

	if (argc < 1) {
		grub_printf ("Usage: gameboy ROM_FILE\n");
		return 1;
	}

	char *file_name = args[0];
 
    int debug = 0;
    int dmg_mode = 0;

    ClientOrServer cs = NO_CONNECT;
    
    if (!init_emu(file_name, debug, dmg_mode, cs)) {
        grub_printf ("Failed to init emulator\n");
		log_message(LOG_ERROR, "failed to load file\n");
		return 1;
    }
    log_message(LOG_INFO, "exiting\n");
    run();
	return 0;
}

static grub_extcmd_t cmd;

GRUB_MOD_INIT(gameboy)
{
  cmd = grub_register_extcmd ("gameboy", grub_cmd_gameboy, 0, N_("FILE"),
				  N_("UEFI gameboy emulator."), NULL);
}

GRUB_MOD_FINI(gameboy)
{
  grub_unregister_extcmd (cmd);
}