#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "noise.h"

#define MAP_X 16
#define MAP_Y 16
#define MAP_Z 4


// 1st try
// +-grid--o-------o-------+
// |0,0    |1,0    |2,0    |
// |   +-bind--+-------+   |
// |   |0,0|   |1,0|   |   |
// o-------O-------o-------+
// |0,1|   |1,1|   |2,1|   |
// |   +-------+-------+   |
// |   |0,1|   |1,1|   |   |
// o-------o-------o-------+
// |0,2|   |1,2|   |2,2|   |
// |   +-------+-------+   |
// |       |       |       |
// +-------+-------+-------+

// 2nd try
//      0,0     1,0     2,0
// +---x---o---x---o---x---+
// |0,0  \ |1,0  \ |2,0  \ |
// x0,1    x1,1    x2,1    x3,1
// | \   / | \   / | \   / |
// o---x---O---x---o---x---+
// |0,1  \ |1,1  \ |2,1  \ |
// x       x       x       x
// | \   / | \   / | \   / |
// o---x---o---x---o---x---+
// |0,2  \ |1,2  \ |2,2  \ |
// x       |       x       x
// | \   / | \   / | \   / |
// +---x---+---x---+---x---+

// 3rd try
// +->-----+->-----+->-----+
// |0,0    |1,0    v2,0    v
// |       |       |       |
// |       |       |       |
// +->-----+->-----+-------+
// |0,1    |1,1    v2,1    v
// |       |       |       |
// |       |       |       |
// x-------x-------+-------+
// v0,2    v1,2    v2,2    v
// |       |       |       |
// |       |       |       |
// x->-----x->-----+->-----+


typedef struct {
	int x,y,z; // z+ is up
} Point;

typedef struct {
	Point parent;
	double distance;
	double strain;
} Connection;

typedef struct {
	Connection base;
	double weight;
	double resistance;
	double pressure;
	int color;
} Cell;

typedef struct {
	Cell *grid;
} Map;

typedef struct {
	Map map;
} Game;

Cell *cell(Game *game, int x, int y, int z) {
	int i = x + MAP_X * (y + MAP_Y * z);
	return game->map.grid + i;
}

int main(int argc, char *argv[])
{
	Game game;
	game.map.grid = malloc( MAP_X * MAP_Y * MAP_Z * sizeof(Cell) );

	fprintf(stderr,"\e[3J\e[H\e[2J");

	double M = 0;
	double m = 999999999;
	for(int z = 0; z < MAP_Z; z++) {
		for(int y = 0; y < MAP_Y; y++) {
			for(int x = 0; x < MAP_X; x++) {
				double v = noise_octave(x,y,z,1);
				if(v < m) m = v;
				if(v > M) M = v;
				cell(&game, x,y,z)->resistance = v;
			}
		}
	}

	for(int z = 0; z < MAP_Z; z++) {
		for(int y = 0; y < MAP_Y; y++) {
			for(int x = 0; x < MAP_X; x++) {
				double *r = &(cell(&game, x,y,z)->resistance);
				double d = pow(z / (double)MAP_Z, 6);
				*r = ((*r - m) / (M - m)) * (1-d) + (d);
			}
		}
	}

	for(int z = 0; z < MAP_Z; z++) {
		for(int y = 0; y < MAP_Y; y++) {
			for(int x = 0; x < MAP_X; x++) {
				fprintf(stderr, "%7.3f", cell(&game, x,y,z)->resistance);
			}
			fprintf(stderr, "\n");
		}
		fprintf(stderr, "------\n");
	}

	fflush(stderr);
	return 0;
}
