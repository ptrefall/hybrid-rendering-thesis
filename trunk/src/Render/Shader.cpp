#include "Shader.h"
#include <GL3\gl3w.h>
#include <iostream>

using namespace Render;

Shader::Shader(const std::string &vs_contents, const std::string &gs_contents, const std::string &fs_contents) 
{
	glGenProgramPipelines(1, &handle);

	vs = createShader(GL_VERTEX_SHADER, vs_contents);
	gs = createShader(GL_GEOMETRY_SHADER, gs_contents);
	fs = createShader(GL_FRAGMENT_SHADER, fs_contents);

	if(vs > 0) glUseProgramStages(handle, GL_VERTEX_SHADER_BIT, vs);
	if(gs > 0) glUseProgramStages(handle, GL_GEOMETRY_SHADER_BIT, gs);
	if(fs > 0) glUseProgramStages(handle, GL_FRAGMENT_SHADER_BIT, fs);
}

Shader::~Shader()
{
	if(vs > 0) glDeleteProgram(vs);
	if(gs > 0) glDeleteProgram(gs);
	if(fs > 0) glDeleteProgram(fs);
	glDeleteProgramPipelines(1, &handle);
}

void Shader::bind()
{
	glBindProgramPipeline(handle);
}

void Shader::unbind()
{
	glBindProgramPipeline(0);
}

unsigned int Shader::createShader(unsigned int type, const std::string &contents)
{
	if(contents.empty())
		return 0;

	char const *contentsPtr = contents.c_str();
	return glCreateShaderProgramv(type, 1, &contentsPtr);
}
