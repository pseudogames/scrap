#include "array.h"
#include "stdlib.h"

inline double mind(double a, double b) { return a < b ? a : b; }
inline double maxd(double a, double b) { return a > b ? a : b; }
inline double absd(double x) { return x < 0 ? -x : x; }
inline double clampd(double x, double a, double b) { return x < a ? a : x > b ? b : x; }
inline double lerpd(double x, double a, double b) { return a + (x * (b - a)); }

void vol_init(Vol *vol, int x, int y, int z, int elem_size)
{
 	vol->size.x = x;
 	vol->size.y = y;
 	vol->size.z = z;
 	vol->elem_size = elem_size;
	vol->data = malloc( elem_size * x * y * z );
}

inline void *vol_ptr(Vol *vol, int x, int y, int z) {
	return vol->data + vol->elem_size * (x + vol->size.x * ( y + vol->size.y * z ));
}

void vol_free(Vol *vol)
{
	free(vol->data);
}

