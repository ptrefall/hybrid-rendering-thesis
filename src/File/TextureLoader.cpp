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
	internal_tex_data data = internal_load(filename);

	Render::T2DTexParams params(data.format, data.format, GL_UNSIGNED_BYTE, data.w, data.h, wrap_mode, data.data);
	auto tex = std::make_shared<Render::Tex2D>(params);

	return tex;
}

Render::Tex2DArrayPtr TextureLoader::load_array(const std::string &filename, unsigned int width, unsigned int height, unsigned int slice_count, unsigned int wrap_mode)
{
	internal_tex_data data = internal_load(filename);
	unsigned int w = data.w / slice_count;
	unsigned int h = data.h / slice_count;

	std::vector<unsigned char> data_array;

	for(unsigned int i = 0; i < slice_count; i++)
	{
		for(unsigned int x = 0; x < w; x++)
		{
			for(unsigned int y = 0; y < h; y++)
			{

				data_array.push_back(data.data[(slice_count * (x+y*w))*data.bpp + 0]); //Red color
				data_array.push_back(data.data[(slice_count * (x+y*w))*data.bpp + 1]); //Green color
				data_array.push_back(data.data[(slice_count * (x+y*w))*data.bpp + 2]); //Blue color
				if(data.bpp == 4)
					data_array.push_back(data.data[(slice_count * (x+y*w))*data.bpp + 3]); //Alpha color
			}
		}
	}

	Render::Tex2DArrayParams params(data.format, data.format, GL_UNSIGNED_BYTE, w,h,slice_count,wrap_mode,&data_array[0]);
	auto tex = std::make_shared<Render::Tex2DArray>(params);
	return tex;
}

TextureLoader::internal_tex_data TextureLoader::internal_load(const std::string &filename)
{
	ILuint img_id;
    ilGenImages(1, &img_id);

    ilBindImage(img_id);
    int success = ilLoadImage((base_dir+filename).c_str());
    if(!success)
    {
            ILenum err = ilGetError();
            return internal_tex_data();
    }

    if(ilGetInteger(IL_IMAGE_ORIGIN) != IL_ORIGIN_LOWER_LEFT)
    {     
            iluFlipImage();
    }

    success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
    if(!success)
    {
            ILenum err = ilGetError();
            return internal_tex_data();
    }

	internal_tex_data data;
    data.bpp = ilGetInteger(IL_IMAGE_BPP);
    data.w = ilGetInteger(IL_IMAGE_WIDTH);
    data.h = ilGetInteger(IL_IMAGE_HEIGHT);
    data.format = ilGetInteger(IL_IMAGE_FORMAT);
    data.data = ilGetData();
	return data;
}