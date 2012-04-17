#include "Camera.h"


using namespace Scene;
using namespace Eigen;

Camera::Camera(unsigned int w, unsigned int h, float fov, float near, float far)
{
  updateProjection(w,h,fov,near,far);
  view.setIdentity();
  updateView();
}

const Eigen::Matrix4f &Camera::updateProjection(unsigned int w, unsigned int h, float fov, float near, float far)
{
  projection.setIdentity();
  float aspect = w/(float)h;
  float theta = fov * 0.5f;
  float range = far - near;
  float invtan = 1.0f/tan(theta);

  projection(0,0) = invtan / aspect;
  projection(1,1) = invtan;
  projection(2,2) = -(near + far) / range;
  projection(3,2) = -1;
  projection(2,3) = -2 * near * far / range;
  projection(3,3) = 0;

  return projection;
}

const Eigen::Affine3f &Camera::updateView()
{
  Quaternionf q = orientation.conjugate();
  view.linear() = q.toRotationMatrix();
  view.translation() = - (view.linear() * position);
  return view;
}

void Camera::setDirection(const Eigen::Vector3f &direction)
{
  Vector3f up = orientation * Vector3f::UnitY();
  Matrix3f camAxes;
  camAxes.col(2) = (-direction).normalized();
  camAxes.col(0) = up.cross( camAxes.col(2) ).normalized();
  camAxes.col(1) = camAxes.col(2).cross( camAxes.col(0) ).normalized();

}

void Camera::setTarget(const Eigen::Vector3f &position)
{
}
