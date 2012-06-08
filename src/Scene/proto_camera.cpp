
#include "proto_camera.h"


using namespace Scene;

FirstPersonCameraPtr FirstPersonCamera::singleton;

FirstPersonCameraPtr FirstPersonCamera::getSingleton()
{
    if(singleton == nullptr)
        singleton = std::make_shared<FirstPersonCamera>();
    return singleton;
}

void FirstPersonCamera::Shutdown()
{
	if(singleton)
	{
		long count = singleton.use_count();
		singleton.reset();
	}
}


FirstPersonCamera::FirstPersonCamera() 
    : view_to_clip ( glm::mat4(1.f) )
	,world_to_view( glm::mat4(1.f) )
	,pos( glm::vec3(0.f) )
	,cameraStrafe( glm::vec3(1.f, 0.f, 0.f) )
	,cameraUp ( glm::vec3(0.f, 1.f, 0.f) )
	,cameraForward ( glm::vec3(0.f, 0.f, 1.f) )
	,vFov (45.f)
	,near_dist (0.1f)
	,far_dist (1000.f)
	,movementUnitsPerSecond (5.f)
	,mouseDegreesPerSecond(10.f)
	,old_mouse ( glm::vec2(0.f) )
{
}

const glm::mat4 &FirstPersonCamera::updateProjection(unsigned int w, unsigned int h, float vFov, float near, float far)
{
	view_to_clip = glm::perspective<float>(vFov, w/(float)h, near, far);
	return view_to_clip;
}

void FirstPersonCamera::lookAt( const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up )
{
	setPos(pos);
	world_to_view = glm::lookAt( pos, target, up );
	orientation = glm::quat_cast(world_to_view); // keep Quat & Matrix in sync
}

void FirstPersonCamera::update(bool left_key, bool right_key, bool back_key, bool forwards_key,
	glm::vec2 mouse_coords, bool mouse_is_down, float delta )
{
	if ( mouse_is_down )
	{	
		// Rotation deltas
		glm::vec2 mouse_speed = delta * mouseDegreesPerSecond * (mouse_coords - old_mouse);

		glm::quat rx = glm::angleAxis(mouse_speed.y, glm::vec3(1.f, 0.f, 0.f));
		glm::quat ry = glm::angleAxis(mouse_speed.x, glm::vec3(0.f, 1.f, 0.f));
		orientation = glm::normalize( rx*orientation*ry );
		world_to_view = glm::mat4_cast(orientation); // keep Quat & Matrix in sync
	}
	
    float xMove = float(right_key-left_key);
    float zMove = float(forwards_key-back_key);
	// Use view matrix direction vectors for FPS-like control
    glm::vec3 moveDir = getStrafeDirection()*xMove + getLookDirection()*zMove;
    if ( glm::length2(moveDir) ) {
		float speed = delta * movementUnitsPerSecond; // meters per second
        pos += speed * glm::normalize(moveDir);
		world_to_view = glm::translate( glm::mat4_cast(orientation) , -pos);
	}

	old_mouse = mouse_coords;
}				  