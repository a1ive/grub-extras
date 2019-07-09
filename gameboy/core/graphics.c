#include "cpu.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "mmu/memory.h"
#include "memory_layout.h"
#include "graphics.h"
#include "sprite_priorities.h"
#include "bits.h"
#include "rom_info.h"

#include "../non_core/graphics_out.h"
#include "../non_core/framerate.h"
#include "../non_core/logger.h"

#ifdef PSVITA
#define VITA_PIX_X 960
#define VITA_PIX_Y 544
#endif

static int old_buffer[144][160];
static int cgb_bg_prio[144][160];

// Stores 32 bit color representation of the screen_buffer
static uint32_t rgb_pixels[144 * 160];

// Stores the processed bg palette colours
static uint32_t rendered_bg_palette[0x20];
static uint32_t rendered_sprite_palette[0x20];

static uint8_t row;
static uint8_t lcd_ctrl;
static uint8_t *bg_palette;
static uint8_t *sprite_palette;

int frame_drawn = 0;

static void refresh_gbc_bg_palettes(void);
static void refresh_gbc_sprite_palettes(void);

/*  A color in GBC is represented by 3 5 bit numbers stored within 2 bytes.*/
typedef struct {uint8_t red; uint8_t green; uint8_t blue;} GBC_color;

int init_gfx() {

    start_framerate(DEFAULT_FPS); 
    bg_palette = get_bg_palette();
    sprite_palette = get_sprite_palette();
    refresh_gbc_bg_palettes();
    refresh_gbc_sprite_palettes();

#ifdef PSVITA //VITA
	int result = init_screen(VITA_PIX_X, VITA_PIX_Y, rgb_pixels);
#else
    int result = init_screen(GB_PIXELS_X, GB_PIXELS_Y, rgb_pixels);
#endif
	log_message(LOG_INFO, "init sprite prio list\n");
	init_sprite_prio_list();    
        
    return result;
}

static uint32_t cgb_color_to_rgb(uint16_t c) {
    uint8_t red =   ((c & 0x1F) * 255) / 31;
    uint8_t green = (((c >> 5) & 0x1F) * 255) / 31;  
    uint8_t blue =  (((c >> 10) & 0x1F)* 255) / 31; 

   return 0xFF000000 | (red << 16) | (green << 8) | (blue << 0);
}

static void refresh_gbc_bg_palettes(void) {

    if (bg_palette_dirty) {
   
        for (int i = 0; i < 0x20; i++) {
            // Obtain 15 bit gameboy color for background palette
            uint16_t gb_color = bg_palette[i * 2] | ((bg_palette[(i * 2) + 1] & 0x7F) << 8);
            rendered_bg_palette[i] = cgb_color_to_rgb(gb_color);
        }
    
        bg_palette_dirty = false;
    }
}

static void refresh_gbc_sprite_palettes(void) {

    if (sprite_palette_dirty) {
   
        for (int i = 0; i < 0x20; i++) {
            // Obtain 15 bit gameboy color for sprite palette
            uint16_t gb_color = sprite_palette[i * 2] | ((sprite_palette[(i * 2) + 1] & 0x7F) << 8);
            rendered_sprite_palette[i] = cgb_color_to_rgb(gb_color);
        }
    
        sprite_palette_dirty = false;
    }
}

// Convert dot matrix gameboy's 2 bit color into a 15bit color
static uint32_t get_dmg_sprite_col(int c, int palette_no) {
    if (cgb) {
        return rendered_sprite_palette[(palette_no * 4) +  c];    
    }
    switch (c) {
        case 0: return cgb_color_to_rgb(0x7FFF);
        case 1: return cgb_color_to_rgb(0x56B5);
        case 2: return cgb_color_to_rgb(0x294A);
        case 3: return cgb_color_to_rgb(0x0000);
        default : return cgb_color_to_rgb(0x0);
    }
}

static uint32_t get_dmg_bg_col(int c) {
    if (cgb) {
        return rendered_bg_palette[c];
    }
    switch (c) {
        case 0: return cgb_color_to_rgb(0x7FFF);
        case 1: return cgb_color_to_rgb(0x56B5);
        case 2: return cgb_color_to_rgb(0x294A);
        case 3: return cgb_color_to_rgb(0x0000);
        default : return 0x0;
    }
}



static void draw_sprite_row(void) {
   
    refresh_gbc_sprite_palettes();

    // 8x16 or 8x8
    int height = lcd_ctrl & BIT_2 ? 16 : 8;

    uint8_t obp_0 = io_mem[OBP0_REG]; 
    uint8_t obp_1 = io_mem[OBP1_REG]; 
    int palletes[2][4];

    //Calculate both color palletes
    palletes[0][0] =  obp_0  & 0x3;
    palletes[0][1] = (obp_0 >> 2) & 0x3;
    palletes[0][2] = (obp_0 >> 4) & 0x3;
    palletes[0][3] = (obp_0 >> 6) & 0x3;

    palletes[1][0] = obp_1  & 0x3;
    palletes[1][1] = (obp_1 >> 2) & 0x3;
    palletes[1][2] = (obp_1 >> 4) & 0x3;
    palletes[1][3] = (obp_1 >> 6) & 0x3;

    Sprite_Iterator si = create_sprite_iterator();
    int sprite_no;
    int sprite_count = 0;
    int sprite_nos[10];

    /*40 Sprites, loop through from least priority to most priority
      limited to 10 a line */
    while((sprite_no = sprite_iterator_next(&si)) != -1 && sprite_count < 10)  {
        
        int16_t y_pos = oam_get_mem((sprite_no * 4)) - 16;
        int16_t x_pos = oam_get_mem((sprite_no * 4) + 1) - 8;
        
        //If sprite doesn't intersect current line, no need to draw
        if (y_pos > row || row >= y_pos + height || x_pos >= 160) {
            continue;
        }
        
        sprite_nos[sprite_count] = sprite_no; 
        sprite_count++;
    }

    for (int i = sprite_count - 1; i >= 0; i--) {
         sprite_no  = sprite_nos[i];

         int16_t y_pos = oam_get_mem((sprite_no * 4)) - 16;
         int16_t x_pos = oam_get_mem((sprite_no * 4) + 1) - 8;
         uint8_t tile_no = oam_get_mem((sprite_no * 4) + 2);
         uint8_t attributes = oam_get_mem((sprite_no * 4) + 3);
    
        
         if (height == 16) {
            tile_no &= ~0x1;
         }                           
    
        
        int x_flip = attributes & BIT_5;
        int y_flip = attributes & BIT_6;
        
        int v_bank = 0;
        int cgb_palette_number = 0;
        if (cgb) {
           
            if (is_booting || cgb_features) {
                cgb_palette_number = attributes & 0x7;
                v_bank = !!(attributes & BIT_3);
            }            
            // DMG mode in CGB, there is only 2 predefined BG palettes which the
            // boot rom initialized at startup containing 3 colors each
            // and only the original vram bank is available
            else {
                cgb_palette_number = !!(attributes & BIT_4);
                v_bank = 0;
            }
        }

        uint16_t tile_loc = TILE_SET_0_START + (tile_no * 16);
        
        // Obtain row of sprite to draw, if sprite is flipped
        // need to obtain row relative to bottom of sprite
        uint8_t line =  (!y_flip) ? row - y_pos  : height + y_pos - row -1;
        uint16_t line_offset = 2 * line;
        uint8_t high_byte = get_vram(tile_loc + line_offset, v_bank);
        uint8_t low_byte =  get_vram(tile_loc + line_offset + 1, v_bank);

        int pal_no = (attributes & BIT_4) ? 1 : 0;
        
        // Draw all pixels in current line of sprite
        for (int x = 0; x < 8; x++) {
            // Not on screen
            if (x_pos + x >= 160 || x_pos + x < 0) {
                continue;
            }
            int bit_pos = (x_flip) ? x : 7 - x;
            int bit_0 = (low_byte >> bit_pos) & 0x1;
            int bit_1 = (high_byte >> bit_pos) & 0x1;
            uint8_t color_id = (bit_0 << 1) | bit_1;
            int sprite_prio  = !(attributes & 0x80);
            
            // If priority bit not set but background is transparent and
            // current pixel isn't transparent draw. Otherwise if priority set
            // as long as pixel isn't transparent, draw it
            uint8_t final_color_id = palletes[pal_no][color_id]; 
            if (!sprite_prio) {
                if (color_id != 0 && (!cgb_bg_prio[row][x_pos + x] && !old_buffer[row][x_pos + x])) {
                    if (!cgb || !(is_booting || cgb_features)) {
                       rgb_pixels[(row * GB_PIXELS_X) + x_pos + x] = rendered_sprite_palette[(pal_no * 4) + final_color_id];
                       old_buffer[row][x_pos + x] = color_id;
                   } else {                        
                       rgb_pixels[(row * GB_PIXELS_X) + x_pos + x] = rendered_sprite_palette[(cgb_palette_number * 4) + color_id];
                       old_buffer[row][x_pos + x] = color_id;
                   }
                }               
            } else  {
                if (color_id != 0 && (!cgb || (!cgb_bg_prio[row][x_pos + x] || !old_buffer[row][x_pos + x]))) {
                    if (!cgb || !(is_booting || cgb_features)) {
                       rgb_pixels[(row * GB_PIXELS_X) + x_pos + x] = get_dmg_sprite_col(final_color_id, pal_no);
                       old_buffer[row][x_pos + x] = color_id;
                   } else {
                       rgb_pixels[(row * GB_PIXELS_X) + x_pos + x] = rendered_sprite_palette[(cgb_palette_number * 4) + color_id];
                       old_buffer[row][x_pos + x] = color_id;
                   }
                }
            } 
             
        }
    }
}




static void draw_tile_window_row(uint16_t tile_mem, uint16_t bg_mem) {
   
    uint8_t bgp = io_mem[BGP_REF];
    int pallete[4];
    //Calculate color pallete
    pallete[0] =  bgp  & 0x3;
    pallete[1] = (bgp >> 2) & 0x3;
    pallete[2] = (bgp >> 4) & 0x3;
    pallete[3] = (bgp >> 6) & 0x3;
    
    uint8_t win_y = io_mem[WY_REG];//window_line;
    int16_t y_pos = row - win_y; // Get line 0 - 255 being drawn    
    uint16_t tile_row = (y_pos >> 3); // Get row 0 - 31 of tile
    
    /* WX_REG values < 7 are treated as WX_REG = 7, fixes clipping
     * of the podracer in star wars episode 1 - racer */
    int16_t win_x = io_mem[WX_REG] < 7 ? 0 : io_mem[WX_REG] - 7;
   
    if (win_x > 159 || io_mem[WY_REG] > 143 || row < win_y) {
        return;
    }
    
   // int skew_left = win_x % 8;
   // int skew_right = (8 - skew_left) % 8;
    int skew = (8 - (win_x % 8)) % 8;
    
    // 160 pixel row, 20 tiles, 8 pixel row per tile
    for (int i = 0 - skew; i < 160 + skew; i+=8) {
        int pixel_x_start = 0;
        int start_x = i;
        //Part of tile offscreen
        if (start_x < win_x) { 
            if (start_x < (win_x - 7)) { // None of tile onscreen
                continue;
             } else {
                 pixel_x_start = win_x - start_x;
                 start_x = win_x;
             }
        }
     
        int x_pos = start_x - win_x;
        int tile_col = (x_pos) >> 3;
        int tile_no = get_vram0(bg_mem + (tile_row << 5)  + tile_col);
        
        int tile_attributes = 0;
        int palette_no = 0;
        int tile_vram_bank_no = 0;        
        int bg_prio = 0;

        if (cgb) {
            tile_attributes = get_vram(bg_mem + (tile_row << 5) + tile_col, 1);

            if (is_booting || cgb_features) {
                palette_no = tile_attributes & 0x7;
                tile_vram_bank_no = !!(tile_attributes & BIT_3);                
                 bg_prio = tile_attributes & BIT_7;
                
            /* DMG mode in CGB, there is only 1 predefined BG palette which the
             * boot rom initialized at startup containing 4 colors 
             * and only the original vram bank is available */
            } else {
                palette_no = 0;
                tile_vram_bank_no = 0; 
            }
        }

        // Signed tile no, need to convert to offset
        if (tile_mem == TILE_SET_1_START) {
            tile_no = (tile_no & 127) - (tile_no & 128) + 128;
        }
       
        int tile_loc = tile_mem + (tile_no * 16); //Location of tile in memory
        int line_offset = (y_pos % 8) * 2; //Offset into tile of our line
        

        int byte0 = get_vram(tile_loc + line_offset, tile_vram_bank_no);
        int byte1 = get_vram(tile_loc + line_offset + 1, tile_vram_bank_no);
       
         
        // If Horizontal flip flag set in CGB mode
        int horiz_flip = tile_attributes & BIT_5;

        // For each pixel in the line of the tile
        for (int j = pixel_x_start; j < 8; j++) {

            if ((start_x + j) >= 0 && (start_x + j) < 160) {
                int bit_1 = (byte1 >> (horiz_flip ? j : (7 - j))) & 0x1;
                int bit_0 = (byte0 >> (horiz_flip ? j : (7 - j))) & 0x1;
                int color_id = (bit_1 << 1) | bit_0;

                if (!cgb || !(is_booting || cgb_features)) {
                    rgb_pixels[(row * GB_PIXELS_X) + (i + j)] = get_dmg_bg_col(pallete[color_id]); 
                    old_buffer[row][i + j] = color_id;
                } else {
                    rgb_pixels[(row * GB_PIXELS_X) + (i + j)] = rendered_bg_palette[(palette_no * 4) + color_id];
                    old_buffer[row][i + j] = color_id;
                    cgb_bg_prio[row][i + j] = bg_prio ? 1 : 0; 
                }
            }
        }   
    }      

}

//Render the supplied row with background tiles
static void draw_tile_bg_row(uint16_t tile_mem, uint16_t bg_mem) {
    uint8_t bgp = io_mem[BGP_REF];
    int pallete[4];
    //Calculate color pallete
    pallete[0] =  bgp  & 0x3;
    pallete[1] = (bgp >> 2) & 0x3;
    pallete[2] = (bgp >> 4) & 0x3;
    pallete[3] = (bgp >> 6) & 0x3;
    
    uint8_t y_pos = row + io_mem[SCROLL_Y_REG];  
    int tile_row = y_pos >> 3; // Get row 0 - 31 of tile
    uint8_t scroll_x = io_mem[SCROLL_X_REG];
   
    int skew_left = scroll_x & 0x7;
    int skew_right = (8 - skew_left) & 0x7;

    //uint8_t tile_nos[160] = 
    //memcpy(tile_nos, 
    for (int i = 0 - skew_left; i < 160 + skew_right; i+= 8) {

        uint8_t x_pos = i + scroll_x;
        int tile_col = x_pos >> 3;
        int tile_no = get_vram0(bg_mem + (tile_row << 5) + tile_col);

        int tile_attributes = 0;
        int palette_no = 0;
        int tile_vram_bank_no = 0;
        int bg_prio = 0;

        if (cgb) {
            tile_attributes = get_vram1(bg_mem + (tile_row << 5) + tile_col);
            
            if (is_booting || cgb_features) {
                palette_no = tile_attributes & 0x7;
                tile_vram_bank_no = !!(tile_attributes & BIT_3);           
                bg_prio = tile_attributes & BIT_7;
            
            // DMG mode in CGB, there is only 1 predefined BG palette which the
            // boot rom initialized at startup containing 4 colors 
            // and only the original vram bank is available
            } else {
                palette_no = 0;
                tile_vram_bank_no = 0; 
            }
        }

        // Signed tile no, need to convert to offset
        if (tile_mem == TILE_SET_1_START) {
            tile_no = (tile_no & 127) - (tile_no & 128) + 128;
        }
        
        // If Verical flip flag set in CGB mode
        int vert_flip = tile_attributes & BIT_6;         

        int tile_loc = tile_mem + (tile_no * 16); //Location of tile in memory
        int line_offset = (vert_flip ? (7 - (y_pos & 0x7)) : (y_pos & 0x7)) << 1; //Offset into tile of our line
            
        int byte0 = get_vram(tile_loc + line_offset, tile_vram_bank_no);
        int byte1 = get_vram(tile_loc + line_offset + 1, tile_vram_bank_no);


        // If Horizontal flip flag set in CGB mode
        int horiz_flip = tile_attributes & BIT_5;
        
        //Render entire tile row
        for (int j = 0; j < 8; j++) {

            if (i + j >= 0 && i + j < 160) {

                int bit_1 = (byte1 >> (horiz_flip ? j : (7 - j))) & 0x1;
                int bit_0 = (byte0 >> (horiz_flip ? j : (7 - j))) & 0x1;
                int color_id = (bit_1 << 1) | bit_0;

                if (!cgb || !(is_booting || cgb_features)) {
                    rgb_pixels[(GB_PIXELS_X * row) + (i + j)] = get_dmg_bg_col(pallete[color_id]); 
                    old_buffer[row][i + j] = color_id;
                } else {
                    rgb_pixels[(row * GB_PIXELS_X) + (i + j)] = rendered_bg_palette[(palette_no * 4) + color_id];
                    old_buffer[row][i + j] = color_id;
                    cgb_bg_prio[row][i + j] = bg_prio ? 1 : 0;
                   
               }

            }
         }   
            
    }
}    

static void draw_tile_row(void) {
 
    uint8_t win_y_pos = io_mem[WY_REG];

    uint16_t tile_mem; // Either tile set 0 or 1

    // Check if using Tile set 0 or 1 
    tile_mem = lcd_ctrl & BIT_4 ? TILE_SET_0_START : TILE_SET_1_START;
     
    refresh_gbc_bg_palettes();
    
    //Draw background    
    uint16_t bg_mem = lcd_ctrl & BIT_3 ? BG_MAP_DATA1_START : BG_MAP_DATA0_START;
    draw_tile_bg_row(tile_mem, bg_mem);

    //Draw Window display if it's on
    if ((lcd_ctrl & BIT_5) && (win_y_pos <= row)) {
        uint16_t win_bg_mem = lcd_ctrl & BIT_6 ? BG_MAP_DATA1_START :BG_MAP_DATA0_START;
        draw_tile_window_row(tile_mem, win_bg_mem);
    }    
}


void output_screen(void) {
    draw_screen();
    adjust_to_framerate();
}

//Render the row number stored in the LY register
void draw_row(void) {

    lcd_ctrl = io_mem[LCDC_REG];
    row = io_mem[LY_REG];

    //Render only if screen is on
    if ((lcd_ctrl & BIT_7)) {
        uint8_t render_sprites = (lcd_ctrl & BIT_1);
        uint8_t render_tiles = (lcd_ctrl  & BIT_0);

        if ((cgb && (cgb_features || is_booting)) || render_tiles) {
            draw_tile_row();
        
        }
        
        if (render_sprites) { 
            draw_sprite_row();
        }
   } 

   if (row >= 143) {
        output_screen();
        frame_drawn = 1;
   }  
}

