#include "../non_core/graphics_out.h"

#include <grub/types.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/efi/api.h>
#include <grub/efi/efi.h>
#include <grub/efi/graphics_output.h>

#include <stdlib.h>
#include <string.h>

#define GB_RES_X 160
#define GB_RES_Y 144

static grub_efi_guid_t graphics_output_guid = GRUB_EFI_GOP_GUID;

static struct grub_efi_gop_blt_pixel *pixels;
static struct grub_efi_gop_blt_pixel *old_pixels; // Current screen before running the emulator
static struct grub_efi_gop *gop = NULL;
static struct grub_efi_gop_blt_pixel *output_pixels = NULL;
static unsigned int screen_width = 0;
static unsigned int screen_height = 0;

extern unsigned int scale;
extern unsigned int full_size;

static unsigned int current_scale = 1;
static unsigned int x_res = 0;
static unsigned int y_res = 0;

/* Initialize graphics
 * returns 1 if successful, 0 otherwise */
int init_screen(int win_x, int win_y, uint32_t *p) {
 
	grub_efi_uintn_t handle_count;
	grub_efi_handle_t *handle_buffer = NULL;
	
	grub_efi_boot_services_t *b;
	b = grub_efi_system_table->boot_services;

	scale = 1; 
	current_scale = scale;
	screen_width = win_x;
	screen_height = win_y;

	pixels = (struct grub_efi_gop_blt_pixel *)p;

	// Locate all instances of GOP
	grub_efi_status_t status = efi_call_5 (b->locate_handle_buffer, GRUB_EFI_BY_PROTOCOL
	, &graphics_output_guid, NULL, &handle_count, &handle_buffer);

	if (status != GRUB_EFI_SUCCESS) {
		grub_printf ("ShowStatus: Graphics output protocol not found\n");
		return 0;
	}
	
	grub_printf ("Graphics output protocols found!\n");

	for (grub_efi_uintn_t i = 0; i < handle_count; i++) {
		status = efi_call_3 (b->handle_protocol, handle_buffer[i]
			, &graphics_output_guid, (void **) &gop);
		
		if (status != GRUB_EFI_SUCCESS) {
			grub_printf ("ShowStatus: gBS->HandleProtocol[%ld] failed\n", (unsigned long)i);
			continue;
		} else {

			grub_efi_uintn_t size = 0;
			struct grub_efi_gop_mode_info *info;
			status = efi_call_4 (gop->query_mode, gop, gop->mode->mode, &size, &info);
			info = (struct grub_efi_gop_mode_info *)grub_malloc(size);
			
			if (info == NULL) {
			   continue; 
			}

			status = efi_call_4 (gop->query_mode, gop, gop->mode->mode, &size, &info);
			if (status != GRUB_EFI_SUCCESS) {
				if (info)
					grub_free(info);
				grub_printf ("Unable to get graphics mode info\n");
				continue;
			}

			x_res = info->width;
			y_res = info->height;

			old_pixels = grub_malloc(x_res * y_res * sizeof(struct grub_efi_gop_blt_pixel));
			if (old_pixels != NULL) {
		        efi_call_10 (gop->blt, gop, old_pixels, GRUB_EFI_BLT_VIDEO_TO_BLT_BUFFER,
								0, 0, 0, 0, x_res, y_res, 0); 
			}
			//grub_free(info);
			return 1;
		}
	}
	return 0; // Failed
}

void draw_screen() {
		
		if (scale != current_scale) {
			// If resolution is too big/too small
			if (scale < 1 || (GB_RES_X * scale > x_res) || GB_RES_Y * scale > y_res) {
				scale = current_scale;
			} else {
				if (old_pixels != NULL) {
		            efi_call_10 (gop->blt, gop, old_pixels, GRUB_EFI_BLT_BUFFER_TO_VIDEO,
									0, 0, 0, 0, x_res, y_res, 0); 
				}

				if (output_pixels != NULL) {
					grub_free(output_pixels);
				}
				output_pixels = grub_malloc(GB_RES_X * GB_RES_Y * scale * scale * sizeof(struct grub_efi_gop_blt_pixel));
				current_scale = scale;
			}
		}

		grub_efi_uintn_t x_offset = (x_res - (scale * GB_RES_X)) / 2;
		grub_efi_uintn_t y_offset = (y_res - (scale * GB_RES_Y)) / 2;
	  
		if (current_scale <= 1) {
		    efi_call_10 (gop->blt, gop, pixels, GRUB_EFI_BLT_BUFFER_TO_VIDEO,
							0, 0, x_offset, y_offset, GB_RES_X, GB_RES_Y, 0); 
		} else {

			uint32_t *frame_buffer = (uint32_t *)output_pixels;
			
			for (grub_efi_uintn_t y_pix = 0; y_pix < GB_RES_Y; y_pix++) {
				uint32_t *frame_buffer_row = ((uint32_t *)output_pixels) + (y_pix * current_scale * current_scale * GB_RES_X);
				for (grub_efi_uintn_t y_pix_scale = 0; y_pix_scale < current_scale; y_pix_scale++) {
					frame_buffer = frame_buffer_row + (GB_RES_X * scale * y_pix_scale);
					uint32_t *current_pix = ((uint32_t *)pixels) + (y_pix * GB_RES_X);
					for (grub_efi_uintn_t x_pix = 0; x_pix < GB_RES_X; x_pix++, frame_buffer +=current_scale) {
						for (grub_efi_uintn_t i = 0; i < current_scale; i++) {
							frame_buffer[i] = *(((int*)current_pix) + x_pix);
						}
					} 
				}
			}

			efi_call_10 (gop->blt, gop, output_pixels, GRUB_EFI_BLT_BUFFER_TO_VIDEO,
						0, 0, x_offset, y_offset, GB_RES_X * scale, GB_RES_Y * scale, 0); 
		}
}

void cleanup_graphics_out(void) {
	// Return screen buffer to state before running the emulator
	if (old_pixels != NULL) {
		efi_call_10 (gop->blt, gop, old_pixels, GRUB_EFI_BLT_BUFFER_TO_VIDEO,
						0, 0, 0, 0, x_res, y_res, 0); 
		grub_free(old_pixels);
	}
}
