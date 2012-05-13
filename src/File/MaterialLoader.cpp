#include "MaterialLoader.h"

#include <algorithm>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>

using namespace File;

MaterialLoader::MaterialLoader(const std::string &base_dir)
	: base_dir(base_dir)
{
}

Render::MaterialPtr MaterialLoader::load(const std::string &filename)
{
	return nullptr;
}
