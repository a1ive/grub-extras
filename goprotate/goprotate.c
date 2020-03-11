/* goprotate.c - change EFI boot order.  */
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

#include <grub/efi/api.h>
#include <grub/efi/efi.h>
#include <grub/dl.h>
#include <grub/err.h>
#include <grub/extcmd.h>
#include <grub/i18n.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/types.h>

GRUB_MOD_LICENSE ("GPLv3+");

static const struct grub_arg_option options_goprotate[] =
{
  {"0", '0', 0, N_("rot0"), 0, 0},
  {"1", '1', 0, N_("rot90"), 0, 0},
  {"2", '2', 0, N_("rot180"), 0, 0},
  {"3", '3', 0, N_("rot270"), 0, 0},
  {0, 0, 0, 0, 0, 0}
};

#define GOP_ROTATE_PROTOCOL_GUID \
  { 0xda62085c, 0x96e4, 0x4345, \
    { 0xa3, 0xb0, 0x98, 0x89, 0xda, 0xf3, 0x16, 0xb7 } \
  }

static grub_efi_guid_t gop_rot_guid = GOP_ROTATE_PROTOCOL_GUID;

enum rotate_type
{
  ROT0 = 0,
  ROT90 = 1,
  ROT180 = 2,
  ROT270 = 3,
  ROTMAX = 4,
};

struct grub_efi_gop_rotate_protocol
{
  grub_efi_status_t (*get_pos) (struct grub_efi_gop_rotate_protocol *this,
                                       enum rotate_type *rotation);
  grub_efi_status_t (*set_pos) (struct grub_efi_gop_rotate_protocol *this,
                                       enum rotate_type rotation);
};
typedef struct grub_efi_gop_rotate_protocol grub_efi_gop_rotate_protocol_t;

static grub_err_t
grub_cmd_goprotate (grub_extcmd_context_t ctxt,
                    int argc __attribute__ ((unused)),
                    char **args __attribute__ ((unused)))
{
  struct grub_arg_list *state = ctxt->state;
  grub_efi_status_t status;
  grub_efi_boot_services_t *b;
  grub_efi_gop_rotate_protocol_t *rot_p = NULL;
  b = grub_efi_system_table->boot_services;

  status = efi_call_3 (b->locate_protocol,
                       &gop_rot_guid, NULL, (void **)&rot_p);
  if (status != GRUB_EFI_SUCCESS || !rot_p)
    return grub_error (GRUB_ERR_BAD_OS, N_("ROTATE_PROTOCOL not found."));

  if (state[0].set)
    efi_call_2 (rot_p->set_pos, rot_p, ROT0);
  if (state[1].set)
    efi_call_2 (rot_p->set_pos, rot_p, ROT90);
  if (state[2].set)
    efi_call_2 (rot_p->set_pos, rot_p, ROT180);
  if (state[3].set)
    efi_call_2 (rot_p->set_pos, rot_p, ROT270);
  return GRUB_EFI_SUCCESS;
}

static grub_extcmd_t cmd_goprotate;

GRUB_MOD_INIT(goprotate)
{
  cmd_goprotate = grub_register_extcmd ("goprotate", grub_cmd_goprotate, 0,
                N_("OPTIONS"), N_("Rotate screen."), options_goprotate);
}

GRUB_MOD_FINI(goprotate)
{
  grub_unregister_extcmd (cmd_goprotate);
}
