#pragma once

#include <GL3/gl3w.h>

#include <memory>

namespace Render
{
	class DebugOutput;
	typedef std::shared_ptr<DebugOutput> DebugOutputPtr;
	
	class DebugOutput
	{
	public:
		DebugOutput();
		~DebugOutput();

		static void CALLBACK DebugOutputToConsole(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,GLvoid *userParam);
		 
	private:
		unsigned int handle;
	};
}
