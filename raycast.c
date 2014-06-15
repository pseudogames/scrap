#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <stdio.h>
#include "raycast.h"
#include "warn.h"
#include "tff.h"

//#define HEAD
#ifdef HEAD
#include "head.h"
#endif


GLuint g_vao;
GLuint g_programHandle;
GLint g_angle = 0;
GLuint g_frameBuffer;
// transfer function
GLuint g_tffTexObj;
GLuint g_bfTexObj;
GLuint g_texWidth;
GLuint g_texHeight;
GLuint g_volTexObj;
GLuint g_rcVertHandle;
GLuint g_rcFragHandle;
GLuint g_bfVertHandle;
GLuint g_bfFragHandle;
float g_stepSize = 0.001f;

#define GL_ERROR() checkForOpenGLError(__FILE__, __LINE__)
void  checkForOpenGLError(const char* file, int line)
{
	GLenum glErr = glGetError();
	if(glErr != GL_NO_ERROR) {
		log_fatal(LOG, EXIT_FAILURE, "glError in file %s @line %d: %s\n", file, line, gluErrorString(glErr));
	}
}

void initVBO();
void initShader();
void initFrameBuffer(GLuint, GLuint, GLuint);
GLuint initTFF1DTex(Map *map);
GLuint initFace2DTex(GLuint texWidth, GLuint texHeight);
GLuint initVol3DTex(Map *map);
void render(Screen *screen, GLenum cullFace);

void raycast_init()
{
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		log_fatal(LOG, EXIT_FAILURE, "Error: %d, %s\n", err, glewGetErrorString(err));
	}
}

void raycast_context(Screen *screen, Map *map)
{
	g_texWidth = screen->size.x;
	g_texHeight = screen->size.y;
	initVBO();
	initShader();
	g_tffTexObj = initTFF1DTex(map);
	g_bfTexObj = initFace2DTex(g_texWidth, g_texHeight);
	g_volTexObj = initVol3DTex(map);
	GL_ERROR();
	initFrameBuffer(g_bfTexObj, g_texWidth, g_texHeight);
	GL_ERROR();

	glClearColor(0,0,0,1);

}
// init the vertex buffer object
void initVBO()
{
	GLfloat vertices[24] = {
		0.0, 0.0, 0.0,
		0.0, 0.0, 1.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 1.0,
		1.0, 0.0, 0.0,
		1.0, 0.0, 1.0,
		1.0, 1.0, 0.0,
		1.0, 1.0, 1.0
	};
	// draw the six faces of the boundbox by drawwing triangles
	// draw it contra-clockwise
	// front: 1 5 7 3
	// back: 0 2 6 4
	// left��0 1 3 2
	// right:7 5 4 6    
	// up: 2 3 7 6
	// down: 1 0 4 5
	GLuint indices[36] = {
		1,5,7,
		7,3,1,
		0,2,6,
		6,4,0,
		0,1,3,
		3,2,0,
		7,5,4,
		4,6,7,
		2,3,7,
		7,6,2,
		1,0,4,
		4,5,1
	};
	GLuint gbo[2];

	glGenBuffers(2, gbo);
	GLuint vertexdat = gbo[0];
	GLuint veridxdat = gbo[1];
	glBindBuffer(GL_ARRAY_BUFFER, vertexdat);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	// used in glDrawElement()
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veridxdat);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36*sizeof(GLuint), indices, GL_STATIC_DRAW);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	// vao like a closure binding 3 buffer object: verlocdat vercoldat and veridxdat
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0); // for vertexloc
	glEnableVertexAttribArray(1); // for vertexcol

	// the vertex location is the same as the vertex color
	glBindBuffer(GL_ARRAY_BUFFER, vertexdat);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veridxdat);
	// glBindVertexArray(0);
	g_vao = vao;
}
void drawBox(GLenum glFaces)
{
	glEnable(GL_CULL_FACE);
	glCullFace(glFaces);
	glBindVertexArray(g_vao);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLuint *)NULL);
	glDisable(GL_CULL_FACE);
}
// check the compilation result
GLboolean compileCheck(GLuint shader)
{
	GLint err;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &err);
	if (GL_FALSE == err)
	{
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0)
		{
			char* log = (char *)malloc(logLen);
			GLsizei written;
			glGetShaderInfoLog(shader, logLen, &written, log);
			log_info(LOG, "Shader log: %s\n", log);
			free(log);		
		}
	}
	return err;
}

#define fileReadStringMalloc(filename) _fileReadStringMalloc(__FILE__, __LINE__, filename)
char *_fileReadStringMalloc(/*debug*/ char *file, int line, const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) {
		log_fatal(file, line, EXIT_FAILURE, "Error openning file: %s\n", filename);
	}
	
	fseek(fp, 0L, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	if (size <= 0) {
		log_fatal(file, line, EXIT_FAILURE, "Could not get size of file: %s\n", filename);
	}

	char *content = malloc(size+1); // + '\0'
	
	if (content == NULL) {
		log_fatal(file, line, EXIT_FAILURE, "Could not allocate content buffer"
			" of %d bytes for file: %s\n", size, filename);
	}

	if(fread(content, size, 1, fp) == 0) {
		log_fatal(file, line, EXIT_FAILURE, "Error reading: %s\n", filename); // TODO strerror()
	}

	content[size] = '\0';

	return content;
}

// init shader object
GLuint initShaderObj(const GLchar* filename, GLenum shaderType)
{

	GLchar *shaderCode = (GLchar *) fileReadStringMalloc(filename);

	// create the shader Object
	GLuint shader = glCreateShader(shaderType);
	if (0 == shader) {
		log_fatal(LOG, EXIT_FAILURE, "Error creating vertex shader.\n");
	}
	// log_info(LOG, "\n%s\n\n", shaderCode);
	const GLchar* codeArray[] = {shaderCode};
	glShaderSource(shader, 1, codeArray, NULL);

	free(shaderCode);

	// compile the shader
	glCompileShader(shader);
	if (GL_FALSE == compileCheck(shader)) {
		log_fatal(LOG, EXIT_FAILURE, "shader compilation failed\n");
	}
	return shader;
}
GLint checkShaderLinkStatus(GLuint pgmHandle)
{
	GLint status;
	glGetProgramiv(pgmHandle, GL_LINK_STATUS, &status);
	if (GL_FALSE == status)
	{
		GLint logLen;
		glGetProgramiv(pgmHandle, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0)
		{
			GLchar * log = (GLchar *)malloc(logLen);
			GLsizei written;
			glGetProgramInfoLog(pgmHandle, logLen, &written, log);
			log_info(LOG, "Program log: %s\n", log);
		}
	}
	return status;
}
// link shader program
GLuint createShaderPgm()
{
	// Create the shader program
	GLuint programHandle = glCreateProgram();
	if (0 == programHandle)
	{
		log_fatal(LOG, EXIT_FAILURE, "Error create shader program\n");
	}
	return programHandle;
}

// init the 1 dimentional texture for transfer function
GLuint initTFF1DTex(Map *map)
{
	GLuint tff1DTex;
	glGenTextures(1, &tff1DTex);
	glBindTexture(GL_TEXTURE_1D, tff1DTex);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#if 0
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, tff_dat);
#else
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, map->palette);
#endif
	return tff1DTex;
}

// init the 2D texture for render backface 'bf' stands for backface
GLuint initFace2DTex(GLuint bfTexWidth, GLuint bfTexHeight)
{
	GLuint backFace2DTex;
	glGenTextures(1, &backFace2DTex);
	glBindTexture(GL_TEXTURE_2D, backFace2DTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, bfTexWidth, bfTexHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	return backFace2DTex;
}

// init 3D texture to store the volume data used fo ray casting
GLuint initVol3DTex(Map *map)
{

	glGenTextures(1, &g_volTexObj);
	// bind 3D texture target
	glBindTexture(GL_TEXTURE_3D, g_volTexObj);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	// pixel transfer happens here from client to OpenGL server
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

#ifdef HEAD
	glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY, 
		256, 256, 225,
		0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 
		(GLubyte *)head256_raw);
#else
	glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY, 
		map->size.x, map->size.y, map->size.z, 
		0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 
		(GLubyte *)map->volume);
#endif

	log_info(LOG, "volume texture created\n");
	return g_volTexObj;
}

void checkFramebufferStatus()
{
	GLenum complete = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (complete != GL_FRAMEBUFFER_COMPLETE)
	{
		log_fatal(LOG, EXIT_FAILURE, "framebuffer is not complete\n");
	}
}

// init the framebuffer, the only framebuffer used in this program
void initFrameBuffer(GLuint texObj, GLuint texWidth, GLuint texHeight)
{
	// create a depth buffer for our framebuffer
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texWidth, texHeight);

	// attach the texture and the depth buffer to the framebuffer
	glGenFramebuffers(1, &g_frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, g_frameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texObj, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	checkFramebufferStatus();
	glEnable(GL_DEPTH_TEST);    
}

void rcSetUinforms(Screen *screen)
{
	// setting uniforms such as
	// ScreenSize 
	// StepSize
	// TransferFunc
	// ExitPoints i.e. the backface, the backface hold the ExitPoints of ray casting
	// VolumeTex the texture that hold the volume data i.e. head256.raw
	GLint screenSizeLoc = glGetUniformLocation(g_programHandle, "ScreenSize");
	if (screenSizeLoc >= 0)
	{
		glUniform2f(screenSizeLoc, (float)screen->size.x, (float)screen->size.y);
	}
	else
	{
		log_info(LOG, "ScreenSize is not bind to the uniform\n");
	}
	GLint stepSizeLoc = glGetUniformLocation(g_programHandle, "StepSize");
	GL_ERROR();
	if (stepSizeLoc >= 0)
	{
		glUniform1f(stepSizeLoc, g_stepSize);
	}
	else
	{
		log_info(LOG, "StepSize is not bind to the uniform\n");
	}
	GL_ERROR();
	GLint transferFuncLoc = glGetUniformLocation(g_programHandle, "TransferFunc");
	if (transferFuncLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_1D, g_tffTexObj);
		glUniform1i(transferFuncLoc, 0);
	}
	else
	{
		log_info(LOG, "TransferFunc is not bind to the uniform\n");
	}
	GL_ERROR();    
	GLint backFaceLoc = glGetUniformLocation(g_programHandle, "ExitPoints");
	if (backFaceLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, g_bfTexObj);
		glUniform1i(backFaceLoc, 1);
	}
	else
	{
		log_info(LOG, "ExitPoints is not bind to the uniform\n");
	}
	GL_ERROR();    
	GLint volumeLoc = glGetUniformLocation(g_programHandle, "VolumeTex");
	if (volumeLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_3D, g_volTexObj);
		glUniform1i(volumeLoc, 2);
	}
	else
	{
		log_info(LOG, "VolumeTex is not bind to the uniform\n");
	}

}
// init the shader object and shader program
void initShader()
{
	// vertex shader object for first pass
	g_bfVertHandle = initShaderObj("shader/backface.vert", GL_VERTEX_SHADER);
	// fragment shader object for first pass
	g_bfFragHandle = initShaderObj("shader/backface.frag", GL_FRAGMENT_SHADER);
	// vertex shader object for second pass
	g_rcVertHandle = initShaderObj("shader/raycasting.vert", GL_VERTEX_SHADER);
	// fragment shader object for second pass
	g_rcFragHandle = initShaderObj("shader/raycasting.frag", GL_FRAGMENT_SHADER);
	// create the shader program , use it in an appropriate time
	g_programHandle = createShaderPgm();
}

// link the shader objects using the shader program
void linkShader(GLuint shaderPgm, GLuint newVertHandle, GLuint newFragHandle)
{
	const GLsizei maxCount = 2;
	GLsizei count;
	GLuint shaders[maxCount];
	glGetAttachedShaders(shaderPgm, maxCount, &count, shaders);
	// log_info(LOG, "get VertHandle: %s\n", shaders[0]);
	// log_info(LOG, "get FragHandle: %s\n", shaders[1]);
	GL_ERROR();
	for (int i = 0; i < count; i++) {
		glDetachShader(shaderPgm, shaders[i]);
	}
	// Bind index 0 to the shader input variable "VerPos"
	glBindAttribLocation(shaderPgm, 0, "VerPos");
	// Bind index 1 to the shader input variable "VerClr"
	glBindAttribLocation(shaderPgm, 1, "VerClr");
	GL_ERROR();
	glAttachShader(shaderPgm,newVertHandle);
	glAttachShader(shaderPgm,newFragHandle);
	GL_ERROR();
	glLinkProgram(shaderPgm);
	if (GL_FALSE == checkShaderLinkStatus(shaderPgm)) {
		log_fatal(LOG, EXIT_FAILURE, "Failed to relink shader program!\n");
	}
	GL_ERROR();
}


// both of the two pass use the "render() function"
// the first pass render the backface of the boundbox
// the second pass render the frontface of the boundbox
// together with the frontface, use the backface as a 2D texture in the second pass
// to calculate the entry point and the exit point of the ray in and out the box.
void render(Screen *screen, GLenum cullFace)
{
	GL_ERROR();
	glClearColor(0.2f,0.2f,0.2f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 0
	//  transform the box
	glm::mat4 projection = glm::perspective(60.0f, (GLfloat)screen->size.x/screen->size.y, 0.1f, 400.f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f),
			glm::vec3(0.0f, 0.0f, 0.0f), 
			glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 model = mat4(1.0f);
	model *= glm::rotate((float)g_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	// to make the "head256.raw" i.e. the volume data stand up.
	model *= glm::rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
	model *= glm::translate(glm::vec3(-0.5f, -0.5f, -0.5f)); 
	// notice the multiplication order: reverse order of transform
	glm::mat4 mvp = projection * view * model;
#else
	GLfloat mvp[4][4] = {
		{0.620712, 0.000000, 0.934047, 0.933580},
		{1.617009, -0.000000, -0.358547, -0.358368},
		{-0.000000, -1.732051, 0.000000, 0.000000},
		{-1.118860, 0.866025, 1.513200, 1.712394}
	};
#endif

	GLuint mvpIdx = glGetUniformLocation(g_programHandle, "MVP");
	if (mvpIdx >= 0)
	{
		glUniformMatrix4fv(mvpIdx, 1, GL_FALSE, &mvp[0][0]);
	}
	else
	{
		log_info(LOG, "can't get the MVP");
	}
	GL_ERROR();
	drawBox(cullFace);
	GL_ERROR();
	// glutWireTeapot(0.5);
}

// the color of the vertex in the back face is also the location
// of the vertex
// save the back face to the user defined framebuffer bound
// with a 2D texture named `g_bfTexObj`
// draw the front face of the box
// in the rendering process, i.e. the ray marching process
// loading the volume `g_volTexObj` as well as the `g_bfTexObj`
// after vertex shader processing we got the color as well as the location of
// the vertex (in the object coordinates, before transformation).
// and the vertex assemblied into primitives before entering
// fragment shader processing stage.
// in fragment shader processing stage. we got `g_bfTexObj`
// (correspond to 'VolumeTex' in glsl)and `g_volTexObj`(correspond to 'ExitPoints')
// as well as the location of primitives.
// the most important is that we got the GLSL to exec the logic. Here we go!
// draw the back face of the box
void raycast_display(Screen *screen, Map *map)
{
	if(map->dirty) {
		initVol3DTex(map);
		map->dirty = 0;
	}

	glEnable(GL_DEPTH_TEST);
	// test the gl_error
	GL_ERROR();
	// render to texture
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_frameBuffer);
	glViewport(0, 0, screen->size.x, screen->size.y);
	linkShader(g_programHandle, g_bfVertHandle, g_bfFragHandle);
	glUseProgram(g_programHandle);
	// cull front face
	render(screen, GL_FRONT);
	glUseProgram(0); GL_ERROR();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screen->size.x, screen->size.y);
	linkShader(g_programHandle, g_rcVertHandle, g_rcFragHandle); GL_ERROR();
	glUseProgram(g_programHandle);
	rcSetUinforms(screen); GL_ERROR();
	// glUseProgram(g_programHandle);
	// cull back face
	render(screen, GL_BACK);

	// need or need not to resume the state of only one active texture unit?
	// glActiveTexture(GL_TEXTURE1);
	// glBindTexture(GL_TEXTURE_2D, 0);
	// glDisable(GL_TEXTURE_2D);
	// glActiveTexture(GL_TEXTURE2);
	// glBindTexture(GL_TEXTURE_3D, 0);    
	// glDisable(GL_TEXTURE_3D);
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_1D, 0);    
	// glDisable(GL_TEXTURE_1D);
	// glActiveTexture(GL_TEXTURE0);

	glUseProgram(0); GL_ERROR(); 

	// // for test the first pass
	// glBindFramebuffer(GL_READ_FRAMEBUFFER, g_frameBuffer);
	// checkFramebufferStatus();
	// glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	// glViewport(0, 0, screen->size.x, screen->size.y);
	// glClearColor(0.0, 0.0, 1.0, 1.0);
	// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// GL_ERROR();
	// glBlitFramebuffer(0, 0, screen->size.x, screen->size.y,0, 0,
	// 		      screen->size.x, screen->size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	// glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// GL_ERROR();
}
