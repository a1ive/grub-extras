#ifndef SPRITE_PRIOS_H
#define SPRITE_PRIOS_H

#include <stdint.h>

#define MAX_SPRITES 40

typedef struct node Node;

typedef struct sprite_iterator {
    Node *next;
} Sprite_Iterator;

void init_sprite_prio_list(void);

/* Given the sprite number and its new starting x position
 * reorders the given sprite's priority */   
void update_sprite_prios(int sprite_no, uint8_t x_pos);

Sprite_Iterator create_sprite_iterator(void);
int sprite_iterator_next(Sprite_Iterator *si);
#endif //SPRITE_PRIOS_H

