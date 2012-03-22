#include "Uniform.h"
#include <GL3\gl3w.h>
#include <iostream>

using namespace Render;

Uniform::Uniform(unsigned int program, const std::string &name) 
	: program(program), location(-1), name(name)
{
	location = glGetUniformLocation(program, name.c_str());
	//if(location < 0)
	//	throw std::runtime_error("Couldn't find uniform with name " + name + " in shader program!");
}

void Uniform::bind(int data)
{
	glProgramUniform1i(program, location, data);
}

void Uniform::bind(float data)
{
	glProgramUniform1f(program, location, data);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Uniform::Uniform(const std::string &name) 
	: program(0), location(-1), name(name)
{
}

void Uniform::bind(int data, unsigned int program)
{
	if(location < 0)
		location = glGetUniformLocation(program, name.c_str());

	glProgramUniform1i(program, location, data);
}

void Uniform::bind(float data, unsigned int program)
{
	if(location < 0)
		location = glGetUniformLocation(program, name.c_str());

	glProgramUniform1f(program, location, data);
}
