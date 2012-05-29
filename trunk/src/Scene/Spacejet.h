#pragma once

#include "../Render/VAO.h"
#include "../Render/VBO.h"
#include "../Render/IBO.h"

#include "Mesh.h"

#include "../File/ShaderLoader.h"

#include <glm/glm.hpp>
#include <memory>

namespace Scene
{
	class Spacejet;
	typedef std::shared_ptr<Spacejet> SpacejetPtr;

	class Spacejet : public Mesh
	{
	public:
		Spacejet(MeshDataPtr data);
		void init(const File::ShaderLoaderPtr &shader_loader);

		void render(const Render::ShaderPtr &active_program) override;

		void setTexture(int slot, const Render::Tex2DPtr &tex, const std::string &uni_name) override;

	private:
		File::ShaderLoaderPtr shader_loader;
		Render::ShaderPtr spacejet_shader;
	};
}
