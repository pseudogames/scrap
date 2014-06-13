#ifndef __TRANSFORM_H
#define __TRANSFORM_H

typedef struct { double x,y;     } Vec2d;
typedef struct { double x,y,z;   } Vec3d;
typedef struct { int     x,y;     } Vec2i;
typedef struct { int     x,y,z;   } Vec3i;
typedef struct { int     x,y,z,w; } Vec4i;

typedef struct {
	Vec3d pos;
	Vec2d angle;
	Vec3d scale;
} Transform;

double mind(double a, double b);
double maxd(double a, double b);
double absd(double x);
double clampd(double x, double a, double b);
double lerpd(double x, double a, double b);

#endif // __TRANSFORM_H
