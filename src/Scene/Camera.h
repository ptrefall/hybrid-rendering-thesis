#pragma once

#include <glm/ext.hpp>
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

    const glm::mat4 &updateProjection(unsigned int w, unsigned int h, float fov, float near, float far);
    const glm::mat4 &updateView();

    const glm::mat4 &getProjection() const { return projection; }
    const glm::mat4 &getView() const { return view; }
	
    void setPosition( const glm::vec3 &position ) { this->position = position; }
    void setOrientation( const glm::quat &orientation) { this->orientation = orientation; }

    const glm::vec3 &getPosition() const { return position; }

    void setDirection(const glm::vec3 &direction);
    void setTarget(const glm::vec3 &position);

    static CameraPtr active_camera;

	private:
        static CameraPtr singleton;
		glm::mat4 projection;
		glm::mat4 view;

    glm::quat orientation;
    glm::vec3 position;
	};
}
