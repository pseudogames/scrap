#include <SDL.h>
#ifdef __APPLE__
# include <OpenGL/gl.h>
#else
# include <GL/gl.h>
#endif
#include <pthread.h>
#include <stdio.h>

#include "sdnoise1234.h"

typedef struct { double x,y;   } Vec2d;
typedef struct { double x,y,z; } Vec3d;
typedef struct { int     x,y;   } Vec2i;
typedef struct { int     x,y,z; } Vec3i;

typedef struct {
	double *volume;
	Vec3i size;
} Map;

typedef struct {
	Vec3d pos;
	Vec2d angle;
	Vec3d scale;
} Transform;

typedef struct {
	Vec2i size;
	SDL_Window *win;
	SDL_GLContext glctx;
	Transform view;
} Screen;

inline double mind(double a, double b) { return a < b ? a : b; }
inline double maxd(double a, double b) { return a > b ? a : b; }
inline double clampd(double x, double a, double b) { return x < a ? a : x > b ? b : x; }
inline double lerpd(double x, double a, double b) { return a + (x * (b - a)); }

void *map_create(void *data)
{
	Map *map = (Map *)data;
	
	const double scale = 0.025;
	double *p = map->volume, px=0, py=0, pz=0;
	for(int z=0; z < map->size.z; z++, pz+=scale) {
		fprintf(stderr, "map volume layer %d/%d...\n", 1+z, map->size.z); fflush(stderr);
		for(int y=0; y < map->size.y; y++, py+=scale) {
			for(int x=0; x < map->size.x; x++, px+=scale, p++) {
				*p = sdnoise3(px, py, pz, NULL, NULL, NULL);
			}
		}
	}
	fprintf(stderr, "map volume generated\n");
	return NULL;
}


int main (int argc, char *argv[])
{
	Screen screen;
 	screen.size.x = 640;
 	screen.size.y = 480;

	SDL_Init(SDL_INIT_VIDEO);

	screen.win = SDL_CreateWindow(
			"SDL2/OpenGL Demo", 
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
			screen.size.x, screen.size.y, 
			SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	if(!screen.win) {
		printf("SDL_CreateWindow: %s\n", SDL_GetError());
		return 1;
	}

	screen.glctx = SDL_GL_CreateContext(screen.win);
	glClearColor(0,0,0,1);


	Map map;
 	map.size.x = screen.size.x*2;
 	map.size.y = screen.size.y*2;
 	map.size.z = 32;
	map.volume = malloc( map.size.x * map.size.y * map.size.z * sizeof(double) );

	pthread_t creator;
	pthread_create( &creator, NULL, map_create, (void*)&map );
	


	int quit=0;
	while (!quit)
	{
		int t = SDL_GetTicks();

		// input
		SDL_Event event;
		while(SDL_PollEvent(&event)) {

			switch (event.type)
			{
				case SDL_QUIT:
					quit=1;
					break;

				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE: quit=1; break;
						case SDLK_LEFT:   screen.view.angle.x -= M_PI/10; break;
						case SDLK_RIGHT:  screen.view.angle.x += M_PI/10; break;
						case SDLK_UP:     screen.view.angle.y -= M_PI/10; break;
						case SDLK_DOWN:   screen.view.angle.y += M_PI/10; break;
					}
					break;

				 case SDL_MOUSEBUTTONUP:
				 case SDL_MOUSEBUTTONDOWN:
					switch (event.button.button)
					{
						case SDL_BUTTON_LEFT:
							// event.button.state == SDL_PRESSED) {
						break;

						case SDL_BUTTON_RIGHT:
							// unit[i].target_x = event.button.x+map_x;
						break;
					}
					break;

				 case SDL_MOUSEMOTION:
					 // mouse_x = event.motion.x;
					break;

				 case SDL_MOUSEWHEEL:
					break;
			}
		}

		// if(mouse_x < screen_width * 0.1) map_x-=10;
		// map_x = CLAMP(map_x, 0, map_width - screen_width);


		// render
		glClear(GL_COLOR_BUFFER_BIT);
			// TODO
		SDL_GL_SwapWindow(screen.win);

		
		const int fps = 30;
		int elapsed = SDL_GetTicks() - t;
		int delay = 1000 / fps - elapsed;
		if(delay > 0) SDL_Delay(delay);
	}

	free(map.volume);
	SDL_GL_DeleteContext(screen.glctx); 
	SDL_DestroyWindow(screen.win);
	SDL_Quit();
	return 0;
}
