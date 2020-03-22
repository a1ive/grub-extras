/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2020  Free Software Foundation, Inc.
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

#include <grub/dl.h>
#include <grub/misc.h>
#include <grub/acpi.h>
#include <grub/command.h>
#include <grub/i18n.h>
#include <grub/mm.h>

GRUB_MOD_LICENSE ("GPLv3+");

static grub_err_t
grub_cmd_s0enabler (grub_command_t cmd __attribute__ ((unused)),
                    int argc __attribute__ ((unused)),
                    char **args __attribute__ ((unused)))

{
  struct grub_acpi_rsdp_v20 *rsdp = NULL;
  struct grub_acpi_table_header *xsdt, *entry;
  int entry_cnt, i;
  grub_uint64_t *entry_ptr;

  rsdp = grub_machine_acpi_get_rsdpv2 ();
  if (! rsdp)
    return grub_error (GRUB_ERR_BAD_OS, "RSDP V2 not found.");
  if (rsdp->rsdpv1.revision >= 0x02)
    xsdt = (struct grub_acpi_table_header *)(grub_addr_t)(rsdp->xsdt_addr);
  else
    return grub_error (GRUB_ERR_BAD_OS, "XSDT not found.");
  if (grub_memcmp(xsdt->signature, "XSDT", 4) != 0)
    return grub_error (GRUB_ERR_BAD_OS, "Invalid XSDT.");
  entry_cnt = (xsdt->length
               - sizeof (struct grub_acpi_table_header)) / sizeof(grub_uint64_t);
  entry_ptr = (grub_uint64_t *)(xsdt + 1);
  for (i = 0; i < entry_cnt; i++, entry_ptr++)
  {
    entry = (struct grub_acpi_table_header *)(grub_addr_t)(*entry_ptr);
    if (grub_memcmp(entry->signature, "FACP", 4) == 0)
    {
      struct grub_acpi_fadt *fadt = (struct grub_acpi_fadt *) entry;
      grub_printf ("FADT: %p\n", fadt);
      grub_printf ("Patching flags 0x%x", fadt->flags);
      fadt->flags |= 1 << 21;
      grub_printf ("->0x%x\n", fadt->flags);
      grub_printf ("Patching checksum 0x%x", fadt->hdr.checksum);
      fadt->hdr.checksum = 0;
      fadt->hdr.checksum = 1 + ~grub_byte_checksum (fadt, fadt->hdr.length);
      grub_printf ("->0x%x\n", fadt->hdr.checksum);
      return GRUB_ERR_NONE;
    }
  }
  return grub_error (GRUB_ERR_BAD_OS, "FADT not found.");
}

static grub_command_t cmd;

GRUB_MOD_INIT(s0enabler)
{
  cmd = grub_register_command ("enable_s0", grub_cmd_s0enabler, 0,
                               N_("Enable S0 Low Power Idle mode."));
}

GRUB_MOD_FINI(s0enabler)
{
  grub_unregister_command (cmd);
}
