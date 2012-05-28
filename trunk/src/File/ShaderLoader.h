#pragma once

#include "../Render/Shader.h"

#include <memory>
#include <string>
#include <stack>

namespace File
{
	class ShaderLoader;
	typedef std::shared_ptr<ShaderLoader> ShaderLoaderPtr;

	class ShaderLoader
	{
	public:
		ShaderLoader(const std::string &base_dir);

		Render::ShaderPtr load(const std::string &vs_filename, const std::string &gs_filename = std::string(), const std::string &fs_filename = std::string());

		void push_bind(const Render::ShaderPtr &shader);
		void pop_bind();

	private:
		std::string loadContents(const std::string &filename);
		std::string base_dir;
		std::stack<Render::ShaderPtr> boundShaderStack;
	};
}
