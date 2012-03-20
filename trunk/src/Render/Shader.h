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

		unsigned int getVS() const { return vs; }
		unsigned int getGS() const { return gs; }
		unsigned int getFS() const { return fs; }

	private:
		unsigned int createShader(unsigned int type, const std::string &contents);
		
		unsigned int handle;
		unsigned int vs, gs, fs;

	};
}