#include "Shader.h"
#include <GL3\gl3w.h>
#include <iostream>
#include <vector>

using namespace Render;

Shader::Shader(const std::string &vs_contents, const std::string &gs_contents, const std::string &fs_contents) 
	: pipeline_handles(nullptr), pipeline_size(0), vs_index(0), gs_index(0), fs_index(0)
{
	unsigned int shaders[3] = {0,0,0};
	shaders[0] = createShader(GL_VERTEX_SHADER, vs_contents);
	shaders[1] = createShader(GL_GEOMETRY_SHADER, gs_contents);
	shaders[2] = createShader(GL_FRAGMENT_SHADER, fs_contents);

	unsigned int shadercount = 0;
	for(auto i = 0; i < 3; i++)
		if(shaders[i] > 0)
			shadercount++;

	pipeline_size = shadercount + 1;

	pipeline_handles = new unsigned int[pipeline_size];
	glGenProgramPipelines(pipeline_size, pipeline_handles);
	bind();
	unbind();

	std::vector<unsigned int> pipe(pipeline_handles, pipeline_handles+pipeline_size);

	unsigned int pipeline_index = 1;
	for(auto i = 0; i < 3; i++)
		link(pipeline_index, shaders[i]);

	pipeline_index = 1;
	use(pipeline_index, shaders[0], GL_VERTEX_SHADER_BIT, vs_index);
	use(pipeline_index, shaders[1], GL_GEOMETRY_SHADER_BIT, gs_index);
	use(pipeline_index, shaders[2], GL_FRAGMENT_SHADER_BIT, fs_index);
}

Shader::~Shader()
{
	glDeleteProgramPipelines(pipeline_size, pipeline_handles);

	if(pipeline_handles != nullptr)
		delete[] pipeline_handles;
}

void Shader::bind()
{
	glBindProgramPipeline(pipeline_handles[0]);
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
	//return glCreateShaderProgramv(type, 1, &contentsPtr);
	unsigned int shader = glCreateShader(type);
	glShaderSource(shader, 1, &contentsPtr, NULL);
	glCompileShader(shader);

	GLint Result = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &Result);

	fprintf(stdout, "Compiling shader\n%s...\n", contentsPtr);
	int InfoLogLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> Buffer(InfoLogLength);
	glGetShaderInfoLog(shader, InfoLogLength, NULL, &Buffer[0]);
	fprintf(stdout, "%s\n", &Buffer[0]);

	return shader;
}

void Shader::link(unsigned int &handle_index, unsigned int shader)
{
	if(shader > 0)
	{
		pipeline_handles[handle_index] = glCreateProgram();
		glProgramParameteri(pipeline_handles[handle_index], GL_PROGRAM_SEPARABLE, GL_TRUE);
		glAttachShader(pipeline_handles[handle_index], shader);
		glDeleteShader(shader);
		glLinkProgram(pipeline_handles[handle_index]);
		handle_index++;
	}
}

void Shader::use(unsigned int &handle_index, unsigned int shader, unsigned int shader_bit, unsigned int &shader_index)
{
	if(shader > 0)
	{
		glUseProgramStages(pipeline_handles[0], shader_bit, pipeline_handles[handle_index]);
		shader_index = handle_index;
		handle_index++;
	}
}