
#include "proto_camera.h"
#include <glm/ext.hpp>

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
/*    : projection ( glm::mat4(1.f) )
    ,view( glm::mat4(1.f) )
    ,pos( glm::vec3(0.f) )
    ,cameraStrafe( glm::vec3(1.f, 0.f, 0.f) )
    ,cameraUp ( glm::vec3(0.f, 1.f, 0.f) )
    ,cameraForward ( glm::vec3(0.f, 0.f, 1.f) )
    ,vFov (45.f)
    ,far_dist (1.f)
    ,near_dist (1000.f)
    ,movementUnitsPerSecond (50.f)
    ,mouseDegreesPerSecond(10.f)
    ,hang (0.f)
    ,vang (0.f)
    ,oldmousx (0.f)
    ,oldmousy (0.f)*/ 
{
    projection = glm::mat4(1.f);
    view = glm::mat4(1.f);
	pos = glm::vec3(0.f);
	cameraStrafe = glm::vec3(1.f, 0.f, 0.f);
    cameraUp= glm::vec3(0.f, 1.f, 0.f);
    cameraForward = glm::vec3(0.f, 0.f, 1.f);
	vFov = 45.f;
	near_dist = 1.f;
	far_dist = 1000.f;
    movementUnitsPerSecond = 50.f;
    mouseDegreesPerSecond = 10.f;
    hang = 0.f;
    vang = 0.f;
	oldmousx = 0.f;
	oldmousy = 0.f;

    pos = glm::vec3(0,0,-45);
    hang = glm::radians( 35.f );
}

const glm::mat4 &FirstPersonCamera::updateProjection(unsigned int w, unsigned int h, float vFov, float near, float far)
{
	projection = glm::perspective<float>(vFov, w/(float)h, near, far);
	return projection;
}

void FirstPersonCamera::lookAt( const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up )
{
	setPos(pos);
	view = glm::lookAt( pos, target, up );
}

//void FirstPersonCamera::quat_code()
//{
//	glm::quat q_rotate;
//	q_rotate = glm::rotate( q_rotate, hang, glm::vec3( 0, 1, 0 ) );
//	q_rotate = glm::rotate( q_rotate, -vang, glm::vec3( 1, 0, 0
//}

void FirstPersonCamera::update(bool left_key, bool right_key, bool back_key, bool forwards_key,
	float mouse_x, float mouse_y, bool mouse_is_down, float delta )
{
	float mouse_speed_x = delta * mouseDegreesPerSecond * (mouse_x - oldmousx);
	float mouse_speed_y = delta * mouseDegreesPerSecond * (mouse_y - oldmousy);
	oldmousx = mouse_x;
	oldmousy = mouse_y;
	if ( mouse_is_down )
	{	
        hang -= glm::radians(mouse_speed_x);
		vang -= glm::radians(mouse_speed_y);
	}
	float speed = delta * movementUnitsPerSecond; // meters per second

    // Direction : Spherical coordinates to Cartesian coordinates conversion
    glm::vec3 lookDir(
        cos(vang) * sin(hang),
        sin(vang),
        cos(vang) * cos(hang)
    );

    // Right vector
    glm::vec3 right = glm::vec3(
        sin(hang - 3.14f/2.0f),
        0,
        cos(hang - 3.14f/2.0f)
    );

    glm::vec3 up = glm::cross( right, lookDir );

    float xMove = float(right_key-left_key);
    float zMove = float(forwards_key-back_key);
    glm::vec3 moveDir = right*xMove + lookDir*zMove;
    if ( glm::length2(moveDir) ) 
        pos += speed * glm::normalize(moveDir);

    cameraStrafe = right;
    cameraUp = up;
    cameraForward = lookDir;

    view = glm::lookAt( pos, pos+lookDir, up );

    if ( 0 ) {
        glm::vec3 iStr = glm::vec3(view[0].x, view[1].x, view[2].x);
        glm::vec3 iHei = glm::vec3(view[0].y, view[1].y, view[2].y);
        glm::vec3 iFor = glm::vec3(view[0].z, view[1].z, view[2].z);
        glm::vec3 iPos = glm::vec3(view[3].x, view[3].y, view[3].z);
        printf("\n");
        printf("%.1f %.1f %.1f\n", cameraStrafe.x, cameraStrafe.y, cameraStrafe.z);
        printf("%.1f %.1f %.1f\n", cameraUp.x, cameraUp.y, cameraUp.z);
        printf("%.1f %.1f %.1f\n", cameraForward.x, cameraForward.y, cameraForward.z);
        printf("%.1f %.1f %.1f\n", pos.x, pos.y, pos.z);
        printf("\n");
        printf("%.1f %.1f %.1f\n", iStr.x, iStr.y, iStr.z);
        printf("%.1f %.1f %.1f\n", iHei.x, iHei.y, iHei.z);
        printf("%.1f %.1f %.1f\n", iFor.x, iFor.y, iFor.z);
        printf("%.1f %.1f %.1f\n", iPos.x, iPos.y, iPos.z);
    }

	//glm::mat4 rmx = glm::rotate( glm::mat4(1.0f), vang, cameraStrafe );
	//glm::mat4 rmy = glm::rotate( glm::mat4(1.0f), hang, cameraUp );
	//glm::mat4 rotMat = rmx * rmy;

	//glm::vec4 moveDir( speed * (left_key-right_key), 0.0f, speed * (forwards_key-back_key), 0.0f );
	//glm::vec4 lookDir = moveDir * rotMat;
	//glm::vec3 newPos = getPos() + glm::vec3( lookDir );

	//setPos(newPos);
	//view = glm::translate(rotMat, newPos);
}				  