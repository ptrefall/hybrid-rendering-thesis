#include "PBO.h"

using namespace Render;

PBO::PBO(const unsigned int &size, const unsigned int &draw_type, bool unpack)
	: bound(false)
{
	glGenBuffers(1, &handle);
	bind();
	if(unpack)
		glBufferData(GL_PIXEL_UNPACK_BUFFER, size, nullptr, draw_type);
	else
		glBufferData(GL_PIXEL_PACK_BUFFER, size, nullptr, draw_type);
}

PBO::~PBO()
{
	glDeleteBuffers(1, &handle);
}

void PBO::bind(bool unpack)
{
	bound = true;
	if(unpack)
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, handle);
	else
		glBindBuffer(GL_PIXEL_PACK_BUFFER, handle);
}

void PBO::unbind(bool unpack)
{
	bound = false;
	if(unpack)
	{
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}
	else
	{
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
	}
}

void PBO::align(unsigned int bits, bool unpack)
{
	if(!bound)
		return;

	if(unpack)
		glPixelStorei(GL_UNPACK_ALIGNMENT, bits);
	else
		glPixelStorei(GL_PACK_ALIGNMENT, bits);
}

unsigned int PBO::copyToTextureOnGPU(const Tex2DPtr &tex, unsigned int offset)
{
	if(!bound)
		bind(true);

	auto error = glGetError();
	if(error != GL_NO_ERROR)
		int hello = 0;

	tex->bind();
	error = glGetError();
	if(error != GL_NO_ERROR)
		int hello = 0;

	tex->update((GLubyte*)nullptr + offset, false);

	error = glGetError();
	if(error != GL_NO_ERROR)
		int hello = 0;

	tex->unbind();

	error = glGetError();
	if(error != GL_NO_ERROR)
		int hello = 0;

	unbind(true);

	error = glGetError();
	if(error != GL_NO_ERROR)
		int hello = 0;

	return offset + (tex->getBpp() * tex->getWidth() * tex->getHeight() * sizeof(unsigned char));
}

unsigned int PBO::copyToTextureOnCPU(const Tex2DPtr &tex, unsigned int offset, unsigned int draw_type, unsigned int access)
{
	if(!bound)
		bind(true);

	tex->bind();
	bind(true);

	unsigned int buffer_size = tex->getBpp() * tex->getWidth() * tex->getHeight();
	if(tex->getInternalFormat() >= GL_RGBA32F && tex->getInternalFormat() <= GL_RGB16F)
		buffer_size *= sizeof(float);

	// Note that glMapBufferARB() causes sync issue.
	// If GPU is working with this buffer, glMapBufferARB() will wait(stall)
	// until GPU to finish its job. To avoid waiting (idle), you can call
	// first glBufferDataARB() with NULL pointer before glMapBufferARB().
	// If you do that, the previous data in PBO will be discarded and
	// glMapBufferARB() returns a new allocated pointer immediately
	// even if GPU is still working with the previous data.
	glBufferData(GL_PIXEL_UNPACK_BUFFER, buffer_size, 0, draw_type);

	unsigned char *new_tex_data = new unsigned char[buffer_size];
	// map the buffer object into client's memory
	GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, access);

	if(ptr)
	{
		// update data directly on the mapped buffer
		memcpy(new_tex_data, ptr + offset, buffer_size);
		
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); // release the mapped buffer
	}

	tex->update(new_tex_data);
	delete[] new_tex_data;

	tex->unbind();
	unbind(true);

	return offset + buffer_size;
}

unsigned int PBO::bufferFromTextureOnGPU(const Tex2DPtr &tex, unsigned int offset, unsigned int draw_type)
{
	if(!bound)
		bind(false);

	unsigned int buffer_size = tex->getBpp() * tex->getWidth() * tex->getHeight();
	if(tex->getInternalFormat() >= GL_RGBA32F && tex->getInternalFormat() <= GL_RGB16F)
		buffer_size *= sizeof(float);

	glBufferData(GL_PIXEL_PACK_BUFFER, buffer_size, nullptr, draw_type);
	tex->bind();
	tex->download(false);

	unbind(false);
	return offset + buffer_size;
}

unsigned int PBO::bufferFromTextureOnCPU(const Tex2DPtr &tex, unsigned int offset, unsigned int draw_type, unsigned int access)
{
	if(!bound)
		bind(false);

	unsigned int buffer_size = tex->getBpp() * tex->getWidth() * tex->getHeight();
	if(tex->getInternalFormat() >= GL_RGBA32F && tex->getInternalFormat() <= GL_RGB16F)
		buffer_size *= sizeof(float);

	// Note that glMapBufferARB() causes sync issue.
	// If GPU is working with this buffer, glMapBufferARB() will wait(stall)
	// until GPU to finish its job. To avoid waiting (idle), you can call
	// first glBufferDataARB() with NULL pointer before glMapBufferARB().
	// If you do that, the previous data in PBO will be discarded and
	// glMapBufferARB() returns a new allocated pointer immediately
	// even if GPU is still working with the previous data.
	glBufferData(GL_PIXEL_PACK_BUFFER, buffer_size, 0, draw_type);

	// map the buffer object into client's memory
	GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, access);

	if(ptr)
	{
		// update data directly on the mapped buffer
		memcpy(ptr + offset, tex->getData(), buffer_size);
		
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER); // release the mapped buffer
	}

	unbind(false);

	return offset + buffer_size;
}
