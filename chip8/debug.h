#pragma once

#include <grub/misc.h>

#define DEBUG_MODE 0

#define LOG(fmt, ...) \
			do { if (DEBUG_MODE) grub_printf(fmt, __VA_ARGS__); } while (0)
