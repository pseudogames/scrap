#include <SDL.h>
#define GLEW_STATIC
#include <gl/glew.h>
#include <gl/gl.h>
#include <gl/glext.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
  
    return EXIT_SUCCESS;
}


