#pragma once
#ifndef OSMESACONTEXT_H
#define	OSMESACONTEXT_H
#include <GL/gl.h>
#include <string>
#include <ctime>
#include <sys/timeb.h>

#ifndef WIN32
#include <cstdarg>
#endif // !WIN32


#define LOGFILE	"facelog.OSMesa::Log"     // all OSMesa::Log(); messages will be appended to this file


namespace OSMesa
{
	void Log(std::string message);    // logs a message to LOGFILE

	std::string format(const char *fmt, ...);

	bool MesaCreateContext(unsigned int Width, unsigned int Height);
	bool MesaDestroyContext();
	GLuint MesaCreateShader();
	GLuint MesaCreateProgram();

	int getMilliCount();

	int getMilliSpan(int nTimeStart);
}


#endif	/* OSMESACONTEXT_H */
