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
#include <grub/extcmd.h>
#include <grub/i18n.h>
#include <grub/normal.h>
#include <grub/term.h>
#include <grub/menu.h>

#include "fm.h"

void
grubfm_clear_menu (void)
{
  grub_menu_t menu = grub_env_get_menu();
  menu->entry_list = NULL;
  menu->size=0;
}

void
grubfm_add_menu (const char *title, const char *icon,
                 const char *hotkey, const char *src, int hidden)
{
  const char **args = NULL;
  char **class = NULL;
  args = grub_malloc (sizeof (args[0]));
  if (!args)
    return;
  args[0] = grub_strdup (title);
  if (icon)
  {
    class = grub_malloc (2 * sizeof (class[0]));
    if (!class)
      return;
    class[0] = grub_strdup (icon);
    class[1] = NULL;
  }
  grub_normal_add_menu_entry (1, args, class, NULL, NULL, hotkey,
                              NULL, src, 0, hidden);
  if (class[0])
    grub_free (class[0]);
  if (class)
    grub_free (class);
  if (args)
    grub_free (args);
}
