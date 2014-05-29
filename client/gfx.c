#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/gl.h>

#define CIRCLERESOLUTION 60

static GLuint uniColorId, vbo;

void initGfx(FILE* logFile){
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(logFile, "Error: %s\n", glewGetErrorString(err));
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
#ifdef STUPIDINTEL
	glGenBuffers(1, &vboCircle);
#endif
	const GLchar* vertexPrg =
"#version 120\n"
"attribute vec2 pos;"
//"in vec3 color;"
//"out vec3 fragColor;"
"void main(){"
//"fragColor = color;"
"gl_Position = vec4(pos, 0.0, 1.0);"
"}";
	const GLchar* fragmentPrg =
"#version 120\n"
//"in vec3 fragColor;"
"uniform vec3 uniColor;"
//"out vec4 outColor;"
"void main(){"
"gl_FragColor = vec4(uniColor, 1.0);"
"}";
	GLuint vertexPrgId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexPrgId, 1, &vertexPrg, NULL);
	glCompileShader(vertexPrgId);
	GLuint fragmentPrgId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentPrgId, 1, &fragmentPrg, NULL);
	glCompileShader(fragmentPrgId);
	GLuint prgId = glCreateProgram();
	glAttachShader(prgId, vertexPrgId);
	glAttachShader(prgId, fragmentPrgId);
//	glBindFragDataLocation(prgId, 0, "outColor");
	glLinkProgram(prgId);
	glUseProgram(prgId);

	GLint status;
	glGetShaderiv(vertexPrgId, GL_COMPILE_STATUS, &status);
	if(status!=GL_TRUE){
		fputs("OMG AAAAAAAAAAAAAAA\n", logFile);
		char buffer[512];
		glGetShaderInfoLog(vertexPrgId, 512, NULL, buffer);
		fputs(buffer, logFile);
		fputc('\n', logFile);
	}
	glGetShaderiv(fragmentPrgId, GL_COMPILE_STATUS, &status);
	if(status!=GL_TRUE){
		fputs("OMG AAAAAAAAHHHHHHH\n", logFile);
		char buffer[512];
		glGetShaderInfoLog(fragmentPrgId, 512, NULL, buffer);
		fputs(buffer, logFile);
		fputc('\n', logFile);
	}

	GLint posAttrib = glGetAttribLocation(prgId, "pos");
//	GLint colorAttrib = glGetAttribLocation(prgId, "color");
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
//	glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));
	glEnableVertexAttribArray(posAttrib);
//	glEnableVertexAttribArray(colorAttrib);
	uniColorId = glGetUniformLocation(prgId, "uniColor");

	glLineWidth(2);
}

void setColorWhite(){glUniform3f(uniColorId, 1.0, 1.0, 1.0);}

void setColorFromHex(uint32_t color){
	glUniform3f(uniColorId, (float)(color&0xFF0000)/0xFF0000, (float)(color&0xFF00)/0xFF00, (float)(color&0xFF)/0xFF);
}

void drawBox(float x1, float y1, float x2, float y2){
	float points[]={
		x1, y1,
		x2, y1,
		x2, y2,
		x1, y2};
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STREAM_DRAW);
	glDrawArrays(GL_QUADS, 0, 4);
	glMapBufferRange(GL_ARRAY_BUFFER, 0, 0, GL_MAP_WRITE_BIT|GL_MAP_UNSYNCHRONIZED_BIT|GL_MAP_INVALIDATE_BUFFER_BIT);
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

void drawRectangle(float x1, float y1, float x2, float y2){
	float points[]={
		x1, y1,
		x2, y1,
		x2, y2,
		x1, y2};
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STREAM_DRAW);
	glDrawArrays(GL_LINE_LOOP, 0, 4);
	glMapBufferRange(GL_ARRAY_BUFFER, 0, 0, GL_MAP_WRITE_BIT|GL_MAP_UNSYNCHRONIZED_BIT|GL_MAP_INVALIDATE_BUFFER_BIT);
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

void drawLine(float x1, float y1, float x2, float y2){
	float points[]={x1, y1, x2, y2};
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STREAM_DRAW);
	glDrawArrays(GL_LINES, 0, 2);
	glMapBufferRange(GL_ARRAY_BUFFER, 0, 0, GL_MAP_WRITE_BIT|GL_MAP_UNSYNCHRONIZED_BIT|GL_MAP_INVALIDATE_BUFFER_BIT);
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

void drawCircle(float cx, float cy, float r){
	if(cx>0){
		if(cx> 1+r) return;
	}else{
		if(cx<-1-r) return;
	}
	if(cy>0){
		if(cy> 1+r) return;
	}else{
		if(cy<-1-r) return;
	}
	int numSegments = (int)(CIRCLERESOLUTION*sqrtf(r));
	if(numSegments<4) numSegments = 4;
	float* points = malloc(numSegments*2*sizeof(float));
	float* current = points;
	float t = 2*M_PI/numSegments;//'t' is for theta
	float myCos = cosf(t);
	float mySin = sinf(t);
	float x = r;
	float y = 0;
	int count = numSegments;
	for(; count > 0; count--){
		*(current++) = cx+x;
		*(current++) = cy+y;
		t = x;//'t' is for temporary storage
		x = myCos*x + mySin*y;
		y = myCos*y - mySin*t;
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*numSegments, points, GL_STREAM_DRAW);
	glDrawArrays(GL_LINE_LOOP, 0, numSegments);
	free(points);
	glMapBufferRange(GL_ARRAY_BUFFER, 0, 0, GL_MAP_WRITE_BIT|GL_MAP_UNSYNCHRONIZED_BIT|GL_MAP_INVALIDATE_BUFFER_BIT);
	glUnmapBuffer(GL_ARRAY_BUFFER);
}
