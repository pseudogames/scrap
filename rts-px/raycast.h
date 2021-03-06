#ifndef __RAYCAST_H
#define __RAYCAST_H

#include "display.h"

void raycast_init();
void raycast_context(Screen *screen, Map *map);
void raycast_display(Screen *screen, Map *map);

#endif // __RAYCAST_H
