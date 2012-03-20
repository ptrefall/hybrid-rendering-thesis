#include "Uniform.h"
#include <GL3\gl3w.h>
#include <iostream>

using namespace Render;

Uniform::Uniform(unsigned int program, const std::string &name) 
	: program(program)
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
