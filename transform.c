#include "transform.h"

inline double mind(double a, double b) { return a < b ? a : b; }
inline double maxd(double a, double b) { return a > b ? a : b; }
inline double absd(double x) { return x < 0 ? -x : x; }
inline double clampd(double x, double a, double b) { return x < a ? a : x > b ? b : x; }
inline double lerpd(double x, double a, double b) { return a + (x * (b - a)); }
