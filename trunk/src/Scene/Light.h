#pragma once

#include "SceneNode.h"
#include "../Render/Uniform.h"

#include <glm/glm.hpp>
#include <memory>

namespace Scene
{
	class Light;
	typedef std::shared_ptr<Light> LightPtr;

	class Light : public SceneNode
	{
	public:
		Light(unsigned int lightId);

		void render(const Render::ShaderPtr &active_program) override {}

		struct Data
		{
			unsigned int lightId;
			glm::vec3 &position;
			//Expand this later
			Data(unsigned int lightId, glm::vec3 &position) : lightId(lightId), position(position) {}
		};
		typedef std::shared_ptr<Data> DataPtr;
		void setData(const DataPtr &data) { this->data = data; }
		DataPtr getData() const { return data; }

		void bind(const Render::ShaderPtr &active_program);
		void unbind();


	private:
		DataPtr data;

		Render::UniformPtr uni_position;
	};
}
