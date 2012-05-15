#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>


using namespace Scene;
using namespace glm;

CameraPtr Camera::singleton;

CameraPtr Camera::getSingleton()
{
    if(singleton == nullptr)
        singleton = std::make_shared<Scene::Camera>();
    return singleton;
}

Camera::Camera()
{
}

void Camera::init(unsigned int w, unsigned int h, float fov, float near, float far)
{
    position = vec3(0,0,0);
    setDirection( vec3(0,0,1) );
    updateProjection(w,h,fov,near,far);
    view = mat4(1.0);
    updateView();
}

const glm::mat4 &Camera::updateProjection(unsigned int w, unsigned int h, float fov, float near, float far)
{
  projection = glm::perspectiveFov<float>(fov, (float)w, (float)h, near, far);
  return projection;
}

const glm::mat4 &Camera::updateView()
{
	quat q = glm::conjugate(orientation);
	auto rot_matrix = glm::mat4_cast<float>(q);
	auto trans_matrix = glm::translate(position);
	view = rot_matrix * trans_matrix;
	return view;
}

void Camera::setDirection(const glm::vec3 &direction)
{
  vec3 up = orientation * vec3(0.0f, 1.0f, 0.0f);
  mat3 camAxes;
  camAxes[2] = normalize(-direction);
  camAxes[0] = normalize(cross(up, ( camAxes[2] )));
  camAxes[1] = normalize(cross(camAxes[2],( camAxes[0] )));
  orientation = quat_cast<float>(camAxes);
}

void Camera::setTarget(const glm::vec3 &position)
{
}
