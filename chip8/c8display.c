#include <grub/types.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/err.h>
#include <grub/dl.h>
#include <grub/term.h>
#include <grub/efi/api.h>
#include <grub/efi/efi.h>
#include <grub/efi/graphics_output.h>

#include "c8display.h"

static struct grub_efi_gop *graph_out = NULL;
static grub_efi_guid_t graphics_output_guid = GRUB_EFI_GOP_GUID;

static struct grub_efi_gop_blt_pixel PX_WHITE = {255, 255, 255, 0};
static struct grub_efi_gop_blt_pixel PX_BLACK = {0, 0, 0, 0};

static grub_efi_char8_t display[DISPLAY_WIDTH * DISPLAY_HEIGHT] = {0};

display_status_t display_init()
{
	grub_efi_status_t status;

	grub_efi_handle_t *handle_buffer = NULL;
	grub_efi_uintn_t handle_count;
	grub_efi_boot_services_t *b;
	b = grub_efi_system_table->boot_services;

	if (graph_out)
		return display_clear();

	status = efi_call_5 (b->locate_handle_buffer, GRUB_EFI_BY_PROTOCOL,
						 &graphics_output_guid, NULL, &handle_count, &handle_buffer);
	if (status != GRUB_EFI_SUCCESS)
	{
		grub_printf("Failed while initializing display: b->locate_handle_buffer with GRUB_EFI_GOP_GUID returned %ld\n", (unsigned long)status);
		return DS_ERROR;
	}

	/* First GOP will most probably be the visible one. */
	status = efi_call_3 (b->handle_protocol, handle_buffer[0],
						 &graphics_output_guid, (void **) &graph_out);
	if (status != GRUB_EFI_SUCCESS)
	{
		grub_printf("Failed while initializing display: b->handle_protocol at 0 with GRUB_EFI_GOP_GUID returned %ld\n", (unsigned long)status);
		return DS_ERROR;
	}

	status = efi_call_2 (grub_efi_system_table->con_out->reset,
						 grub_efi_system_table->con_out, 0);
	if (status != GRUB_EFI_SUCCESS)
		return DS_ERROR;

	status = efi_call_2 (grub_efi_system_table->con_out->enable_cursor,
						 grub_efi_system_table->con_out, 0);
	if (status != GRUB_EFI_SUCCESS)
		return DS_ERROR;

	return display_clear();
}

display_status_t display_clear()
{
	grub_efi_status_t status;

	status = efi_call_1 (grub_efi_system_table->con_out->clear_screen,
						 grub_efi_system_table->con_out);
	if (status != GRUB_EFI_SUCCESS)
		return DS_ERROR;

	for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
		display[i] = PX_BLANK;

	return DS_OK;
}

px_status_t display_px_set(unsigned const int x, unsigned const int y, const px_content_t value)
{
	grub_efi_status_t status;
	if (x > DISPLAY_WIDTH - 1 || y > DISPLAY_HEIGHT - 1 || !graph_out)
		return PX_ERROR;

	if (display[x + y * DISPLAY_WIDTH] == value)
		return PX_CLEAN;
	else
	{
		switch (value)
		{
			case PX_BLANK:
			status = efi_call_10 (graph_out->blt, graph_out, &PX_BLACK,
								  GRUB_EFI_BLT_VIDEO_FILL, 0, 0, 
								  x * PX_SCALE_FACTOR, y * PX_SCALE_FACTOR,
								  PX_SCALE_FACTOR, PX_SCALE_FACTOR, 0);
			if (status != GRUB_EFI_SUCCESS)
			{
				return PX_ERROR;
			}
			break;

			case PX_FILLED:
			status = efi_call_10 (graph_out->blt, graph_out, &PX_WHITE,
								  GRUB_EFI_BLT_VIDEO_FILL, 0, 0, 
								  x * PX_SCALE_FACTOR, y * PX_SCALE_FACTOR,
								  PX_SCALE_FACTOR, PX_SCALE_FACTOR, 0);
			if (status != GRUB_EFI_SUCCESS)
			{
				return PX_ERROR;
			}
			break;
		}

		display[x + y * DISPLAY_WIDTH] = value;

		return PX_DIRTY;
	}
}

px_status_t display_px_xor(unsigned const int x, unsigned const int y, const px_content_t value)
{
	grub_efi_status_t status;
	if (x > DISPLAY_WIDTH - 1 || y > DISPLAY_HEIGHT - 1 || !graph_out)
		return PX_ERROR;

	if (display[x + y * DISPLAY_WIDTH] == PX_BLANK && value == PX_BLANK)
		return PX_CLEAN;
	else if (display[x + y * DISPLAY_WIDTH] == PX_FILLED && value == PX_FILLED)
	{
		status = efi_call_10 (graph_out->blt, graph_out, &PX_BLACK,
							  GRUB_EFI_BLT_VIDEO_FILL, 0, 0, 
							  x * PX_SCALE_FACTOR, y * PX_SCALE_FACTOR,
							  PX_SCALE_FACTOR, PX_SCALE_FACTOR, 0);
		if (status != GRUB_EFI_SUCCESS)
			{
				return PX_ERROR;
			}
		display[x + y * DISPLAY_WIDTH] = PX_BLANK;
		return PX_DIRTY;
	}
	else if (display[x + y * DISPLAY_WIDTH] == PX_BLANK && value == PX_FILLED)
	{
		status = efi_call_10 (graph_out->blt, graph_out, &PX_WHITE,
								  GRUB_EFI_BLT_VIDEO_FILL, 0, 0, 
								  x * PX_SCALE_FACTOR, y * PX_SCALE_FACTOR,
								  PX_SCALE_FACTOR, PX_SCALE_FACTOR, 0);
		if (status != GRUB_EFI_SUCCESS)
			{
				return PX_ERROR;
			}
		display[x + y * DISPLAY_WIDTH] = PX_FILLED;
		return PX_CLEAN;
	}
	else if (display[x + y * DISPLAY_WIDTH] == PX_FILLED && value == PX_BLANK)
	{
		return PX_CLEAN;
	}

	return PX_ERROR;
}

px_status_t display_px_flip(unsigned const int x, unsigned const int y)
{
	grub_efi_status_t status;
	if (x > DISPLAY_WIDTH - 1 || y > DISPLAY_HEIGHT - 1 || !graph_out)
		return PX_ERROR;

	switch (display[x + y * DISPLAY_WIDTH])
	{
		case PX_BLANK:
		status = efi_call_10 (graph_out->blt, graph_out, &PX_WHITE,
								  GRUB_EFI_BLT_VIDEO_FILL, 0, 0, 
								  x * PX_SCALE_FACTOR, y * PX_SCALE_FACTOR,
								  PX_SCALE_FACTOR, PX_SCALE_FACTOR, 0);
		if (status != GRUB_EFI_SUCCESS)
		{
			return PX_ERROR;
		}
		display[x + y * DISPLAY_WIDTH] = PX_FILLED;
		return PX_DIRTY;

		case PX_FILLED:
		status = efi_call_10 (graph_out->blt, graph_out, &PX_BLACK,
							  GRUB_EFI_BLT_VIDEO_FILL, 0, 0, 
							  x * PX_SCALE_FACTOR, y * PX_SCALE_FACTOR,
							  PX_SCALE_FACTOR, PX_SCALE_FACTOR, 0);
		if (status != GRUB_EFI_SUCCESS)
		{
			return PX_ERROR;
		}
		display[x + y * DISPLAY_WIDTH] = PX_BLANK;
		return PX_DIRTY;
	}

	return PX_CLEAN;
}
