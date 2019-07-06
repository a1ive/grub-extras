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

 /* based on https://github.com/78541234h/UEFI_Final_Project */

#include <grub/types.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/err.h>
#include <grub/disk.h>
#include <grub/dl.h>
#include <grub/extcmd.h>
#include <grub/file.h>
#include <grub/i18n.h>
#include <grub/term.h>
#include <grub/time.h>
#include <grub/efi/api.h>
#include <grub/efi/efi.h>
#include <grub/efi/graphics_output.h>
#include "level.h"

GRUB_MOD_LICENSE ("GPLv3+");

static const struct grub_arg_option options_sokoban[] = {
  {"zoom", 'z', 0, N_("Magnifying the image size."), 0, 0},
  {0, 0, 0, 0, 0, 0}
};

static unsigned char *sokoban_disk_addr;
static unsigned int sokoban_disk_size = 0;

static int
grub_sokoban_iterate (grub_disk_dev_iterate_hook_t hook, void *hook_data,
		      grub_disk_pull_t pull)
{
  if (pull != GRUB_DISK_PULL_NONE)
    return 0;
  return hook ("sokoban", hook_data);
}

static grub_err_t
grub_sokoban_open (const char *name, grub_disk_t disk)
{
  if (grub_strcmp (name, "sokoban"))
      return grub_error (GRUB_ERR_UNKNOWN_DEVICE, "not a sokoban memdisk");
  disk->total_sectors = sokoban_disk_size / GRUB_DISK_SECTOR_SIZE;
  disk->max_agglomerate = GRUB_DISK_MAX_MAX_AGGLOMERATE;
  disk->id = 0;
  return GRUB_ERR_NONE;
}

static void
grub_sokoban_close (grub_disk_t disk __attribute((unused)))
{
}

static grub_err_t
grub_sokoban_read (grub_disk_t disk __attribute((unused)), grub_disk_addr_t sector,
		    grub_size_t size, char *buf)
{
  grub_memcpy (buf, sokoban_disk_addr + (sector << GRUB_DISK_SECTOR_BITS), size << GRUB_DISK_SECTOR_BITS);
  return 0;
}

static grub_err_t
grub_sokoban_write (grub_disk_t disk __attribute((unused)), grub_disk_addr_t sector,
		     grub_size_t size, const char *buf)
{
  grub_memcpy (sokoban_disk_addr + (sector << GRUB_DISK_SECTOR_BITS), buf, size << GRUB_DISK_SECTOR_BITS);
  return 0;
}

static struct grub_disk_dev grub_sokoban_dev =
  {
    .name = "sokoban",
    .id = GRUB_DISK_DEVICE_MEMDISK_ID,
    .disk_iterate = grub_sokoban_iterate,
    .disk_open = grub_sokoban_open,
    .disk_close = grub_sokoban_close,
    .disk_read = grub_sokoban_read,
    .disk_write = grub_sokoban_write,
    .next = 0
  };

static struct grub_efi_gop *gop = NULL;
static grub_efi_guid_t graphics_output_guid = GRUB_EFI_GOP_GUID;

static int CELLSIZE = 1;

typedef enum
{
  UP, DOWN, LEFT, RIGHT
} direction_t;

typedef struct
{
  int x;
  int y;
  char text[3];
} node;

typedef struct
{
  int x;
  int y;
} next_move_t;

static direction_t move_directions;
static int cols = 0;
static int rows = 0;
static int box_num = 0;
static node man = {0, 0, "\0"};
static int x_pos[10] = { 0 };
static int y_pos[10] = { 0 };

#define WALL         '#'
#define DESTINATION  'X'
#define BOX          'O'
#define REACHED      'Q'
#define EMPTY        ' '
#define PLAYER       '@'

static struct grub_efi_gop_blt_pixel BLACK[1] = {{ 0,0,0,0 }};
static struct grub_efi_gop_blt_pixel SKIN[1] = {{216,229,247,0}};
static struct grub_efi_gop_blt_pixel WHITE[1] = {{255,255,255,0}};
static struct grub_efi_gop_blt_pixel LIGHT_BROWN[1] = {{47,81,175,0}};
static struct grub_efi_gop_blt_pixel DARK_BROWN[1] = {{35,40,147,0}};
static struct grub_efi_gop_blt_pixel LIGHT_BLUE[1] = {{229,183,159,0}};
static struct grub_efi_gop_blt_pixel DARK_BLUE[1] = {{161,128,44,0}};
static struct grub_efi_gop_blt_pixel LIGHT_RED[1] = {{108,127,247,0}};
static struct grub_efi_gop_blt_pixel PINK[1] = {{201,181,255,0}};
static struct grub_efi_gop_blt_pixel RED[1] = {{0,0,255,0}};
static struct grub_efi_gop_blt_pixel YELLOW[1] = {{0,255,255,0}};

static char map[50][50];

static grub_uint8_t MAN_PIC[20][13] =
{
  {0,0,0,1,1,1,1,1,1,1},
  {0,0,1,2,2,2,2,2,2,2,1},
  {0,1,2,2,3,3,3,3,3,2,2,1},
  {0,1,3,4,4,4,4,4,4,4,3,1},
  {0,1,3,4,1,4,4,4,1,4,3,1},
  {0,1,3,4,1,4,4,4,1,4,3,1},
  {0,1,3,5,4,4,4,4,4,5,3,1},
    {0,1,3,4,1,4,4,4,1,4,3,1},
  {0,1,3,4,4,1,1,1,4,4,3,1},
  {0,0,1,4,4,4,4,4,4,4,1},
  {0,1,6,6,7,7,7,7,7,6,6,1},
  {1,6,6,6,6,7,7,7,6,6,6,6,1},
  {1,6,1,6,6,7,7,7,6,6,1,6,1},
  {1,7,1,6,6,7,7,7,6,6,1,7,1},
  {1,4,1,6,6,7,7,7,6,6,1,4,1},
  {0,1,1,7,7,7,7,7,7,7,1,1},
  {0,0,1,8,9,9,9,9,9,8,1},
  {0,0,1,9,9,1,1,1,9,9,1},
  {0,0,1,6,6,1,6,1,6,6,1},
  {0,0,0,1,1,0,0,0,1,1}
};

static grub_uint8_t BOX_PIC[16][16] =
{
  { 0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0 },
  { 3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1 },
  { 3,2,1,1,2,2,2,3,3,2,2,2,1,1,2,1 },
  { 3,2,1,1,2,2,3,3,3,3,2,2,1,1,2,1 },
  { 3,2,2,2,2,3,3,3,3,3,3,2,2,2,2,1 },
  { 3,2,2,2,2,3,3,3,3,3,3,2,2,2,2,1 },
  { 3,2,2,2,2,3,3,3,3,3,3,2,2,2,2,1 },
  { 3,2,2,2,2,3,3,3,3,3,3,2,2,2,2,1 },
  { 3,2,2,2,2,2,3,3,3,3,2,2,2,2,2,1 },
  { 3,2,2,2,2,2,2,3,3,2,2,2,2,2,2,1 },
  { 3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1 },
  { 3,2,2,2,2,2,2,3,3,2,2,2,2,2,2,1 },
  { 3,2,1,1,2,2,3,3,3,3,2,2,1,1,2,1 },
  { 3,2,1,1,2,2,2,3,3,2,2,2,1,1,2,1 },
  { 3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1 },
  { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
};

static grub_uint8_t DESTINATION_PIC[19][15] =
{
  { 1,1,1,1,1,2,2,2,1,1,1,1,1,1,1},
  { 1,1,2,2,2,2,2,2,2,1,1,1,1,1,1},
  { 1,1,2,2,2,2,2,2,2,2,1,1,2,2,1},
  { 1,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
  { 1,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
  { 1,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
  { 1,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
  { 1,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
  { 1,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
  { 1,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
  { 1,1,2,2,1,2,2,2,2,2,2,2,2,2,1},
  { 1,1,2,2,1,1,1,1,2,2,2,2,2,1,1},
  { 1,1,2,2,1,1,1,1,1,1,2,1,1,1,1},
  { 1,1,2,2,1,1,1},
  { 1,1,2,2,1,1,1},
  { 1,1,2,2,1,1,1},
  { 1,1,2,2,1,1,1},
  { 1,1,2,2,1,1,1},
  { 1,1,2,2,1,1,1},
};

static void
draw_rectangle (int x, int y, int w, int l, struct grub_efi_gop_blt_pixel* color)
{
  efi_call_10 (gop->blt, gop, color, GRUB_EFI_BLT_VIDEO_FILL, 0, 0,
               x*CELLSIZE, y*CELLSIZE, w*CELLSIZE, l*CELLSIZE, 0);
}

static void
copy_rectangle (int s_x, int s_y, int e_x, int e_y, int width, int length)
{
  efi_call_10 (gop->blt, gop, NULL, GRUB_EFI_BLT_VIDEO_TO_VIDEO,
               s_x*CELLSIZE, s_y*CELLSIZE, e_x*CELLSIZE, e_y*CELLSIZE,
               width*CELLSIZE, length*CELLSIZE, 0);
}

static void
clear_screen (void)
{
  efi_call_1 (grub_efi_system_table->con_out->clear_screen,
              grub_efi_system_table->con_out);
}

static void
print_xy (const char *str, grub_efi_uintn_t x, grub_efi_uintn_t y)
{
  int len = (grub_strlen (str) + 1) * sizeof (grub_efi_char16_t);
  grub_efi_char16_t *ret = NULL;
  grub_efi_char16_t *p16;
  char *p8 = (char *) str;
  ret = p16 = grub_malloc (len);
  if (!ret)
    return;
  while (*p8)
    *(p16++) = *(p8++);
  *p16 = 0;
  efi_call_3 (grub_efi_system_table->con_out->set_cursor_position,
              grub_efi_system_table->con_out, x, y);
  efi_call_2 (grub_efi_system_table->con_out->output_string,
              grub_efi_system_table->con_out, ret);
  grub_free (ret);
}

static void
draw_box (void)
{
  struct grub_efi_gop_blt_pixel common[1] = {{0,0,0,0}};
  for (int i = 0; i < 16; i++)
  {
    for (int j = 0; j < 16; j++)
    {
      switch (BOX_PIC[i][j])
      {
      case 1:
        common[0] = BLACK[0];
        break;
      case 2:
        common[0] = LIGHT_RED[0];
        break;
      case 3:
        common[0] = LIGHT_BROWN[0];
        break;
      }
      efi_call_10 (gop->blt, gop, common, GRUB_EFI_BLT_VIDEO_FILL,
                   0, 0, (j + 97)*CELLSIZE, (i + 7)*CELLSIZE, 1*CELLSIZE, 1*CELLSIZE, 0);
    }
  }
}

static void
draw_destination (void)
{
  struct grub_efi_gop_blt_pixel common[1] = {{0,0,0,0}};
  for (int i = 0; i < 19; i++)
  {
    for (int j = 0; j < 15; j++)
    {
      switch (DESTINATION_PIC[i][j])
      {
      case 1:
        common[0] = BLACK[0];
        break;
      case 2:
        common[0] = YELLOW[0];
        break;
      case 3:
        common[0] = RED[0];
        break;
      case 4:
        common[0] = DARK_BROWN[0];
        break;
      default:
        break;
      }
      efi_call_10 (gop->blt, gop, common, GRUB_EFI_BLT_VIDEO_FILL,
                   0, 0, (j + 67)*CELLSIZE, (i + 5)*CELLSIZE, 1*CELLSIZE, 1*CELLSIZE, 0);
    }
  }
}

static void
draw_man (void)
{
  struct grub_efi_gop_blt_pixel common[1] = {{0,0,0,0}};
  for (int i = 0; i < 19; i++)
  {
    for (int j = 0; j < 13; j++)
    {
      switch (MAN_PIC[i][j]) {
      case 1:
        common[0] = BLACK[0];
        break;
      case 2:
        common[0] = LIGHT_BROWN[0];
        break;
      case 3:
        common[0] = DARK_BROWN[0];
        break;
      case 4:
        common[0] = SKIN[0];
        break;
      case 5:
        common[0] = LIGHT_RED[0];
        break;
      case 6:
        common[0] = WHITE[0];
        break;
      case 7:
        common[0] = PINK[0];
        break;
      case 8:
        common[0] = DARK_BLUE[0];
        break;
      case 9:
        common[0] = LIGHT_BLUE[0];
        break;
      }
      efi_call_10 (gop->blt, gop, common, GRUB_EFI_BLT_VIDEO_FILL,
                   0, 0, (j + 38)*CELLSIZE, (i + 4)*CELLSIZE, 1*CELLSIZE, 1*CELLSIZE, 0);
    }
  }
}

static void
draw_map (void)
{
  /* WALL */
  draw_rectangle(1, 1, 28, 13,DARK_BROWN);
  draw_rectangle(0, 16, 14, 13,DARK_BROWN);
  draw_rectangle(17, 16, 14, 13,DARK_BROWN);
  
  draw_destination();
  draw_man();
  draw_box();
  int i, j;
  for (i = 0; i < rows; i++)
  {
    for (j = 0; j < cols; j++)
    {
      char tmp = map[i][j];
      if (tmp == WALL)
        copy_rectangle(0, 0, 30 * j, 30 * i, 30, 30);
      else if (tmp == DESTINATION)
        copy_rectangle(61, 0, 30 * j, 30 * i, 30, 30);
      else if (tmp == BOX || tmp == REACHED )
        copy_rectangle(91, 0, 30 * j, 30 * i, 30, 30);
      else if (tmp == PLAYER)
        copy_rectangle(31, 0, 30 * j, 30 * i, 30, 30);
    }
  }
  /* tips */
  print_xy ("Move: Arrow keys", 60, 1);
  print_xy ("Replay: F1", 60, 2);
  print_xy ("Exit: ESC", 60, 3);
}

static grub_err_t
load_map (unsigned char *buffer, grub_ssize_t size)
{
  char c; 
  int _row = 0;
  int _col = 0;
  int i;
  for (i = 0; i < size; i++)
  {
    c = buffer[i];
    if (c != 0)
    {
      if (c == '\n')
      {
        _row++;
        _col = 0;
      }
      else
      {
        if (_row < 50 && _col < 50)
          map[_row][_col++] = c;
        else
        {
          grub_error (GRUB_ERR_BAD_OS, N_("map error"));
          goto fail;
        }
      }
    }
  }
  rows = _row + 1;
  cols = _col;
  char tmp;
  int j;
  box_num = 0;
  for (i = 0; i < rows; i++)
  {
    for (j = 0; j < cols; j++)
    {
      tmp = map[i][j];
      if (tmp == DESTINATION || tmp == REACHED)
      {
        x_pos[box_num] = j;
        y_pos[box_num] = i;
        box_num++;
      }
      if (tmp == PLAYER)
      {
        //player location
        man.x = j;
        man.y = i;
        man.text[0] = tmp;
        man.text[1] = '\0';
      }
    }
  }
fail:
  return grub_errno;
}

static void
update_map (int x, int y, char c)
{ 
  switch (c)
  {
  case DESTINATION:
    copy_rectangle(61, 0, 30 * x, 30 * y, 29, 30);
    break;
  case REACHED:
  case BOX:
    copy_rectangle(90, 0, 30 * x, 30 * y, 30, 30);
    break;
  case PLAYER:
    copy_rectangle(31, 0, 30 * x, 30 * y, 29, 30);
    break;
  case EMPTY:
    draw_rectangle(30 * x, 30 * y, 30, 30, BLACK);
    break;
  case WALL:
    copy_rectangle(0, 0, 30 * x, 30 * y, 30, 30);
    break;
  }
}

static int
filrate (void)
{
  int i;
  int j = 0;
  char c;
  for (i = 0;  i < box_num; i++) {
    c = map[y_pos[i]][x_pos[i]];
    if (c == EMPTY)
    {
      map[y_pos[i]][x_pos[i]] = DESTINATION;
    }

    if (c == BOX || c== REACHED)
    {
      map[y_pos[i]][x_pos[i]] = REACHED;
      j++;
    }
  }
  if (j == box_num)
    return 1;
  else
    return 0;
}

static grub_err_t
draw_game (void *buffer, grub_ssize_t size)
{
  clear_screen ();
  load_map (buffer, size);
  if (grub_errno != GRUB_ERR_NONE)
    goto fail;
  draw_map ();
fail:
  return grub_errno;
}

static int
play_game(void)
{
  char thing_front_man;
  char thing_front_box;
  node old_pos;
  old_pos.x = man.x;
  old_pos.y = man.y;
  int _move = 0;
  int _x = 0;
  int _y = 0;
  int __x = 4;
  int __y = 3;

  next_move_t next_move = {0, 0};
  switch (move_directions)
  {
  case UP:
    next_move.x = 0;
    next_move.y = -1;
    break;
  case DOWN:
    next_move.x = 0;
    next_move.y = 1;
    break;
  case LEFT:
    next_move.x = -1;
    next_move.y = 0;
    break;
  case RIGHT:
    next_move.x = 1;
    next_move.y = 0;
    break;
  }

  _x = man.x + next_move.x;
  _y = man.y + next_move.y;
  thing_front_man = map[_y][_x];
  if (thing_front_man != WALL)
  {
    if (thing_front_man == EMPTY || thing_front_man == DESTINATION)
    {
      man.y += next_move.y;
      man.x += next_move.x;
      map[man.y][man.x] = PLAYER;
      map[old_pos.y][old_pos.x] = EMPTY;
      _move = 1;
    }
    else
    {
      /* BOX */
      __x = _x + next_move.x;
      __y = _y + next_move.y;
      thing_front_box = map[__y][__x];
      if (thing_front_box == DESTINATION || thing_front_box == EMPTY)
      {
        map[__y][__x] = BOX;
        map[_y][_x] = PLAYER;
        map[man.y][man.x] = EMPTY;
        man.x += next_move.x;
        man.y += next_move.y;
        _move = 1;
      }
    }
  }
  if (_move == 1)
  {
    if (filrate())
    {
      /* WIN */
      return 1;
    }
    update_map(old_pos.x, old_pos.y, map[old_pos.y][old_pos.x]);
    update_map(_x, _y, map[_y][_x]);
    if (__x != 0)
      update_map(__x, __y, map[__y][__x]);
  }
  return 0;
}

static int
game (const char *file_name)
{
  int win = 0;
  unsigned char *buffer = NULL;
  grub_file_t file = 0;
  grub_ssize_t size = 0;
  file = grub_file_open (file_name, GRUB_FILE_TYPE_CAT);
  if (! file)
    goto fail;
  size = grub_file_size (file);
  if (!size)
  {
    grub_error (GRUB_ERR_BAD_OS, N_("premature end of file %s"), file_name);
    goto fail;
  }
  buffer = grub_malloc (size);
  if (!buffer)
  {
    grub_error (GRUB_ERR_OUT_OF_MEMORY, N_("out of memory"));
    goto fail;
  }
  if (grub_file_read (file, buffer, size) != size)
  {
    if (grub_errno == GRUB_ERR_NONE)
      grub_error (GRUB_ERR_BAD_OS, N_("premature end of file %s"), file_name);
    goto fail;
  }

  int refresh = 1;
  do
  {
    if (refresh)
      draw_game (buffer, size);
    refresh = 0;
    if (grub_errno != GRUB_ERR_NONE)
      return -1;
    int key = 0;
    do
    {
      key = grub_getkey_noblock ();
    }
    while (key != GRUB_TERM_KEY_UP &&
           key != GRUB_TERM_KEY_DOWN &&
           key != GRUB_TERM_KEY_LEFT &&
           key != GRUB_TERM_KEY_RIGHT &&
           key != GRUB_TERM_KEY_F1 &&
           key != GRUB_TERM_ESC);
    if (key == GRUB_TERM_KEY_UP)
      move_directions = UP;
    else if (key == GRUB_TERM_KEY_DOWN)
      move_directions = DOWN;
    else if (key == GRUB_TERM_KEY_LEFT)
      move_directions = LEFT;
    else if (key == GRUB_TERM_KEY_RIGHT)
      move_directions = RIGHT;
    else if (key == GRUB_TERM_KEY_F1)
    {
      refresh = 1;
      continue;
    }
    else if (key == GRUB_TERM_ESC)
    {
      win = 0;
      break;
    }
    else
      continue;
    if (play_game ())
    {
      win = 1;
      break;
    }
  } while (1);
  grub_free (buffer);
  return win;
fail:
  if (buffer)
    grub_free (buffer);
  return -1;
}

static grub_err_t
grub_cmd_sokoban (grub_extcmd_context_t ctxt,
		int argc, char **args)
{
  struct grub_arg_list *state = ctxt->state;
  /* load game data */
  sokoban_disk_size = ALIGN_UP(sokoban_data_len, GRUB_DISK_SECTOR_SIZE);
  sokoban_disk_addr = grub_malloc (sokoban_disk_size);
  grub_memcpy (sokoban_disk_addr, sokoban_data, sokoban_data_len);
  grub_disk_dev_register (&grub_sokoban_dev);
  /* open Graphics_Output Protocol */
  grub_efi_status_t status;
  grub_efi_simple_text_output_mode_t saved_console_mode;
  grub_efi_boot_services_t *b;
  b = grub_efi_system_table->boot_services;
  status = efi_call_3 (b->locate_protocol, &graphics_output_guid, NULL, (void **)&gop);
  if (status != GRUB_EFI_SUCCESS)
  {
    grub_error (GRUB_ERR_BAD_OS, "Unable to locate Graphics Output Protocol.");
    goto fail;
  }
  /* Save the current console cursor position and attributes */
  grub_memcpy(&saved_console_mode,
              grub_efi_system_table->con_out->mode,
              sizeof(grub_efi_simple_text_output_mode_t));
  efi_call_2 (grub_efi_system_table->con_out->enable_cursor,
              grub_efi_system_table->con_out, 0);
  efi_call_2 (grub_efi_system_table->con_out->set_attributes,
              grub_efi_system_table->con_out,
              GRUB_EFI_TEXT_ATTR(GRUB_EFI_LIGHTGRAY, GRUB_EFI_BACKGROUND_BLACK));
  efi_call_3 (grub_efi_system_table->con_out->set_cursor_position,
              grub_efi_system_table->con_out, 0, 0);

  if (state[0].set)
  {
    CELLSIZE = 2;
  }
  if (argc == 1)
  {
    game (args[0]);
  }
  else
  {
    grub_dl_load ("newc");
    int i = 1;
    int win = 0;
    char name[20];
    for (i = 1; i < 35; i++)
    {
      grub_snprintf (name, 20, "(sokoban)/%d.txt", i);
      win = game (name);
      if (win == 1)
        continue;
      else
        break;
    }
  }

  efi_call_2 (grub_efi_system_table->con_out->enable_cursor,
              grub_efi_system_table->con_out, saved_console_mode.cursor_visible);
  efi_call_3 (grub_efi_system_table->con_out->set_cursor_position,
              grub_efi_system_table->con_out,
              saved_console_mode.cursor_column,
              saved_console_mode.cursor_row);
  efi_call_2 (grub_efi_system_table->con_out->set_attributes,
              grub_efi_system_table->con_out,
              saved_console_mode.attribute);
  clear_screen ();
  if (sokoban_disk_addr)
    grub_free (sokoban_disk_addr);
  grub_disk_dev_unregister (&grub_sokoban_dev);
fail:
  return grub_errno;
}

static grub_extcmd_t cmd;

GRUB_MOD_INIT(sokoban)
{
  cmd = grub_register_extcmd ("sokoban", grub_cmd_sokoban, 0, N_("[FILE]"),
				  N_("UEFI sokoban game."), options_sokoban);
}

GRUB_MOD_FINI(sokoban)
{
  grub_unregister_extcmd (cmd);
}