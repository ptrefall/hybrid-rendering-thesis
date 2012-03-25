#pragma once

#include <Eigen/Eigen>
#include <memory>

namespace Scene
{
	class Camera;
	typedef std::shared_ptr<Camera> CameraPtr;

	class Camera
	{
	public:
		Camera(unsigned int w, unsigned int h, float fov, float near, float far);

	private:
		Eigen::Matrix4f perspective;
		Eigen::Matrix4f view;
	};
}
