/*  apple_gmux.c */
/*  https://github.com/sasanj/apple_gmux  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2003,2007  Free Software Foundation, Inc.
 *  Copyright (C) 2003  NIIBE Yutaka <gniibe@m17n.org>
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
#include <grub/cpu/io.h>
#include <grub/time.h>

GRUB_MOD_LICENSE ("GPLv3+");
#define GMUX_PORT_SWITCH_DISPLAY	0x10
#define GMUX_PORT_SWITCH_DDC		0x28
#define GMUX_PORT_SWITCH_EXTERNAL	0x40
#define GMUX_PORT_DISCRETE_POWER	0x50
#define GMUX_PORT_VALUE			0xc2
#define GMUX_PORT_READ			0xd0
#define GMUX_PORT_WRITE			0xd4

#define GMUX_IOSTART		0x700

typedef unsigned char u8;

enum discrete_state {STATE_ON, STATE_OFF};
enum gpu_id {IGD, DIS};

static int index_wait_ready(void)
{
  int i = 200;
  u8 gwr = grub_inb(GMUX_IOSTART + GMUX_PORT_WRITE);
  while (i && (gwr & 0x01)) {
    grub_inb(GMUX_IOSTART + GMUX_PORT_READ);
    gwr = grub_inb(GMUX_IOSTART + GMUX_PORT_WRITE);
    grub_millisleep(100);
    i--;
  }
return !!i;
}

static int index_wait_complete(void)
{
  int i = 200;
  u8 gwr = grub_inb(GMUX_IOSTART + GMUX_PORT_WRITE);
  while (i && !(gwr & 0x01)) {
    gwr = grub_inb(GMUX_IOSTART + GMUX_PORT_WRITE);
    grub_millisleep(100);
    i--;
  }
  if (gwr & 0x01)
  grub_inb(GMUX_IOSTART + GMUX_PORT_READ);
  return !!i;
}
static void index_write8(int port, u8 val)
{
	grub_outb(val, GMUX_IOSTART + GMUX_PORT_VALUE);
	index_wait_ready();
	grub_outb((port & 0xff), GMUX_IOSTART + GMUX_PORT_WRITE);
	index_wait_complete();
}

/*static u8 index_read8(int port)
{
	u8 val;
	index_wait_ready();
	grub_outb((port & 0xff), GMUX_IOSTART + GMUX_PORT_READ);
	index_wait_complete();
	val = grub_inb(GMUX_IOSTART + GMUX_PORT_VALUE);

	return val;
}
*/
static void set_discrete_state(enum discrete_state state)
{
	if (state == STATE_ON) {	// switch on dGPU
		index_write8(GMUX_PORT_DISCRETE_POWER, 1);
		index_write8(GMUX_PORT_DISCRETE_POWER, 3);
	} else {			// switch off dGPU
		index_write8(GMUX_PORT_DISCRETE_POWER, 1);
		index_write8(GMUX_PORT_DISCRETE_POWER, 0);
	}
}

/* static u8 get_discrete_state()
{
	return index_read8(GMUX_PORT_DISCRETE_POWER);
}
*/
static void switchto(enum gpu_id id)
{
	if (id == IGD) {	// switch to iGPU
		index_write8(GMUX_PORT_SWITCH_DDC, 1);
		index_write8(GMUX_PORT_SWITCH_DISPLAY, 2);
		index_write8(GMUX_PORT_SWITCH_EXTERNAL, 2);
	} else {		// switch to dGPU
		index_write8(GMUX_PORT_SWITCH_DDC, 2);
		index_write8(GMUX_PORT_SWITCH_DISPLAY, 3);
		index_write8(GMUX_PORT_SWITCH_EXTERNAL, 3);
	}
}

static grub_err_t
grub_cmd_apple_gmux (grub_extcmd_context_t ctxt __attribute__ ((unused)),
		int argc __attribute__ ((unused)),
		char **args __attribute__ ((unused)))
{
  switchto(IGD);
  set_discrete_state(STATE_OFF);
  return 0;
}

static grub_extcmd_t cmd;

GRUB_MOD_INIT(apple_gmux)
{
  cmd = grub_register_extcmd ("apple_gmux", grub_cmd_apple_gmux, 0, 0,
			      N_("Turns off Apple Discrete Grphics Card."), 0);
}

GRUB_MOD_FINI(apple_gmux)
{
  grub_unregister_extcmd (cmd);
}
