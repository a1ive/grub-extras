#pragma once

#include <grub/types.h>
#include <grub/misc.h>
#include <grub/efi/api.h>

#include "debug.h"

grub_efi_status_t read_file(char *path, grub_ssize_t *buf_sz, void *buf);