#include <SDL2/SDL.h>
#include <pthread.h>
#include <stdio.h>

#include "sdnoise1234.h"
#include "array.h"
#include "display.h"

int running = 1;

typedef struct {
	Vol volume;
	int dirty;
} Map;

void *map_volume(void *data)
{
	Map *map = (Map *)data;
	
	const double scale = 0.001;
	double px=0, py=0, pz=0;
	unsigned char *p = map->volume.data;
	for(int z=0; z < map->volume.size.z; z++, pz+=scale) {
		if(!running) goto map_volume_end;
		for(int y=0; y < map->volume.size.y; y++, py+=scale) {
			for(int x=0; x < map->volume.size.x; x++, px+=scale) {
				double limit = sdnoise2(px, py, NULL, NULL);
				double density = sdnoise3(px, py, pz, NULL, NULL, NULL);
//				fprintf(stderr, "d %lf %lf\n", density, limit); exit(1);
				*(p++) = (2.0*z/(double)map->volume.size.z-1 > limit) ? (0xff * (density*0.5+0.5)) : 1;
			}
		}
		map->dirty = 1;
		fprintf(stderr, "map volume layer %d/%d: %d...\n", 1+z, map->volume.size.z, *(p-80)); fflush(stderr);
	}
	fprintf(stderr, "map volume generated\n");

#if 0
	for(;;) {
		fprintf(stderr, "physics scan\n");
		unsigned char *p = map->volume.data;
		for(int z=map->volume.size.z-1; z > 0 ; z--) {
			if(!running) goto map_volume_end;
			int dirty = 0;
			for(int y=0; y < map->volume.size.y; y++) {
				for(int x=0; x < map->volume.size.x; x++) {
					int i0 = P(x,y,z-1);
					int i1 = P(x,y,z);
					if(p[i0]
						dirty = 1
				}
			}
			map->dirty = dirty;
		}
	}
#endif

map_volume_end:
	return NULL;
}


int main (int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);

	Display display;
	display_init(&display, 640, 480);

	Map map;
	vol_init(&map.volume, display.size.x*2, display.size.y*2, 32, 1);
	display_volume(&display, &map.volume);

	pthread_t creator;
	pthread_create( &creator, NULL, map_volume, (void*)&map );

	while (running)
	{
		int t = SDL_GetTicks();

		// input
		SDL_Event event;
		while(SDL_PollEvent(&event)) {

			switch (event.type)
			{
				case SDL_QUIT:
					running = 0;
					break;

				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE: running = 0; break;
						case SDLK_LEFT:   display.view.angle.x -= M_PI/10; break;
						case SDLK_RIGHT:  display.view.angle.x += M_PI/10; break;
						case SDLK_UP:     display.view.angle.y -= M_PI/10; break;
						case SDLK_DOWN:   display.view.angle.y += M_PI/10; break;
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

		// if(mouse_x < display_width * 0.1) map_x-=10;
		// map_x = CLAMP(map_x, 0, map_width - display_width);


		// render
		display_render(&display);

		
		const int fps = 30;
		int elapsed = SDL_GetTicks() - t;
		int delay = 1000 / fps - elapsed;
		if(delay > 0) SDL_Delay(delay);
	}

	vol_free(&map.volume);
	display_free(&display);
	SDL_Quit();
	return 0;
}
