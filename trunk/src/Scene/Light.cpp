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

	auto cam = FirstPersonCamera::getSingleton();
	auto &view = cam->getViewMatrix();
	data->viewspace_position = vec3(view * vec4(position, 1.0));


	{
		std::stringstream ss;
		ss << "light[" << lightId << "].position";
		uni_position = std::make_shared<Render::Uniform>(ss.str());
	}
}

void Light::setPosition(const glm::vec3 &position)
{
	auto cam = FirstPersonCamera::getSingleton();
	auto &view = cam->getViewMatrix();

	data->viewspace_position = vec3(view * vec4(position, 1.0));
}

void Light::bind(const Render::ShaderPtr &active_program)
{
	uni_position->bind(data->viewspace_position, active_program->getFS());
}

void Light::unbind()
{
}
