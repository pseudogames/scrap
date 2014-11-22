#ifndef __DISPLAY_H
#define __DISPLAY_H

#include <SDL2/SDL.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include "array.h"

typedef struct {
	SDL_GLContext ctx;
	GLuint volTexObj;
} GL;

typedef struct {
	Vec2i size;
	SDL_Window *win;
	Transform view;
	GL gl;
} Display;

void display_init(Display *display, int w, int h);
void display_viewport(Display *display);
void display_volume(Display *display, Vol *map);
void display_render(Display *display);
void display_free(Display *display);

#endif // __DISPLAY_H
