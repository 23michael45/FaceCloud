#pragma once
#ifndef OSMESACONTEXT_H
#define	OSMESACONTEXT_H
#include <GL/gl.h>

bool MesaCreateContext(unsigned int Width, unsigned int Height);
bool MesaDestroyContext();
GLuint MesaCreateShader();
GLuint MesaCreateProgram();
#endif	/* OSMESACONTEXT_H */
