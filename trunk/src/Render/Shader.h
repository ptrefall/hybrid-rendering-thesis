#pragma once

#include "../GMEntity.h"
#include "../GMFile.h"
#include <Totem/Component.h>
#include <Totem/Property.h>

#include <memory>

namespace GM { namespace Render
{
	class Shader;
	typedef std::shared_ptr<Shader> ShaderPtr;

	class Shader : public Totem::Component<>
	{
	public:
		Shader(GMEntityPtr owner, const std::string &name);
		virtual ~Shader();
		static std::string getType() { return "Shader"; }

		void initialize();

	public:
		void loadVertexShader(GMFilePtr file, const std::string &folder, const std::string &filename);
		void loadGeometryShader(GMFilePtr file, const std::string &folder, const std::string &filename);
		void loadFragmentShader(GMFilePtr file, const std::string &folder, const std::string &filename);

		void bind();
		void unbind();

	private:
		void compile(int handle, const std::string &filename, const std::string &source);
		GMEntityPtr owner;
		unsigned int vs_handle, gs_handle, fs_handle, program_handle;

		//Potential storage-requirements in order to support file-watch-modifications
		std::string vs_folder, gs_folder, fs_folder;
		std::string vs_file, gs_file, fs_file;
	};
}}