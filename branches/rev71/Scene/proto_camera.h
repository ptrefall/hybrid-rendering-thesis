#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace Scene
{

class FirstPersonCamera;
typedef std::shared_ptr<FirstPersonCamera> FirstPersonCameraPtr;

class FirstPersonCamera
{
public:
	static FirstPersonCameraPtr getSingleton();
	FirstPersonCamera();

	void lookAt( const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up );

	void set( glm::vec3 const& new_pos, float horiz_degrees, float verti_degrees ) ;

	void update(bool left_key, bool right_key, bool back_key, bool forwards_key,
		float mouse_x, float mouse_y, bool mouse_is_down, float delta );

	float getHorizontalAngle() {
		return hang;
	}

	float getVerticalAngle() {
		return vang;
	}

	glm::vec3 getStrafeDirection() 
	{
		return glm::vec3( mCam[0].x, mCam[1].x, mCam[2].x );
	}

	glm::vec3 getUpDirection() 
	{
		return glm::vec3( mCam[0].y, mCam[1].y, mCam[2].y );
	}

	glm::vec3 getLookDirection() 
	{
		return -1.f * glm::vec3( mCam[0].z, mCam[1].z, mCam[2].z );
	}

	void setCameraBasis( const glm::vec3& cameraStrafe, const glm::vec3& cameraUp, const glm::vec3& cameraForward )
	{ 
		this->cameraStrafe = cameraStrafe;
		this->cameraUp = cameraUp;
		this->cameraForward = cameraForward;
	}

	float getFov(){return fov;}
	void setFov( float fov ){ this->fov = fov;}

	float getNearDist() { return near_dist; }
	float getFarDist() { return far_dist; }
	void setNearDist(float near_dist) { this->near_dist = near_dist; }
	void setFarDist(float far_dist) { this->far_dist = far_dist; }

	glm::mat4 getViewMatrix();
	void setViewMatrix( const glm::mat4& mat ) { mCam = mat; }
	void setPos(const glm::vec3& pos );
	glm::vec3 getPos();

	const glm::mat4 &updateProjection(unsigned int w, unsigned int h, float fov, float near, float far);
	const glm::mat4 &getProjection() const { return projection; }

private:
	static FirstPersonCameraPtr singleton;

	glm::mat4 projection;

	float hang, vang;
	glm::mat4 mCam;
	glm::vec3 pos;
	glm::vec3 cameraStrafe, cameraUp, cameraForward;
	float fov;
	float far_dist, near_dist;

	float oldmousx;
	float oldmousy;
};
}