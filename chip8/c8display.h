#pragma once

#include "debug.h"

#define DISPLAY_WIDTH 			64
#define DISPLAY_HEIGHT 			32
#define PX_SCALE_FACTOR			8

typedef enum
{
	DS_OK = 0,
	DS_BUSY,
	DS_ERROR
} display_status_t;

typedef enum
{
	PX_CLEAN = 0,
	PX_DIRTY,
	PX_ERROR
} px_status_t;

typedef enum
{
	PX_BLANK = 0,
	PX_FILLED
} px_content_t;

display_status_t display_init(void);
display_status_t display_clear(void);
px_status_t display_px_set(unsigned const int x, unsigned const int y, const px_content_t value);
px_status_t display_px_xor(unsigned const int x, unsigned const int y, const px_content_t value);
px_status_t display_px_flip(unsigned const int x, unsigned const int y);