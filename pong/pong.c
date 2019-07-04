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
#include <grub/term.h>
#include <grub/time.h>
#include <grub/efi/api.h>
#include <grub/efi/efi.h>
#include <grub/efi/graphics_output.h>

GRUB_MOD_LICENSE ("GPLv3+");

#define NUMBERHEIGHT 5
#define NUMBERWIDTH  4

grub_uint8_t numbers[][NUMBERHEIGHT][NUMBERWIDTH] = {
  //Number '0' pattern
  {{1, 1, 1, 1},
   {1, 0, 0, 1},
   {1, 0, 0, 1},
   {1, 0, 0, 1},
   {1, 1, 1, 1}},
  //Number '1' pattern
  {{0, 0, 1, 0},
   {0, 1, 1, 0},
   {0, 0, 1, 0},
   {0, 0, 1, 0},
   {0, 1, 1, 1}},
  //Number '2' pattern
  {{1, 1, 1, 1},
   {0, 0, 0, 1},
   {1, 1, 1, 1},
   {1, 0, 0, 0},
   {1, 1, 1, 1}},
  //Number '3' pattern
  {{1, 1, 1, 1},
   {0, 0, 0, 1},
   {1, 1, 1, 1},
   {0, 0, 0, 1},
   {1, 1, 1, 1}},
  //Number '4' pattern
  {{1, 0, 0, 1},
   {1, 0, 0, 1},
   {1, 1, 1, 1},
   {0, 0, 0, 1},
   {0, 0, 0, 1}},
  //Number '5' pattern
  {{1, 1, 1, 1},
   {1, 0, 0, 0},
   {1, 1, 1, 1},
   {0, 0, 0, 1},
   {1, 1, 1, 1}},
  //Number '6' pattern
  {{1, 1, 1, 1},
   {1, 0, 0, 0},
   {1, 1, 1, 1},
   {1, 0, 0, 1},
   {1, 1, 1, 1}},
  //Number '7' pattern
  {{1, 1, 1, 1},
   {0, 0, 0, 1},
   {0, 0, 1, 1},
   {0, 1, 0, 0},
   {0, 1, 0, 0}},
  //Number '8' pattern
  {{1, 1, 1, 1},
   {1, 0, 0, 1},
   {1, 1, 1, 1},
   {1, 0, 0, 1},
   {1, 1, 1, 1}},
  //Number '9' pattern
  {{1, 1, 1, 1},
   {1, 0, 0, 1},
   {1, 1, 1, 1},
   {0, 0, 0, 1},
   {1, 1, 1, 1}}
};

#define CELLSIZE 10
#define BALLSIZE CELLSIZE
#define BATCELLS 10

struct grub_efi_gop *gop = NULL;
grub_efi_guid_t graphics_output_guid = GRUB_EFI_GOP_GUID;

static struct grub_efi_gop_blt_pixel white = {0xFF, 0xFF, 0xFF, 0};
static struct grub_efi_gop_blt_pixel black = {0x00, 0x00, 0x00, 0};

static void
draw_cell (grub_uint32_t x, grub_uint32_t y, int reset)
{
  struct grub_efi_gop_blt_pixel color = (reset)?black:white;
  efi_call_10 (gop->blt, gop, &color, GRUB_EFI_BLT_VIDEO_FILL, 
               0, 0, x*CELLSIZE, y*CELLSIZE, CELLSIZE, CELLSIZE, 0);
}

static void
draw_ball (grub_uint32_t x, grub_uint32_t y)
{
  draw_cell (x, y, 0);
}

static void
erase_ball (grub_uint32_t x, grub_uint32_t y)
{
  draw_cell (x, y, 1);
}

static void
draw_bat (grub_uint32_t x, grub_uint32_t y)
{
  grub_uint32_t i;
  for (i = 0; i < BATCELLS; i++)
    draw_cell (x, y + i, 0);
}

static void
erase_bat (grub_uint32_t x, grub_uint32_t y)
{
  grub_uint32_t i;
  for (i = 0; i < BATCELLS; i++)
    draw_cell (x, y + i, 1);
}

static int
bat_block_ball (grub_uint32_t batx, grub_uint32_t baty, grub_uint32_t ballx, grub_uint32_t bally)
{
  if ((batx - ballx != 1) && (ballx - batx != 1))
    return 0;
  return (baty <= bally) && ((baty + BATCELLS) >= bally);
}

static void draw_num (grub_uint8_t score, grub_uint32_t x)
{
  grub_uint32_t i, j;
  for (i = 0; i < NUMBERHEIGHT; i++)
  {
    for (j = 0; j < NUMBERWIDTH; j++)
    {
      if (numbers[score][i][j])
        draw_cell (x + j, i, 0);
    }
  }
}

static void
draw_score (grub_uint8_t r_score, grub_uint8_t l_score, const grub_uint32_t game_width)
{
  grub_uint32_t x = (game_width / 2);
  draw_num (r_score, x - 1 - NUMBERWIDTH);
  draw_num (l_score, x + 1);
}

static void
erase_score (const grub_uint32_t game_width)
{
  grub_uint32_t i, j;
  grub_uint32_t start = (game_width / 2) - 1 - NUMBERWIDTH;
  for (i = 0; i < NUMBERHEIGHT; i++)
  {
    for (j = start; j < start + 2 + 2 * NUMBERWIDTH; j++)
      draw_cell (j, i, 1);
  }
}

static grub_err_t
grub_cmd_pong (grub_extcmd_context_t ctxt __attribute__ ((unused)),
		int argc __attribute__ ((unused)),
		char **args __attribute__ ((unused)))
{
  grub_efi_status_t status;
  grub_efi_boot_services_t *b;
  b = grub_efi_system_table->boot_services;
  status = efi_call_3 (b->locate_protocol, &graphics_output_guid, NULL, (void **)&gop);
  if (status != GRUB_EFI_SUCCESS)
  {
    grub_printf ("Unable to locate Graphics Output Protocol\n");
    return 0;
  }
  const grub_uint32_t screen_width = gop->mode->info->width;
  const grub_uint32_t screen_height = gop->mode->info->height;
  /* cls */
  efi_call_10 (gop->blt, gop, &black, GRUB_EFI_BLT_VIDEO_FILL, 
               0, 0, 0, 0, screen_width, screen_height, 0);

  const grub_uint32_t game_width = screen_width / CELLSIZE;
  const grub_uint32_t game_height = screen_height / CELLSIZE;

  grub_uint32_t ballx = game_width / 2;
  grub_uint32_t bally = game_height / 2;
  grub_uint32_t l_bat_pos = (game_height / 2) - (BATCELLS / 2);
  grub_uint32_t r_bat_pos = (game_height / 2) - (BATCELLS / 2);
  const grub_uint32_t lowest_pos = game_height - BATCELLS;
  const grub_uint32_t x_r_bat = game_width - 1;
  
  draw_bat (0, l_bat_pos);
  draw_bat (x_r_bat, r_bat_pos);
  draw_ball (ballx, bally);
  
  grub_int32_t speedx = -1;
  grub_int32_t speedy = 1;
  const grub_uint32_t bat_speed = 4;
  grub_uint8_t l_score = 0;
  grub_uint8_t r_score = 0;
  
  draw_score (l_score, r_score, game_width);
  
  int stop = 0;
  while (!stop)
  {
    int key = 0;
    do
    {
      key = grub_getkey_noblock ();
      if (key == GRUB_TERM_ESC)
        stop = 1;
      if (key == 's')
      {
        erase_bat (0, l_bat_pos);
        if (l_bat_pos < bat_speed)
          l_bat_pos = 0;
        else
          l_bat_pos -= bat_speed;
        draw_bat (0, l_bat_pos);
      }
      else if (key == 'x')
      {
        erase_bat (0, l_bat_pos);
        l_bat_pos += bat_speed;
        if (l_bat_pos > lowest_pos)
          l_bat_pos = lowest_pos;
        draw_bat (0, l_bat_pos);
      }
      if (key == GRUB_TERM_KEY_UP)
      {
        erase_bat (x_r_bat, r_bat_pos);
        if (r_bat_pos < bat_speed)
          r_bat_pos = 0;
        else
          r_bat_pos -= bat_speed;
        draw_bat (x_r_bat, r_bat_pos);
      }
      else if (key == GRUB_TERM_KEY_DOWN)
      {
        erase_bat (x_r_bat, r_bat_pos);
        r_bat_pos += bat_speed;
        if (r_bat_pos > lowest_pos)
          r_bat_pos = lowest_pos;
        draw_bat (x_r_bat, r_bat_pos);
      }
    } while (key != 0);
    erase_ball (ballx, bally);
    
    if(ballx == 0)
    {
      ++r_score;
      if(r_score >= 9)
        stop = 1;
      ballx = game_width / 2;
      bally = game_height / 2;
      speedx *= -1;
      erase_score(game_width);
      draw_score(l_score, r_score, game_width);
    }
    else if(ballx == game_width)
    {
      ++l_score;
      if(l_score >= 9)
        stop = 1;
      ballx = game_width / 2;
      bally = game_height / 2;
      speedx *= -1;
      erase_score(game_width);
      draw_score(l_score, r_score, game_width);
    }
    else if(bat_block_ball(0, l_bat_pos, ballx, bally) ||
            bat_block_ball(x_r_bat, r_bat_pos, ballx, bally))
    {
      speedx *= -1;
    }

    if(bally <= 0 || bally >= game_height)
      speedy *= -1;

    ballx += speedx;
    bally += speedy;
    draw_ball(ballx, bally);

    grub_millisleep (50);
  }
  grub_printf ("Game over\n");
  return 0;
}

static grub_extcmd_t cmd;

GRUB_MOD_INIT(pong)
{
  cmd = grub_register_extcmd ("pong", grub_cmd_pong, 0, 0,
			      N_("UEFI Pong game.\ns / x : left bat\nUp / Down : right bat"), 0);
}

GRUB_MOD_FINI(pong)
{
  grub_unregister_extcmd (cmd);
}
