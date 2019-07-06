#include <grub/types.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/err.h>
#include <grub/dl.h>
#include <grub/file.h>
#include <grub/term.h>
#include <grub/efi/api.h>

#include "c8fs.h"
#include "c8rom.h"

grub_efi_status_t read_file(char *path, grub_ssize_t *buf_sz, void *buf)
{
	if (!path)
	{
		grub_memcpy(buf, CHIP8ROM, CHIP8ROM_LEN);
		*buf_sz = CHIP8ROM_LEN;
		return GRUB_EFI_SUCCESS;
	}

	grub_file_t file = 0;
	
	file = grub_file_open (path, GRUB_FILE_TYPE_CAT);
	if (!file)
	{
		grub_printf("Failed opening file \"%s\": not found\n", path);
		return GRUB_EFI_NOT_FOUND;
	}

	*buf_sz = grub_file_size (file);
	
	if (*buf_sz > 4096)
	{
		grub_printf("Failed opening file \"%s\": out of resources\n", path);
		return GRUB_EFI_OUT_OF_RESOURCES;
	}

	if (grub_file_read (file, buf, *buf_sz) != *buf_sz)
	{
		grub_printf("Failed opening file \"%s\": bad buffer size\n", path);
		return GRUB_EFI_BAD_BUFFER_SIZE;
	}

	return GRUB_EFI_SUCCESS;
}