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
#include <grub/device.h>
#include <grub/file.h>
#include <grub/fs.h>
#include <grub/normal.h>
#include <grub/term.h>
#include <grub/menu.h>

GRUB_MOD_LICENSE ("GPLv3+");

static void
grubfm_clear_menu (void)
{
  grub_menu_t menu = grub_env_get_menu();
  menu->entry_list = NULL;
  menu->size=0;
}

static void
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

static int
grubfm_enum_device_iter (const char *name,
                         void *data __attribute__ ((unused)))
{
  if (grub_strcmp (name, "memdisk") == 0 ||
      grub_strcmp (name, "proc") == 0 ||
      grub_strcmp (name, "python") == 0)
    return 0;
  grub_device_t dev;

  dev = grub_device_open (name);
  if (dev)
  {
    grub_fs_t fs;

    fs = grub_fs_probe (dev);
    if (fs)
    {
      char *uuid = NULL;
      char *label = NULL;
      if (fs->fs_uuid)
      {
        int err;
        err = fs->fs_uuid (dev, &uuid);
        if (err)
        {
          grub_errno = 0;
          uuid = NULL;
        }
      }

      if (fs->fs_label)
      {
        int err;
        err = fs->fs_label (dev, &label);
        if (err)
        {
          grub_errno = 0;
          label = NULL;
        }
      }

      char *title = NULL;
      title = grub_xasprintf ("(%s) [%s] %s %s", name,
                grub_strlen(label)? label : "NO LABEL",
                fs->name, uuid? uuid : "");
      char *src = NULL;
      src = grub_xasprintf ("grubfm \"(%s)/\"", name);
      grubfm_add_menu (title, "hdd", NULL, src, 0);
      grub_free (title);
      grub_free (src);
      if (label)
        grub_free (label);
      if (uuid)
        grub_free (uuid);
    }
    else
      grub_errno = 0;
    grub_device_close (dev);
  }
  else
    grub_errno = 0;

  return 0;
}

static int
grubfm_enum_file_iter (const char *filename,
                       const struct grub_dirhook_info *info,
                       void *data)
{
  char *dirname = data;
  if (grub_strcmp (filename, ".") == 0 || grub_strcmp (filename, "..") == 0)
    return 0;

  char *pathname;

  if (dirname[grub_strlen (dirname) - 1] == '/')
    pathname = grub_xasprintf ("%s%s", dirname, filename);
  else
    pathname = grub_xasprintf ("%s/%s", dirname, filename);
  if (!pathname)
    return 1;

  if (info->dir)
  {
    char *title = NULL;
    title = grub_xasprintf ("%-12s [%s]", N_("DIR"), filename);
    char *src = NULL;
    src = grub_xasprintf ("grubfm \"%s/\"", pathname);
    grubfm_add_menu (title, "dir", NULL, src, 0);
    grub_free (title);
    grub_free (src);
  }
  else
  {
    grub_file_t file;
    

/* XXX: For ext2fs symlinks are detected as files while they
 * should be reported as directories.  */
    file = grub_file_open (pathname, GRUB_FILE_TYPE_GET_SIZE |
                            GRUB_FILE_TYPE_NO_DECOMPRESS);
    if (! file)
    {
      grub_errno = 0;
      grub_free (pathname);
      return 0;
    }
    grub_file_close (file);

    char *title = NULL;
    title = grub_xasprintf ("%-12s %s",
        grub_get_human_size (file->size, GRUB_HUMAN_SIZE_SHORT), filename);
    char *src = NULL;
    src = grub_xasprintf ("echo \"%s\"\ngetkey", pathname);
    grubfm_add_menu (title, "file", NULL, src, 0);
    grub_free (title);
    grub_free (src);
  }
  grub_free (pathname);
  return 0;
}

static int
grubfm_enum_file (char *dirname)
{
  char *device_name;
  grub_fs_t fs;
  const char *path;
  grub_device_t dev;

  char *parent_dir = NULL;
  char *src = NULL;
  parent_dir = grub_strdup (dirname);
  *grub_strrchr (parent_dir, '/') = 0;
  if (grub_strrchr (parent_dir, '/'))
    *grub_strrchr (parent_dir, '/') = 0;
  else if (parent_dir)
    parent_dir[0] = '\0';
  if (grub_strlen (parent_dir))
    src = grub_xasprintf ("grubfm \"%s/\"", parent_dir);
  else
    src = grub_xasprintf ("grubfm");

  grubfm_add_menu (N_("Back"), "go-previous", NULL, src, 0);
  grub_free (src);
  if (parent_dir)
    grub_free (parent_dir);

  device_name = grub_file_get_device_name (dirname);
  dev = grub_device_open (device_name);
  if (!dev)
    goto fail;

  fs = grub_fs_probe (dev);
  path = grub_strchr (dirname, ')');
  if (!path)
    path = dirname;
  else
    path++;

  if (!path && !device_name)
  {
    grub_error (GRUB_ERR_BAD_ARGUMENT, "invalid argument");
    goto fail;
  }

  if (!*path)
  {
    if (grub_errno == GRUB_ERR_UNKNOWN_FS)
      grub_errno = GRUB_ERR_NONE;
    goto fail;
  }
  else if (fs)
  {
    (fs->fs_dir) (dev, path, grubfm_enum_file_iter, dirname);
  }

 fail:
  if (dev)
    grub_device_close (dev);

  grub_free (device_name);

  return 0;
}

static grub_err_t
grub_cmd_grubfm (grub_extcmd_context_t ctxt __attribute__ ((unused)),
        int argc, char **args)
{
  grubfm_clear_menu ();
  if (argc == 0)
    grub_device_iterate (grubfm_enum_device_iter, NULL);
  else
    grubfm_enum_file (args[0]);
  return 0;
}

static grub_extcmd_t cmd;

GRUB_MOD_INIT(grubfm)
{
  cmd = grub_register_extcmd ("grubfm", grub_cmd_grubfm, 0, 0,
                  N_("GRUB file manager."), 0);
}

GRUB_MOD_FINI(grubfm)
{
  grub_unregister_extcmd (cmd);
}
