#include <SDL.h>
#include <stdio.h>
#include "sdnoise1234.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define CLAMP(a,b,c) ((a) < (b) ? (b) : (a) > (c) ? (c) : (a) )

typedef struct 
{
	int x;
	int y;
} Coordinate;

typedef struct 
{
	int x, y;
	char team;
	char selected;
	char move;
	int target_x;
	int target_y;
} Unit;


void draw_map(SDL_Texture *map, float *lightmap, float *heightmap, int map_width, int map_height);
void init_units(Unit *units, int screen_width, int screen_height);
void action(Unit *units, int t, int last_action_time);
void delay(int t);

int main (int argc, char *argv[])
{
	SDL_Window *win;
	SDL_Renderer *ren;
	int screen_width = 640;
	int screen_height = 480;
	int map_width = 1024; 
	int map_height = 768;

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

	float *lightmap = malloc( map_width * map_height * sizeof (float) );
	float *heightmap = malloc( map_width * map_height * sizeof (float) );

	draw_map(map, lightmap, heightmap, map_width, map_height);


#endif


	int quit=0;
	int i;
	SDL_Event event;
	int mouse_x = screen_width / 2;
	int mouse_y = screen_height / 2;
	#define N_UNITS 32
	Unit *units = malloc(N_UNITS * sizeof (Unit));
	Coordinate map_position;
	map_position.x = map_position.y = 0;
	
	init_units(units, screen_width, screen_height);

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
						case SDLK_LEFT:   map_position.x-=10; break;
						case SDLK_RIGHT:  map_position.x+=10; break;
						case SDLK_UP:     map_position.y-=10; break;
						case SDLK_DOWN:   map_position.y+=10; break;
					}
					break;

				 case SDL_MOUSEBUTTONUP:
				 case SDL_MOUSEBUTTONDOWN:
					switch (event.button.button)
					{
						case SDL_BUTTON_LEFT:
							if(event.button.state == SDL_PRESSED) {
								order_select_x = event.button.x+map_position.x;
								order_select_y = event.button.y+map_position.y;
								order_selecting = 1;
							} else {
								int x0 = MIN(order_select_x, event.button.x+map_position.x);
								int x1 = MAX(order_select_x, event.button.x+map_position.x);
								int y0 = MIN(order_select_y, event.button.y+map_position.y);
								int y1 = MAX(order_select_y, event.button.y+map_position.y);
								int selected = 0;
								for(i=0; i<N_UNITS; i++) {
									if(player_team == units[i].team) {
										selected += units[i].selected = 
											units[i].x >= x0 && units[i].x <= x1 &&
											units[i].y >= y0 && units[i].y <= y1;
									}
								}
								if(!selected) {
									int closest_i = -1;
									int closest_dist = map_width * map_height;
									for(i=0; i<N_UNITS; i++) {
										if(player_team == units[i].team) {
											int dx = units[i].x - x1;
											int dy = units[i].y - y1;
											int d = sqrt(dx * dx + dy * dy);
											if(d < closest_dist) {
												closest_i = i;
												closest_dist = d;
											}
										}
									}
									if(closest_i != -1 && closest_dist < 10 ) {
										units[closest_i].selected = 1;
									}
						
								}


								order_selecting = 0;
							}
						break;

						case SDL_BUTTON_RIGHT:
							for(i=0; i<N_UNITS; i++) {
								if(units[i].selected) {
									units[i].move = 1;
									units[i].target_x = event.button.x+map_position.x;
									units[i].target_y = event.button.y+map_position.y;
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

		if(mouse_x < screen_width * 0.1) map_position.x-=10;
		if(mouse_x > screen_width * 0.9) map_position.x+=10;
		if(mouse_y < screen_height * 0.1) map_position.y-=10;
		if(mouse_y > screen_height * 0.9) map_position.y+=10;

		map_position.x = CLAMP(map_position.x, 0, map_width - screen_width);
		map_position.y = CLAMP(map_position.y, 0, map_height - screen_height);


		// action
		action(units, t, last_action_time);
		last_action_time = t;


		// render
		SDL_RenderClear(ren);
		SDL_Rect src = {map_position.x, map_position.y, screen_width, screen_height};
		SDL_RenderCopy(ren, map, &src, NULL);

		for(i=0; i<N_UNITS; i++) {
			// unit
			SDL_SetRenderDrawColor(ren, 
				0xff * units[i].team * (0.333 + 0.666 * lightmap[units[i].x + units[i].y * map_width]), 
				0, 
				0xff * !units[i].team * (0.333 + 0.666 * lightmap[units[i].x + units[i].y * map_width]), 
				0xff
			);

			SDL_Rect rect = {units[i].x-2-map_position.x, units[i].y-2-map_position.y, 5,5};
			SDL_RenderFillRect(ren, &rect);

			if(units[i].selected) {
				// unit selection
				SDL_SetRenderDrawColor(ren, 0, 0xff, 0, 0xff);
				SDL_RenderDrawRect(ren, &rect);
			}
		}

		if(order_selecting) {
			// selection rect
			SDL_SetRenderDrawColor(ren, 0, 0xff, 0, 0xff);
			SDL_Rect rect = {
				order_select_x-map_position.x, 
				order_select_y-map_position.y, 
				mouse_x - (order_select_x-map_position.x),
				mouse_y - (order_select_y-map_position.y)
			};
			SDL_RenderDrawRect(ren, &rect);
		}

		SDL_RenderPresent(ren);

    delay(t);
	}

	free(heightmap);
	free(lightmap);
	SDL_DestroyTexture(map);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}

void draw_map(SDL_Texture *map, float *lightmap, float *heightmap, int map_width, int map_height) {
	Uint32 *colormap = malloc( map_width * map_height * sizeof (Uint32) );
	int i=0;
	int x, y;
	float l0=999999999, l1=0;
	float h0=999999999, h1=0;
	for(y=0; y < map_height; y++) {
		for(x=0; x < map_width; x++, i++) {
			int o, n_oct = 8;
			float h=0, dx=0,dy=0;
			for(o=0; o<n_oct; o++) {
				float _dx, _dy;
				float f = (0.5 / (1+o));
				h += sdnoise2((float)x/(100.0/(1+o)), (float)y/(100.0/(1+o)), &_dx, &_dy) * f;
				dx += _dx * f;
				dy += _dy * f;
			}
			float l = dx + dy;
			l0 = MIN(l, l0);
			l1 = MAX(l, l1);
			h0 = MIN(h, h0);
			h1 = MAX(h, h1);

			lightmap[i] = l;
			heightmap[i] = h;
		}

	}

	i=0;
	for(y=0; y < map_height; y++) {
		for(x=0; x < map_width; x++, i++) {
			lightmap[i]  = (lightmap[i]  - l0) / (l1 - l0);
			heightmap[i] = (heightmap[i] - h0) / (h1 - h0);
			int c = lightmap[i] * 0xff;
			colormap[i] = c << 16 | c << 8 | c;
		}
	}
	
	SDL_UpdateTexture(map, NULL, colormap, map_width * sizeof (Uint32));
}

void init_units(Unit *units, int screen_width, int screen_height) {
	int i;
	for(i=0; i<N_UNITS; i++) {
		units[i].x = rand() % screen_width;
		units[i].y = rand() % screen_height;
		units[i].move = 0;
		units[i].target_x = units[i].x;
		units[i].target_y = units[i].y;
		units[i].team = rand() % 2;
		units[i].selected = 0;
	}
}

void action(Unit *units, int t, int last_action_time) {
	int i;
	float dt = t - last_action_time;
	float speed = 0.1;
	float motion = speed * dt;
	for(i=0; i<N_UNITS; i++) {
		if(units[i].move) {
			float dx = units[i].target_x - units[i].x; 
			float dy = units[i].target_y - units[i].y;

			float d = sqrt(dx * dx + dy * dy);
			if( (units[i].move = d > 5) ) {
				float a = atan2(dy, dx);
				float dx = cos(a) * motion;
				float dy = sin(a) * motion;
				units[i].x += dx;
				units[i].y += dy;
			}
		}
	}
}

void delay(int t) {
	int fps = 30;
	int elapsed = SDL_GetTicks() - t;
	int delay = 1000 / fps - elapsed;
	if(delay > 0) SDL_Delay(delay);
}