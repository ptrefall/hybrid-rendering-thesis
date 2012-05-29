#include "GPUTimer.h"
#include <GL3/gl3w.h>

using namespace Render;

GPUTimer::GPUTimer()
	: handle(0)
{
	glGenQueries(1, &handle);
}

GPUTimer::~GPUTimer()
{
	glDeleteQueries(1, &handle);
	handle = 0;
}

void GPUTimer::begin()
{
	glBeginQuery(GL_TIME_ELAPSED, handle);	
}

void GPUTimer::end()
{
	glEndQuery(GL_TIME_ELAPSED);
}

int GPUTimer::getTimeElapsed()
{
	GLint time_elapsed = 0;
	glGetQueryObjectiv(handle, GL_QUERY_RESULT, &time_elapsed);
	return time_elapsed;
}

bool GPUTimer::isResultAvailable()
{
	GLint available = 0;
	glGetQueryObjectiv(handle, GL_QUERY_RESULT_AVAILABLE, &available);
	return available == GL_TRUE;
}