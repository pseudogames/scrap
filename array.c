#include "array.h"
#include "stdlib.h"

inline double mind(double a, double b) { return a < b ? a : b; }
inline double maxd(double a, double b) { return a > b ? a : b; }
inline double absd(double x) { return x < 0 ? -x : x; }
inline double clampd(double x, double a, double b) { return x < a ? a : x > b ? b : x; }
inline double lerpd(double x, double a, double b) { return a + (x * (b - a)); }

void vol1u_init(Vol1u *vol, int x, int y, int z)
{
 	vol->size.x = x;
 	vol->size.y = y;
 	vol->size.z = z;
	vol->data = malloc( vol->size.x * vol->size.y * vol->size.z );
}

inline unsigned char vol1u_get(Vol1u *vol, int x, int y, int z)
{
	return vol->data[x + vol->size.x * ( y + vol->size.y * z )];
}

inline void vol1u_set(Vol1u *vol, int x, int y, int z, unsigned char d)
{
	vol->data[x + vol->size.x * ( y + vol->size.y * z )] = d;
}

void vol1u_free(Vol1u *vol)
{
	free(vol->data);
}

