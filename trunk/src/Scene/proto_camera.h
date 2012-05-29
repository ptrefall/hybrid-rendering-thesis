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
	static void Shutdown();
	FirstPersonCamera();

	void lookAt( const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up );

	void update(bool left_key, bool right_key, bool back_key, bool forwards_key,
		float mouse_x, float mouse_y, bool mouse_is_down, float delta );

	float getHorizontalAngle() { return hang; }

	float getVerticalAngle() { return vang; }

	glm::vec3 getStrafeDirection() { return glm::vec3(world_to_view[0].x, world_to_view[1].x, world_to_view[2].x); }

	glm::vec3 getUpDirection() { return glm::vec3(world_to_view[0].y, world_to_view[1].y, world_to_view[2].y); }

	glm::vec3 getLookDirection()  { 
        // compensate for glm::lookAt reversing zDir as of GL convention
        return -glm::vec3(world_to_view[0].z, world_to_view[1].z, world_to_view[2].z); 
    }

	void setCameraBasis( const glm::vec3& cameraStrafe, const glm::vec3& cameraUp, const glm::vec3& cameraForward )
	{ 
		this->cameraStrafe = cameraStrafe;
		this->cameraUp = cameraUp;
		this->cameraForward = cameraForward;
	}

	float getFov(){return vFov;}
	void setFov( float vFov ){ this->vFov = vFov;}

    void setNearDist(float near_dist) { this->near_dist = near_dist; }
	float getNearDist() { return near_dist; }

	float getFarDist() { return far_dist; }
	void setFarDist(float far_dist) { this->far_dist = far_dist; }

    void setPos(const glm::vec3& pos ) { this->pos = pos; };
    const glm::vec3 &getPos() const { return pos; }

	const glm::mat4 &updateProjection(unsigned int w, unsigned int h, float vFov, float near, float far);
	const glm::mat4 &getViewToClipMatrix() const { return view_to_clip; }
    const glm::mat4 &getWorldToViewMatrix() const { return world_to_view; }

	void setSpeed(float speed) { movementUnitsPerSecond = speed; }

private:
	static FirstPersonCameraPtr singleton;

	glm::mat4 view_to_clip;
    glm::mat4 world_to_view;
	
	glm::vec3 pos;
	glm::vec3 cameraStrafe;
    glm::vec3 cameraUp;
    glm::vec3 cameraForward;

	float vFov;
	float near_dist;
	float far_dist;
    
    float movementUnitsPerSecond;
    float mouseDegreesPerSecond;

    float hang;
    float vang;
	float oldmousx;
	float oldmousy;
};
}