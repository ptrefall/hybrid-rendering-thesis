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

void Uniform::bind(const Eigen::Vector2f &data)
{
	glProgramUniform2fv(program, location, 1, &data[0]);
}

void Uniform::bind(const Eigen::Vector3f &data)
{
	glProgramUniform3fv(program, location, 1, &data[0]);
}

void Uniform::bind(const Eigen::Matrix3f &data)
{
	glProgramUniformMatrix3fv(program, location, 1, GL_FALSE, &data(0));
}

void Uniform::bind(const Eigen::Matrix4f &data)
{
	glProgramUniformMatrix4fv(program, location, 1, GL_FALSE, &data(0));
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

void Uniform::bind(const Eigen::Vector2f &data, unsigned int program)
{
	glProgramUniform2fv(program, location, 1, &data[0]);
}

void Uniform::bind(const Eigen::Vector3f &data, unsigned int program)
{
	glProgramUniform3fv(program, location, 1, &data[0]);
}

void Uniform::bind(const Eigen::Matrix3f &data, unsigned int program)
{
	glProgramUniformMatrix3fv(program, location, 1, GL_FALSE, &data(0));
}

void Uniform::bind(const Eigen::Matrix4f &data, unsigned int program)
{
	glProgramUniformMatrix4fv(program, location, 1, GL_FALSE, &data(0));
}
