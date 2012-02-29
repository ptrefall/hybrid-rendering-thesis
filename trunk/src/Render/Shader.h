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
		Shader();
		virtual ~Shader();

		void initialize();

	public:
		//void loadVertexShader(FilePtr file, const std::string &folder, const std::string &filename);
		//void loadGeometryShader(FilePtr file, const std::string &folder, const std::string &filename);
		//void loadFragmentShader(FilePtr file, const std::string &folder, const std::string &filename);

		void bind();
		void unbind();

	private:
		void compile(int handle, const std::string &filename, const std::string &source);
		unsigned int vs_handle, gs_handle, fs_handle, program_handle;

		//Potential storage-requirements in order to support file-watch-modifications
		std::string vs_folder, gs_folder, fs_folder;
		std::string vs_file, gs_file, fs_file;
	};
}