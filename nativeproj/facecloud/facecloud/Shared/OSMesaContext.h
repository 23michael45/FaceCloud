#pragma once
#ifndef OSMESACONTEXT_H
#define	OSMESACONTEXT_H
#include <GL/gl.h>
#include <string>


#define LOGFILE	"facelog.log"     // all Log(); messages will be appended to this file

void Log(std::string message);    // logs a message to LOGFILE



bool MesaCreateContext(unsigned int Width, unsigned int Height);
bool MesaDestroyContext();
GLuint MesaCreateShader();
GLuint MesaCreateProgram();
#endif	/* OSMESACONTEXT_H */
