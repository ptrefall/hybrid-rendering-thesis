#include "TextureLoader.h"

#include <IL\il.h>
#include <IL\ilu.h>

#include <algorithm>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>

using namespace File;

TextureLoader::TextureLoader(const std::string &base_dir)
	: base_dir(base_dir)
{
	ilInit();
}

Render::Tex2DPtr TextureLoader::load(const std::string &filename, unsigned int wrap_mode)
{
	ILuint img_id;
    ilGenImages(1, &img_id);

    ilBindImage(img_id);
    int success = ilLoadImage((base_dir+filename).c_str());
    if(!success)
    {
            ILenum err = ilGetError();
            return nullptr;
    }

    if(ilGetInteger(IL_IMAGE_ORIGIN) != IL_ORIGIN_LOWER_LEFT)
    {     
            iluFlipImage();
    }

    success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
    if(!success)
    {
            ILenum err = ilGetError();
            return nullptr;
    }

    int bpp = ilGetInteger(IL_IMAGE_BPP);
    int w = ilGetInteger(IL_IMAGE_WIDTH);
    int h = ilGetInteger(IL_IMAGE_HEIGHT);
    int format = ilGetInteger(IL_IMAGE_FORMAT);
    unsigned char *data = ilGetData();

	Render::T2DTexParams params(format, format, GL_UNSIGNED_BYTE, w, h, wrap_mode, data);
	auto tex = std::make_shared<Render::Tex2D>(params);

	return tex;
}
