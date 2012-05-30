#pragma once

#include "../../../File/ShaderLoader.h"
#include "../../FBO.h"
#include "../../Shader.h"
#include "../../Uniform.h"

#include <memory>

namespace Render
{
	class GBuffer_Pass;
	typedef std::shared_ptr<GBuffer_Pass> GBuffer_PassPtr;

	class GBuffer_Pass
	{
	public:
		GBuffer_Pass(const File::ShaderLoaderPtr &shader_loader, unsigned int w, unsigned int h);

		void begin();
		void end();

		void bind(unsigned int active_program, unsigned int index_offset);
		void unbind(unsigned int index_offset);

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
