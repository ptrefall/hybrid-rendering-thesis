#include "PBO.h"

using namespace Render;

PBO::PBO(const unsigned int &size, const unsigned int &draw_type, bool unpack)
	: bind_state(PBO_UNBOUND)
{
	glGenBuffers(1, &handle);
	bind(unpack);
	glBufferData(bind_state, size, nullptr, draw_type);
}

PBO::~PBO()
{
	glDeleteBuffers(1, &handle);
}

void PBO::bind(bool unpack)
{
	if(bind_state != PBO_UNBOUND && bind_state != (unpack ? PBO_UNPACK_BOUND : PBO_PACK_BOUND))
	{
		std::string bind_state_str = unpack ? "Unpack Buffer" : "Pack Buffer";
		throw std::runtime_error("Trying to bind a PBO to " + bind_state_str + ", but it's already bound to the opposite buffer type!");
	}
	bind_state = (unpack ? PBO_UNPACK_BOUND : PBO_PACK_BOUND);
	glBindBuffer(bind_state, handle);
}

void PBO::unbind()
{
	if(!bind_state)
		return;

	glBindBuffer(bind_state, 0);
	if(bind_state == PBO_UNPACK_BOUND)
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	else
		glPixelStorei(GL_PACK_ALIGNMENT, 4);

	bind_state = PBO_UNBOUND;
}

void PBO::align(unsigned int bits)
{
	if(!bind_state)
		return;

	if(bind_state == PBO_UNPACK_BOUND)
		glPixelStorei(GL_UNPACK_ALIGNMENT, bits);
	else
		glPixelStorei(GL_PACK_ALIGNMENT, bits);
}

unsigned int PBO::copyToTextureOnGPU(const Tex2DPtr &tex, unsigned int offset)
{
	if(!bind_state)
		bind(true);

	tex->bind();
	tex->update((GLubyte*)nullptr + offset, false);
	tex->unbind();

	unbind();
	return offset + (tex->getBpp() * tex->getWidth() * tex->getHeight() * sizeof(unsigned char));
}

unsigned int PBO::copyToTextureOnCPU(const Tex2DPtr &tex, unsigned int offset, unsigned int draw_type, unsigned int access)
{
	if(!bind_state)
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
	unbind();

	return offset + buffer_size;
}

unsigned int PBO::bufferFromTextureOnGPU(const Tex2DPtr &tex, unsigned int offset, unsigned int draw_type)
{
	if(!bind_state)
		bind(false);

	unsigned int buffer_size = tex->getBpp() * tex->getWidth() * tex->getHeight();
	if(tex->getInternalFormat() >= GL_RGBA32F && tex->getInternalFormat() <= GL_RGB16F)
		buffer_size *= sizeof(float);

	glBufferData(GL_PIXEL_PACK_BUFFER, buffer_size, nullptr, draw_type);
	tex->bind();
	tex->download(false);

	unbind();
	return offset + buffer_size;
}

unsigned int PBO::bufferFromTextureOnCPU(const Tex2DPtr &tex, unsigned int offset, unsigned int draw_type, unsigned int access)
{
	if(!bind_state)
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

	unbind();

	return offset + buffer_size;
}
