#pragma once

#include "FBO.h"
#include "../File/ShaderLoader.h"
#include "Shader.h"
#include "Uniform.h"

#include <memory>

namespace Render
{
	class GBuffer;
	typedef std::shared_ptr<GBuffer> GBufferPtr;

	class GBuffer
	{
	public:
		GBuffer(const File::ShaderLoaderPtr &shader_loader, unsigned int w, unsigned int h);

		void begin();
		void end();

		void bind(unsigned int active_program);
		void unbind();

		void reshape(unsigned int w, unsigned int h);

		UniformPtr getObjectToWorldUniform() const {	return uni_object_to_world; }
		UniformPtr getWorldToViewUniform() const {		return uni_world_to_view; }
		UniformPtr getViewToClipUniform() const {		return uni_view_to_clip; }
		UniformPtr getNormalToViewUniform() const {	return uni_normal_to_view; }

		ShaderPtr getShader() const { return shader; }
		FBOPtr getFBO() const { return fbo; }

	private:
		unsigned int w;
		unsigned int h;

		unsigned int temp_w;
		unsigned int temp_h;

		FBOPtr fbo;
		File::ShaderLoaderPtr shader_loader;
		ShaderPtr shader;

		Render::UniformPtr uni_object_to_world;
		Render::UniformPtr uni_world_to_view;
		Render::UniformPtr uni_view_to_clip;
		Render::UniformPtr uni_normal_to_view;
	};
}
