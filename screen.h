#ifndef __SCREEN_H
#define __SCREEN_H

#include "SDL.h"
#include "transform.h"

typedef struct {
	Vec2i size;
	SDL_Window *win;
	SDL_GLContext glctx;
	Transform view;
} Screen;

#endif // __SCREEN_H