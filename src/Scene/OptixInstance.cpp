#include "OptixInstance.h"
#include "proto_camera.h"

#include "../Render/ATTRIB.h"
#include "../Render/ShaderConstants.h"
#include "Mesh.h"

#include <glm/ext.hpp>
#include <vector>

using namespace Scene;
using namespace glm;

OptixInstance::OptixInstance( const Render::VAOPtr &vao, const Render::VBOPtr &vbo, const Render::IBOPtr &ibo, 
	                          optix::Geometry &geo, optix::Group &parent_group, optix::Material &optix_material)
					 : vao(vao)
					 , vbo(vbo)
					 , ibo(ibo)
					 , parent_group(parent_group)
					 , optix_material(optix_material)
{
	/////////////////////////////////
	optix::Context ctx = geo->getContext();
	instance = ctx->createGeometryInstance();
	instance->setGeometry(geo);
	instance->setMaterialCount(1);
	instance->setMaterial(0, optix_material);
	setupMaterial();

	/* create group to hold instance transform */
	geometrygroup = ctx->createGeometryGroup();
	geometrygroup->setChildCount(1);
	geometrygroup->setChild(0,instance);

	acceleration = ctx->createAcceleration("Bvh", "Bvh" );
	acceleration->setProperty("refit", "1"); // for Bvh
	geometrygroup->setAcceleration(acceleration);
	acceleration->markDirty();

	dummy = ctx->createGroup();
	dummy->setChildCount(0);
	dummy->setAcceleration( ctx->createAcceleration("NoAccel", "NoAccel" ) );
	transform = ctx->createTransform();
	addToScene();

	int count = parent_group->getChildCount();
	parent_group->setChildCount(count+1);
	parent_group->setChild(count, transform);
}

void OptixInstance::render(const Render::ShaderPtr &active_program)
{
	// do nothing. Optix is a retained mode api, add/remove this 
	// instance's node from top_level_group or use a selector program
	// to toggle rendering on/off
}

void OptixInstance::updateTransformFromMatrix( const glm::mat4 &m )
{
	glm::mat4 model = glm::transpose(m);
	transform->setMatrix(false, glm::value_ptr(model), nullptr );
	acceleration->markDirty();
}

void OptixInstance::updateTransformFromPosOriScale()
{
	//glm::mat4 model = glm::translate(mesh->getPosition()) * glm::mat4_cast(mesh->getOrientation()) * glm::scale(mesh->getScale());
	glm::mat4 model = glm::translate(position) * glm::mat4_cast(orientation) * glm::scale(scale);
	model = glm::transpose(model);
	transform->setMatrix(false, glm::value_ptr(model), nullptr );
	acceleration->markDirty();
	parent_group->getAcceleration()->markDirty();
}

void OptixInstance::updateOptixMaterial()
{
	instance["ambient_light_color"]->set3fv( glm::value_ptr(material->ambient) );
	
	/*if ( material->diffuse.length() < 0.01f ) {
		glm::vec3 dif(.5f);
		instance["Kd"]->set3fv( glm::value_ptr(dif) );
	} else {
		instance["Kd"]->set3fv( glm::value_ptr(material->diffuse) );
	}*/

	instance["Kd"]->set3fv( glm::value_ptr(material->diffuse) );
	instance["Ks"]->set3fv( glm::value_ptr(material->specular) );
	
	instance["Ka"]->set3fv( glm::value_ptr(material->ambient) );

	//glm::vec3 ka = glm::vec3(0.5f);
	//instance["Ka"]->set3fv( glm::value_ptr(ka) );

	instance["phong_exp"]->setFloat(material->pp_t_ior[0]);
	//var_reflectivity->set3fv( glm::value_ptr(reflectivity) );
}

void OptixInstance::setupMaterial()
{
	glm::vec3 ambient = glm::vec3(0.5f);
	glm::vec3 kd = glm::vec3(0.5f);
	glm::vec3 ks = glm::vec3(0.5f); 
	glm::vec3 ka = glm::vec3(0.25f); 					 
	glm::vec3 reflectivity = glm::vec3(0.8f);
	float shinyness = 0.f;

	optix::Variable var_ambient =        instance->declareVariable("ambient_light_color");
	optix::Variable var_kd =             instance->declareVariable("Kd");
	optix::Variable var_ks =             instance->declareVariable("Ks");
	optix::Variable var_ka =             instance->declareVariable("Ka");
	optix::Variable var_expv =           instance->declareVariable("phong_exp");
	optix::Variable var_reflectivity =   instance->declareVariable("reflectivity");
	var_ambient->set3fv( glm::value_ptr(ambient) );
	var_kd->set3fv( glm::value_ptr(kd) );
	var_ks->set3fv( glm::value_ptr(ks) );
	var_ka->set3fv( glm::value_ptr(ka) );
	var_expv->setFloat(shinyness);
	var_reflectivity->set3fv( glm::value_ptr(reflectivity) );
}


void OptixInstance::setTexture(int slot, const Render::Tex2DPtr &tex, const std::string &uni_name) 
{ 
}


