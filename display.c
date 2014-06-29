#include <stdio.h>
#include "display.h"
#include "log.h"

void display_viewport(Display *display)
{
    glViewport(0, 0, display->size.x, display->size.y);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    GLdouble aspect = (GLdouble)display->size.x / display->size.y;
    glOrtho(-3.0, 3.0, -3.0 / aspect, 3.0 / aspect, 0.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
}

void display_init(Display *display, int w, int h)
{
 	display->size.x = w;
 	display->size.y = h;

	display->win = SDL_CreateWindow(
			"SDL2/OpenGL Demo", 
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
			display->size.x, display->size.y, 
			SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	if(!display->win) {
		log_fatal(LOG, 1, "SDL_CreateWindow: %s\n", SDL_GetError());
	}

	display->gl.ctx = SDL_GL_CreateContext(display->win);
	
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		log_fatal(LOG, 2, "Error: %d, %s\n", err, glewGetErrorString(err));
	}

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glShadeModel(GL_SMOOTH);

	display_viewport(display);
}

void display_volume(Display *display, Vol1u *vol)
{
	glGenTextures(1, &display->gl.volTexObj);

	// bind 3D texture target
	glBindTexture(GL_TEXTURE_3D, display->gl.volTexObj);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	// pixel transfer happens here from client to OpenGL server
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY, 
		vol->size.x, 
		vol->size.y, 
		vol->size.z, 
		0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 
		(GLubyte *)vol->data);

}

void display_render(Display *display)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

	SDL_GL_SwapWindow(display->win);
}

void display_free(Display *display)
{
	SDL_GL_DeleteContext(display->gl.ctx); 
	SDL_DestroyWindow(display->win);
}