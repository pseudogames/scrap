#include <SDL2/SDL.h>
#include <pthread.h>
#include <stdio.h>

#include "sdnoise1234.h"
#include "transform.h"
#include "screen.h"
#include "raycast.h"


void map_palette(Map *map)
{
	for(int i=0; i < 256; i++) {
		map->palette[i*4+0] = 128;	// R
		map->palette[i*4+1] = (unsigned char)clampd(3*255/(1+absd(128-i)), 128, 255);	// G
		map->palette[i*4+2] = 128;	// B
		map->palette[i*4+3] = (unsigned char)clampd((i-128)*10, 1, 255);	// A
		fprintf(stderr, "palette %d: %d %d %d %d...\n", i,
			map->palette[i*4+0],
			map->palette[i*4+1],
			map->palette[i*4+2],
			map->palette[i*4+3]
	 	); fflush(stderr);
	}
	fprintf(stderr, "map palette generated\n");
}

void *map_volume(void *data)
{
	Map *map = (Map *)data;
	
	const double scale = 0.001;
	double px=0, py=0, pz=0;
	unsigned char *p = map->volume;
	for(int z=0; z < map->size.z; z++, pz+=scale) {
		for(int y=0; y < map->size.y; y++, py+=scale) {
			for(int x=0; x < map->size.x; x++, px+=scale) {
				double limit = sdnoise2(px, py, NULL, NULL);
				double density = sdnoise3(px, py, pz, NULL, NULL, NULL);
//				fprintf(stderr, "d %lf %lf\n", density, limit); exit(1);
				*(p++) = (2.0*z/(double)map->size.z-1 > limit) ? (0xff * (density*0.5+0.5)) : 1;
			}
		}
		fprintf(stderr, "map volume layer %d/%d: %d...\n", 1+z, map->size.z, *(p-80)); fflush(stderr);
		map->dirty = 1;
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

	raycast_init();

	Map map;
 	map.size.x = screen.size.x*2;
 	map.size.y = screen.size.y*2;
 	map.size.z = 32;
 	map.size.w = 1;
	map.volume = malloc( map.size.x * map.size.y * map.size.z * map.size.w );
	map.palette = malloc( 256 * 4 );

	map_palette(&map);

	raycast_context(&screen, &map);


	pthread_t creator;
	pthread_create( &creator, NULL, map_volume, (void*)&map );
	


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
		raycast_display(&screen, &map);
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
