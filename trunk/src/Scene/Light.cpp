#include "Light.h"
#include "proto_camera.h"

#include <glm/ext.hpp>

#include <vector>
#include <sstream>

using namespace Scene;
using namespace glm;

Light::Light(unsigned int lightId)
{
	data = std::make_shared<Data>(lightId);

	auto &world_to_view = FirstPersonCamera::getSingleton()->getWorldToViewMatrix();
	data->viewspace_position = vec3((world_to_view) * vec4(position, 1.0f));

	{
		std::stringstream ss;
		ss << "light[" << lightId << "].position_vs";
		uni_position = std::make_shared<Render::Uniform>(ss.str());
	}
}

void Light::setPosition(const glm::vec3 &position)
{
	auto &world_to_view = FirstPersonCamera::getSingleton()->getWorldToViewMatrix();
	data->viewspace_position = vec3((world_to_view) * vec4(position, 1.0f));
}

void Light::bind(const Render::ShaderPtr &active_program)
{
	uni_position->bind(data->viewspace_position, active_program->getFS());
}

void Light::unbind()
{
}
