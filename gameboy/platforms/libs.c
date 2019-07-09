#include "libs.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

#include <grub/types.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/file.h>
/*
void *uefi_fopen(const char * const path,
					const char * const access __attribute__ ((unused))) {
    
    grub_file_t *file = NULL;

	*file = grub_file_open (path, GRUB_FILE_TYPE_CAT);
	
	if (!*file)
	{
		grub_printf ("Error opening file %s\n", path);
		return 0;
	}

	return file;
}

int uefi_fseek(void *stream, long int offset, int origin __attribute__ ((unused))) {
    grub_file_t* file = (grub_file_t *)stream;
    int res  = grub_file_seek (*file, offset);
	return res;
}

size_t uefi_fread(void *ptr, size_t size, size_t count, void *stream) {
    grub_file_t* file = (grub_file_t *)stream;
    grub_file_read (*file, ptr, size * count);
    return count;
}

void uefi_fclose(void *stream) {
    grub_file_t* file = (grub_file_t *)stream;
    grub_file_close (*file);
}
*/
char *uefi_strncpy(char *dest, const char * const src, size_t len) {
    return grub_strncpy (dest, src, len);
}

char *uefi_strcat(char *dest, const char * const src) {
    return grub_strcat (dest, src);
}

void *uefi_memmove(void *dst, const void *src, size_t len) {
    return grub_memmove (dst, src, len);
}
