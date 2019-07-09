#ifndef GRAPHICS_H
#define GRAPHICS_H

extern int frame_drawn; // Determines if a frame has been drawn

/* Initialize graphics
 * returns 1 if successful, 0 otherwise */
int init_gfx(void);

//Render the row number stored in the LY register
void draw_row(void);

void output_screen(void);


#endif /* GRAPHICS_H */

