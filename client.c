#include <SDL.h>
#include <stdio.h>
#include <time.h>
#include "sdnoise1234.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define CLAMP(a,b,c) ((a) < (b) ? (b) : (a) > (c) ? (c) : (a) )
#define DOT2(x0,y0, x1,y1) (((x0)*(x1))+((y0)*(y1)))
#define DOT3(x0,y0,z0, x1,y1,z1) (((x0)*(x1))+((y0)*(y1))+((z0)*(z1)))
#define LEN3(x0,y0,z0) sqrt(DOT3(x0,y0,z0, x0,y0,z0))

typedef struct 
{
	int x, y;
	char team;
	char selected;
	char move;
	int target_x;
	int target_y;
} Unit;

int main (int argc, char *argv[])
{
	SDL_Window *win;
	SDL_Renderer *ren;
	int screen_width = 320;
	int screen_height = 240;
	int map_width = 600; 
	int map_height = 600;
	int map_depth = 16;

	srand(time(NULL));

	SDL_Init(SDL_INIT_VIDEO);

	SDL_CreateWindowAndRenderer(screen_width, screen_height, SDL_WINDOW_OPENGL, &win, &ren);
	if(!win || !ren) {
		printf("SDL_CreateWindowAndRenderer: %s\n", SDL_GetError());
		return 1;
	}

//	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
//	SDL_RenderSetLogicalSize(ren, 640, 480);

	SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);

#if 0
	SDL_Surface *bmp = SDL_LoadBMP("hello.bmp");
	if (bmp == NULL){
		printf("SDL_LoadBMP Error: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Texture *map = SDL_CreateTextureFromSurface(ren, bmp);
	SDL_FreeSurface(bmp);
	if (map == NULL){
		printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
		return 1;
	}
#else

	SDL_Texture *map = SDL_CreateTexture(ren,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			map_width, map_height);

	Uint32 *colormap = malloc( map_width * map_height * sizeof (Uint32) );
	float *lightmap  = malloc( map_width * map_height * sizeof (float) );
	float *density   = malloc( map_width * map_height * map_depth * sizeof (float) );
	float *normalmap = malloc( map_width * map_height * map_depth * 3 * sizeof (float) );
	int *hitmap      = malloc( map_width * map_height * map_depth * sizeof (int) );

	memset(lightmap, 0, map_width * map_height * sizeof (float));

	{
		int x, y, z;
		
		for(z=0; z < map_depth; z++) {
			for(y=0; y < map_height; y++) {
				for(x=0; x < map_width; x++) {
					int xyz = x + y * map_width + z * map_width * map_height;
					int xyzk = 3 * xyz;
					
					int o, num_octaves = 4;
					float d = 0;
					float dx = 0, dy = 0, dz = 0;
					for(o=0; o < num_octaves; o++) {
						float odx, ody, odz;
						float f = 0.5/(1+o);
						d += sdnoise3(
							(float)x/((float)screen_width/(4*o+1)),
							(float)y/((float)screen_width/(4*o+1)),
							(float)z/((float)screen_width/(4*o+1)),
							&odx, &ody, &odz
						) / f;
						dx += odx / f;
						dy += ody / f;
						dz += odz / f;
					}

					normalmap[xyzk+0] = dx;
					normalmap[xyzk+1] = dy;
					normalmap[xyzk+2] = dz;

					float len = LEN3(
						normalmap[xyzk+0],
						normalmap[xyzk+1],
						normalmap[xyzk+2]
					);
					if(len) {
						normalmap[xyzk+0] /= len;
						normalmap[xyzk+1] /= len;
						normalmap[xyzk+2] /= len;
					}

					density[xyz] = MAX(0, -1 + d + 2*z/(float)map_depth); 

					if(x==10 && y==10) {
						printf("z %d d %f d %f z %f\n", z, d, density[xyz], 2*z/(float)map_depth);
						fflush(stdout);
					}
					hitmap[xyz] = -1;
				}

			}
		}

		for(y=0; y < map_height; y++) {
			for(x=0; x < map_width; x++) {
				int xy = x + y * map_width;
				lightmap[xy] = 0;
				for(z=0; z < map_depth; z++) {
					int xyz = x + y * map_width + z * map_width * map_height;
					int xyzk = 3 * xyz;
					if(density[xyz]) {
#if 0
						int lx=map_width/2, ly=map_height/2, lz=0;
						float vlx = lx-x;
						float vly = ly-y;
						float vlz = lz-z;
						float vll = LEN3(vlx, vly, vlz);
						if(vll) {
							vlx /= vll;
							vly /= vll;
							vlz /= vll;
						}


						float dot = DOT3(
							vlx,
							vly,
							vlz,
							normalmap[xyzk+0],
							normalmap[xyzk+1],
							normalmap[xyzk+2]
						);
#else
						float dot = (
							normalmap[xyzk+0] * 0.3 + 
							normalmap[xyzk+1] * 0.6 + 
							normalmap[xyzk+2] * 0.1
						);
#endif
						lightmap[xy] = 0.5 + 0.5 * dot;
						break;
					}

				}

				int c = lightmap[xy] * 0xff;
				colormap[xy] = c << 16 | c << 8 | c;
			}
		}
		
		SDL_UpdateTexture(map, NULL, colormap, map_width * sizeof (Uint32));
	}

#endif


	int i;
	int quit=0;
	SDL_Event event;
	int fps = 30;
	float map_x=0, map_y=0;
	int mouse_x = screen_width / 2;
	int mouse_y = screen_height / 2;
	#define N_UNITS 32
	Unit unit[N_UNITS];
	for(i=0; i<N_UNITS; i++) {
		unit[i].x = rand() % screen_width;
		unit[i].y = rand() % screen_height;
		unit[i].move = 0;
		unit[i].target_x = unit[i].x;
		unit[i].target_y = unit[i].y;
		unit[i].team = rand() % 2;
		unit[i].selected = 0;
	}

	int order_select_x;
	int order_select_y;
	int order_selecting = 0;
	int player_team = 1;

	int last_action_time = SDL_GetTicks();
	while (!quit)
	{
		int t = SDL_GetTicks();

		// input
		while(SDL_PollEvent(&event)) {

			switch (event.type)
			{
				case SDL_QUIT:
					quit++;
					break;

				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE: quit++; break;
						case SDLK_LEFT:   map_x-=10; break;
						case SDLK_RIGHT:  map_x+=10; break;
						case SDLK_UP:     map_y-=10; break;
						case SDLK_DOWN:   map_y+=10; break;
					}
					break;

				 case SDL_MOUSEBUTTONUP:
				 case SDL_MOUSEBUTTONDOWN:
					switch (event.button.button)
					{
						case SDL_BUTTON_LEFT:
							if(event.button.state == SDL_PRESSED) {
								order_select_x = event.button.x+map_x;
								order_select_y = event.button.y+map_y;
								order_selecting = 1;
							} else {
								int x0 = MIN(order_select_x, event.button.x+map_x);
								int x1 = MAX(order_select_x, event.button.x+map_x);
								int y0 = MIN(order_select_y, event.button.y+map_y);
								int y1 = MAX(order_select_y, event.button.y+map_y);
								int selected = 0;
								for(i=0; i<N_UNITS; i++) {
									if(player_team == unit[i].team) {
										selected += unit[i].selected = 
											unit[i].x >= x0 && unit[i].x <= x1 &&
											unit[i].y >= y0 && unit[i].y <= y1;
									}
								}
								if(!selected) {
									int closest_i = -1;
									int closest_dist = map_width * map_height;
									for(i=0; i<N_UNITS; i++) {
										if(player_team == unit[i].team) {
											int dx = unit[i].x - x1;
											int dy = unit[i].y - y1;
											int d = sqrt(dx * dx + dy * dy);
											if(d < closest_dist) {
												closest_i = i;
												closest_dist = d;
											}
										}
									}
									if(closest_i != -1 && closest_dist < 10 ) {
										unit[closest_i].selected = 1;
									}
						
								}


								order_selecting = 0;
							}
						break;

						case SDL_BUTTON_RIGHT:
							for(i=0; i<N_UNITS; i++) {
								if(unit[i].selected) {
									unit[i].move = 1;
									unit[i].target_x = event.button.x+map_x;
									unit[i].target_y = event.button.y+map_y;
								}
							}
						break;
					}
					break;

				 case SDL_MOUSEMOTION:
					 mouse_x = event.motion.x;
					 mouse_y = event.motion.y;
					break;

				 case SDL_MOUSEWHEEL:
					break;
			}
		}

		if(mouse_x < screen_width * 0.1) map_x-=10;
		if(mouse_x > screen_width * 0.9) map_x+=10;
		if(mouse_y < screen_height * 0.1) map_y-=10;
		if(mouse_y > screen_height * 0.9) map_y+=10;

		map_x = CLAMP(map_x, 0, map_width - screen_width);
		map_y = CLAMP(map_y, 0, map_height - screen_height);


		// action
		float dt = t - last_action_time;
		float speed = 0.1;
		float motion = speed * dt;
		for(i=0; i<N_UNITS; i++) {
			if(unit[i].move) {
				float dx = unit[i].target_x - unit[i].x; 
				float dy = unit[i].target_y - unit[i].y;

				float d = sqrt(dx * dx + dy * dy);
				if( (unit[i].move = d > 5) ) {
					float a = atan2(dy, dx);
					float dx = cos(a) * motion;
					float dy = sin(a) * motion;
					unit[i].x += dx;
					unit[i].y += dy;
				}
			}
		}
		last_action_time = t;


		// render
		SDL_RenderClear(ren);
		SDL_Rect src = {map_x, map_y, screen_width, screen_height};
		SDL_RenderCopy(ren, map, &src, NULL);

		for(i=0; i<N_UNITS; i++) {
			// unit
			SDL_SetRenderDrawColor(ren, 
				0xff * unit[i].team * (0.333 + 0.666 * lightmap[unit[i].x + unit[i].y * map_width]), 
				0, 
				0xff * !unit[i].team * (0.333 + 0.666 * lightmap[unit[i].x + unit[i].y * map_width]), 
				0xff
			);

			SDL_Rect rect = {unit[i].x-2-map_x, unit[i].y-2-map_y, 5,5};
			SDL_RenderFillRect(ren, &rect);

			if(unit[i].selected) {
				// unit selection
				SDL_SetRenderDrawColor(ren, 0, 0xff, 0, 0xff);
				SDL_RenderDrawRect(ren, &rect);
			}
		}

		if(order_selecting) {
			// selection rect
			SDL_SetRenderDrawColor(ren, 0, 0xff, 0, 0xff);
			SDL_Rect rect = {
				order_select_x-map_x, 
				order_select_y-map_y, 
				mouse_x - (order_select_x-map_x),
				mouse_y - (order_select_y-map_y)
			};
			SDL_RenderDrawRect(ren, &rect);
		}

		SDL_RenderPresent(ren);

		
		int elapsed = SDL_GetTicks() - t;
		int delay = 1000 / fps - elapsed;
		if(delay > 0) SDL_Delay(delay);
	}

	free(hitmap);
	free(density);
	free(normalmap);
	free(lightmap);
	free(colormap);
	SDL_DestroyTexture(map);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}
