/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2019  Free Software Foundation, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRUB_FILE_MANAGER_HEADER
#define GRUB_FILE_MANAGER_HEADER

#include <grub/types.h>
#include <grub/misc.h>

/** Debugging output */
#define DBG(...) do {                        \
            grub_printf ( __VA_ARGS__ );            \
            grub_getkey ();            \
    } while ( 0 )

/* menu.c */
void
grubfm_clear_menu (void);
void
grubfm_add_menu (const char *title, const char *icon,
                 const char *hotkey, const char *src, int hidden);

/* list.c */
int
grubfm_enum_device (void);
int
grubfm_enum_file (char *dirname);

#endif
