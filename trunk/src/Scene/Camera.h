#pragma once

#include <Eigen/Geometry>
#include <memory>

namespace Scene
{
	class Camera;
	typedef std::shared_ptr<Camera> CameraPtr;

	class Camera
	{
	public:
        static CameraPtr getSingleton();
		Camera();

        void init(unsigned int w, unsigned int h, float fov, float near, float far);

    const Eigen::Matrix4f &updateProjection(unsigned int w, unsigned int h, float fov, float near, float far);
    const Eigen::Affine3f &updateView();

    const Eigen::Matrix4f &getProjection() const { return projection; }
    const Eigen::Affine3f &getView() const { return view; }

    void setPosition( const Eigen::Vector3f &position ) { this->position = position; }
    void setOrientation( const Eigen::Quaternionf &orientation) { this->orientation = orientation; }

    const Eigen::Vector3f &getPosition() const { return position; }

    void setDirection(const Eigen::Vector3f &direction);
    void setTarget(const Eigen::Vector3f &position);

    static CameraPtr active_camera;

	private:
        static CameraPtr singleton;
		Eigen::Matrix4f projection;
		Eigen::Affine3f view;

    Eigen::Quaternionf orientation;
    Eigen::Vector3f position;
	};
}
