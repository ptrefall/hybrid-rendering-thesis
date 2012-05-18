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

void Camera::Shutdown()
{
	if(singleton)
	{
		long count = singleton.use_count();
		singleton.reset();
	}
}

Camera::Camera()
{
}

void Camera::init(unsigned int w, unsigned int h, float fov, float near, float far)
{
    position = vec3(0,0,0);
    setDirection( vec3(0,0,1) );
    updateProjection(w,h,fov,near,far);
    updateView();
}

const glm::mat4 &Camera::updateProjection(unsigned int w, unsigned int h, float fov, float near, float far)
{
  projection = glm::perspectiveFov<float>(fov, (float)w, (float)h, near, far);
	//projection = glm::perspective<float>(fov, w/h, near, far);
  return projection;
}

const glm::mat4 &Camera::updateView()
{
	quat q = glm::conjugate(orientation);
	auto rot_matrix = glm::mat4_cast<float>(q);
	auto trans_matrix = glm::translate(position);
	view = trans_matrix * rot_matrix;
	//view = glm::translate(vec3(position.x,position.y,position.z));

	// Calculate the translation matrix using the local position vector
    /*mat4 translate = mat4(1.0f);
    translate[3][0] = position.x;
    translate[3][1] = position.y;
    translate[3][2] = position.z;

    mat4 rotation = mat4_cast(orientation);

    // Then, we calculate the local transformation matrix
    view = mat4(1.0f);
    //view *= scale;
    view *= rotation;
    view *= translate;*/
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
	view = lookAt(this->position, position, vec3(0.0f, 1.0f, 0.0f));
}
