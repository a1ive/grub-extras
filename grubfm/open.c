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

#include <grub/types.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/err.h>
#include <grub/dl.h>
#include <grub/file.h>
#include <grub/term.h>
#include <grub/env.h>
#include <grub/normal.h>
#include <grub/command.h>
#include <grub/script_sh.h>

#include "fm.h"

static void
grubfm_add_menu_back (const char *filename)
{
  char *dir = NULL;
  char *src = NULL;
  dir = grub_strdup (filename);
  *grub_strrchr (dir, '/') = 0;

  src = grub_xasprintf ("grubfm \"%s/\"", dir);

  grubfm_add_menu (_("Back"), "go-previous", NULL, src, 0);
  grub_free (src);
  if (dir)
    grub_free (dir);
}

static void
grubfm_open_iso_loopback (grub_file_t file, char *path)
{
  char *src = NULL;
  src = grub_xasprintf ("loopback iso_loop \"%s\"", path);
  if (!grubfm_run_cmd (src, NULL))
    return;
  grub_free (src);
  if (grubfm_file_exist ((char *)"(iso_loop)/boot/grub/loopback.cfg"))
  {
    char *dev_uuid = NULL;
    if (file->fs->fs_uuid)
    {
      int err;
      err = file->fs->fs_uuid (file->device, &dev_uuid);
      if (err)
      {
        grub_errno = 0;
        dev_uuid = NULL;
      }
    }
    grub_env_set ("dev_uuid", dev_uuid);
    grub_env_export ("dev_uuid");
    grub_env_set ("rootuuid", dev_uuid);
    grub_env_export ("rootuuid");
    char *iso_path = NULL;
    iso_path = grub_strchr (path, '/');
    grub_env_set ("iso_path", iso_path);
    grub_env_export ("iso_path");
    grubfm_add_menu (_("Boot ISO (Loopback)"), "gnu-linux", NULL,
        "root=iso_loop\nconfigfile /boot/grub/loopback.cfg", 0);
    grub_free (dev_uuid);
  }
}

static void
grubfm_open_efi (grub_file_t file __attribute__ ((unused)), char *path)
{
#ifdef GRUB_MACHINE_EFI
  char *src = NULL;
  src = grub_xasprintf ("set lang=en_US\nchainloader -b -t \"%s\"", path);
  if (!src)
    return;
  grubfm_add_menu (_("Open As EFI Application"), "uefi", NULL, src, 0);
#endif
}

void
grubfm_open_file (char *path)
{
  grubfm_add_menu_back (path);
  struct grubfm_enum_file_info info;
  grub_file_t file = 0;
  file = grub_file_open (path, GRUB_FILE_TYPE_GET_SIZE |
                           GRUB_FILE_TYPE_NO_DECOMPRESS);
  if (!file)
    return;
  info.name = file->name;
  info.size = (char *) grub_get_human_size (file->size, 
                                            GRUB_HUMAN_SIZE_SHORT);
  grubfm_get_file_type (&info);
  switch (info.type)
  {
    case ISO:
      grubfm_open_iso_loopback (file, path);
      break;
    case DISK:
      break;
    case VHD:
      break;
    case FBA:
      break;
    case IMAGE:
      break;
    case EFI:
      grubfm_open_efi (file, path);
    case LUA:
      break;
    case TAR:
      break;
    case WIM:
      break;
    case NT5:
      break;
    case CFG:
      break;
    case FONT:
      break;
    case MOD:
      break;
    case MBR:
      break;
    case NSH:
      break;
    case LST:
      break;
    case IPXE:
      break;
    case PYTHON:
      break;
    default:
      DBG ("FILE\n");
  }
  grub_file_close (file);
}
