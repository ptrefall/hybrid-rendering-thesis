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

Render::Tex2DArrayPtr TextureLoader::load_array(const std::string &filename, unsigned int width, unsigned int height, unsigned int slice_count_width, unsigned int slice_count_height, unsigned int wrap_mode)
{
	internal_tex_data data = internal_load(filename);
	unsigned int w = width;
	unsigned int h = height;
	unsigned int tile_size = w*h;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	unsigned int slice_count = slice_count_width*slice_count_height;

	Render::Tex2DArrayParams params(data.format, data.format, GL_UNSIGNED_BYTE, w,h,slice_count,wrap_mode,data.data);
	auto tex = std::make_shared<Render::Tex2DArray>(params);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	return tex;
}

/*Render::Tex2DArrayPtr TextureLoader::load_array(const std::string &filename, unsigned int width, unsigned int height, unsigned int slice_count_width, unsigned int slice_count_height, unsigned int wrap_mode)
{
	internal_tex_data data = internal_load(filename);
	unsigned int w = width;
	unsigned int h = height;
	unsigned int tile_size = w*h;

	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	unsigned int slice_count = slice_count_width*slice_count_height;
	Render::Tex2DArrayParams params(data.format, data.format, GL_UNSIGNED_BYTE, w,h,slice_count,wrap_mode,nullptr);
	auto tex = std::make_shared<Render::Tex2DArray>(params);

	std::vector<unsigned char> texels; //((slice_count*tile_size*data.bpp));
	for(unsigned int s_y = 0; s_y < slice_count_height; s_y++)
	for(unsigned int s_x = 0; s_x < slice_count_width; s_x++)
	{
		unsigned int z = s_x + (s_y*slice_count_width);

		texels.resize(w*h*data.bpp);
		for(unsigned int y = 0; y < h; y++)
		for(unsigned int x = 0; x < w; x++)
		{
			for(unsigned int p = 0; p < data.bpp; p++)
			{
				auto texel_index = (x + (y*w))*data.bpp + p;
				int data_index = (z*tile_size*data.bpp) + texel_index;
				texels[texel_index] = data.data[data_index];
			}
		}
		
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, z, w, h, 1, data.format, GL_UNSIGNED_BYTE, &texels[0]);
		texels.clear();
	}
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	return tex;
}*/

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