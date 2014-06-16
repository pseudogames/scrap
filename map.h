#ifndef __MAP_H
#define __MAP_H

#include "transform.h"

typedef struct {
	Vec4i size; // x, y, z, components (e.g., grayscale=1, rgb=3, rgba=4)
	unsigned char *volume; // 3d volume of size with value range 0 .. 0xff 
	unsigned char *palette; // 256 RGBA colors with component range 0 .. 0xff
	int dirty; // TODO make it a region
} Map;

#endif // __MAP_H