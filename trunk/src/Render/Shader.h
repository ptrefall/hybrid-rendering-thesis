#pragma once

#include <memory>
#include <string>

namespace Render
{
	class Shader;
	typedef std::shared_ptr<Shader> ShaderPtr;

	class Shader
	{
	public:
		Shader(const std::string &vs_contents, const std::string &gs_contents, const std::string &fs_contents);
		virtual ~Shader();

		void bind();
		void unbind();

		unsigned int getVS() const { if(vs_index > 0) return pipeline_handles[vs_index]; else return 0; }
		unsigned int getGS() const { if(gs_index > 0) return pipeline_handles[gs_index]; else return 0; }
		unsigned int getFS() const { if(fs_index > 0) return pipeline_handles[fs_index]; else return 0; }

	private:
		unsigned int createShader(unsigned int type, const std::string &contents);
		void link(unsigned int &handle_index, unsigned int shader);
		void use(unsigned int &handle_index, unsigned int shader, unsigned int shader_bit, unsigned int &shader_index);
		
		unsigned int *pipeline_handles;
		unsigned int pipeline_size;
		unsigned int vs_index, gs_index, fs_index;
	};
}