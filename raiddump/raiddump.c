/* raiddump.c - command to copy the contents of a drive to another drive */
/* (c) Maxim Suhanov, 2016 */

#include <grub/dl.h>
#include <grub/disk.h>
#include <grub/misc.h>
#include <grub/extcmd.h>
#include <grub/i18n.h>

GRUB_MOD_LICENSE ("GPLv3+");

static grub_err_t
grub_cmd_raiddump (grub_command_t cmd __attribute__ ((unused)), int argc, char **args)
{
  char buf[GRUB_DISK_SECTOR_SIZE];
  int namelen_in, namelen_out;
  char *filename_in, *filename_out;
  int dump_mode = 0;

  if ( (argc != 2) && (argc != 3) )
    return grub_error (GRUB_ERR_BAD_ARGUMENT, N_("wrong number of arguments"));

  if ( (argc == 3) && grub_strcmp (args[2], "sample") )
    return grub_error (GRUB_ERR_BAD_ARGUMENT, N_("unknown dump mode"));

  if ( (argc == 3) && !grub_strcmp (args[2], "sample") )
    dump_mode = 1;

  filename_in = grub_strdup (args[0]);
  if (! filename_in)
    return grub_error (GRUB_ERR_OUT_OF_MEMORY, N_("out of memory"));

  filename_out = grub_strdup (args[1]);
  if (! filename_out)
    {
      grub_free (filename_in);
      return grub_error (GRUB_ERR_OUT_OF_MEMORY, N_("out of memory"));
    }

  namelen_in = grub_strlen (filename_in);
  namelen_out = grub_strlen (filename_out);

  if (! grub_strcmp (filename_in, "(mem)") || ! grub_strcmp (filename_out, "(mem)") )
    {
      grub_free (filename_in);
      grub_free (filename_out);
      return grub_error (GRUB_ERR_BAD_DEVICE, N_("memory devices aren't supported"));
    }
  else if ( (filename_in[0] != '(') || (filename_in[namelen_in - 1] != ')') )
    {
      grub_free (filename_in);
      grub_free (filename_out);
      return grub_error (GRUB_ERR_BAD_ARGUMENT, N_("filename of a device expected (source)"));
    }
  else if ( (filename_out[0] != '(') || (filename_out[namelen_out - 1] != ')') )
    {
      grub_free (filename_in);
      grub_free (filename_out);
      return grub_error (GRUB_ERR_BAD_ARGUMENT, N_("filename of a device expected (target)"));
    }

  grub_disk_t disk_in, disk_out;

  filename_in[namelen_in - 1] = 0;
  disk_in = grub_disk_open (&filename_in[1]);
  if (! disk_in)
    {
      grub_free (filename_in);
      grub_free (filename_out);
      return 0;
    }

  filename_out[namelen_out- 1] = 0;
  disk_out = grub_disk_open (&filename_out[1]);
  if (! disk_out)
    {
      grub_free (filename_in);
      grub_free (filename_out);
      grub_disk_close (disk_in);
      return 0;
    }

  grub_uint64_t size_disk_in, size_disk_out;
  size_disk_in = grub_disk_get_size (disk_in);
  if ( size_disk_in == GRUB_DISK_SIZE_UNKNOWN )
    {
      grub_free (filename_in);
      grub_free (filename_out);
      grub_disk_close (disk_in);
      grub_disk_close (disk_out);
      return grub_error (GRUB_ERR_BAD_DEVICE, N_("size of the source device is unknown"));
    }

  size_disk_out = grub_disk_get_size (disk_out);
  if ( size_disk_out == GRUB_DISK_SIZE_UNKNOWN )
    {
      grub_free (filename_in);
      grub_free (filename_out);
      grub_disk_close (disk_in);
      grub_disk_close (disk_out);
      return grub_error (GRUB_ERR_BAD_DEVICE, N_("size of the target device is unknown"));
    }

  grub_uint64_t copy_len;
  copy_len = size_disk_in;
  if ( dump_mode == 1 )
    copy_len = 20480;

  grub_printf (N_("Source: %s (size, sectors: %llu)\nTarget: %s (size, sectors: %llu)\n"),
               &filename_in[1], (unsigned long long)size_disk_in, &filename_out[1], (unsigned long long)size_disk_out);

  grub_disk_addr_t i;
  grub_uint64_t b_written;

  b_written = 0;
  for ( i = 0; i < copy_len; i++ )
    {
      if ( grub_disk_read (disk_in, i, 0, sizeof(buf), buf) != GRUB_ERR_NONE )
        break;

      if ( grub_disk_write (disk_out, i, 0, sizeof(buf), buf) != GRUB_ERR_NONE )
        break;

      b_written += sizeof(buf);
    }

  grub_printf (N_("Wrote: %llu bytes\n"), (unsigned long long)b_written);

  grub_free (filename_in);
  grub_free (filename_out);
  grub_disk_close (disk_in);
  grub_disk_close (disk_out);

  return 0;
}

static grub_command_t cmd;

GRUB_MOD_INIT (raiddump)
{
  cmd = grub_register_command ("raiddump", grub_cmd_raiddump, N_("SOURCE TARGET [sample]"),
			      N_("Copy the contents of a source drive to a target drive.\nWhen \"sample\" was specified, only the first 20480 sectors are copied.\n"));
}

GRUB_MOD_FINI (raiddump)
{
  grub_unregister_command (cmd);
}
