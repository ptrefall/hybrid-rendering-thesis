#include "ShaderLoader.h"

#include <algorithm>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>

using namespace File;

ShaderLoader::ShaderLoader(const std::string &base_dir)
	: base_dir(base_dir)
{
}

Render::ShaderPtr ShaderLoader::load(const std::string &filename)
{
	auto shader = std::make_shared<Render::Shader>();
	std::string contents = loadContents(filename);
	//initialize
	return shader;
}

std::string ShaderLoader::loadContents(const std::string &filename)
{
	int fd;
    char name[100];
    int size = -1;
	strcpy(name, (base_dir+filename).c_str());

    fd = _open(name, _O_RDONLY);
    if (fd != -1)
    {
		size = _lseek(fd, 0, SEEK_END) + 1;
		_close(fd);
    }

    if(size <= 0)
		throw std::runtime_error(("Failed to load file " + filename).c_str());

    FILE *fh;
    char *text = (char *) malloc(size);
    int count;

    fh = fopen(name, "r");
    if (!fh)
		throw std::runtime_error(("Failed to load file " + filename).c_str());

    // Get the shader from a file.
    fseek(fh, 0, SEEK_SET);
    count = (int) fread(text, 1, size, fh);
    text[count] = '\0';
	std::string return_text(text);
	free(text);
	return return_text;
}
