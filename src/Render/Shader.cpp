#include "Shader.h"
#include <GL3\gl3w.h>
#include <iostream>

using namespace Render;

Shader::Shader() 
: vs_handle(0), gs_handle(0), fs_handle(0), program_handle(0)
{
}

Shader::~Shader()
{
	if(program_handle)
		glDeleteProgram(program_handle);
}

void Shader::initialize()
{
	if(vs_handle < 0 && gs_handle < 0 && fs_handle < 0)
		throw std::runtime_error("Need to load shader code (vertex, geometry, fragment) before initializing this shader");

	program_handle = glCreateProgram();
	if(vs_handle)
	{
		glAttachShader(program_handle, vs_handle);
		glDeleteShader(vs_handle);
	}
	if(gs_handle)
	{
		glAttachShader(program_handle, gs_handle);
		glDeleteShader(gs_handle);
	}
	if(fs_handle)
	{
		glAttachShader(program_handle, fs_handle);
		glDeleteShader(fs_handle);
	}
}

/*void Shader::loadVertexShader(GMFilePtr file, const std::string &folder, const std::string &filename)
{
	std::string content = file->loadContentOfFile(folder, filename+".vs");
	vs_handle = glCreateShader(GL_VERTEX_SHADER);
	compile(vs_handle, filename, content);
	vs_folder = folder;
	vs_file = filename;
}

void Shader::loadGeometryShader(GMFilePtr file, const std::string &folder, const std::string &filename)
{
	std::string content = file->loadContentOfFile(folder, filename+".gs");
	gs_handle = glCreateShader(GL_GEOMETRY_SHADER);
	compile(gs_handle, filename, content);
	gs_folder = folder;
	gs_file = filename;
}

void Shader::loadFragmentShader(GMFilePtr file, const std::string &folder, const std::string &filename)
{
	std::string content = file->loadContentOfFile(folder, filename+".fs");
	fs_handle = glCreateShader(GL_FRAGMENT_SHADER);
	compile(fs_handle, filename, content);
	fs_folder = folder;
	fs_file = filename;
}*/

void Shader::compile(int handle, const std::string &filename, const std::string &source)
{
	const char *cs = source.c_str();
	glShaderSource(handle, 1, &cs, NULL);
    glCompileShader(handle);

	// Check for errors
	int compiled = 0;
    int infologLength = 0;

    glGetShaderiv(handle, GL_COMPILE_STATUS, &compiled);
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &infologLength);
    if(infologLength)
    {
		int charsWritten  = 0;
		char *infoLog = new char[infologLength];
		glGetShaderInfoLog(handle, infologLength, &charsWritten, infoLog);
		if(charsWritten > 0)
			std::cout << "Shader " <<  filename << " InfoLog: " << infoLog << std::endl;

		delete infoLog;
    }
	if(!compiled)
	{
		glDeleteShader(handle);
		throw std::runtime_error("Shader " + filename + " failed to compile");
	}
}

void Shader::bind()
{
	if(!program_handle)
		throw std::runtime_error("Can't bind a shader that hasn't been initialized!");

	glUseProgram(program_handle);
}

void Shader::unbind()
{
	glUseProgram(0);
}
