#include "OptixMesh.h"
#include "proto_camera.h"

#include "../Render/ATTRIB.h"
#include "../Render/ShaderConstants.h"
#include "Mesh.h"

#include <glm/ext.hpp>
#include <vector>

using namespace Scene;
using namespace glm;

OptixMesh::OptixMesh(const Scene::MeshPtr &triangle_mesh, optix::Geometry &geo, optix::Group &parent_group, /*optix::Acceleration &acceleration,*/ optix::Material &optix_material)
                     : triangle_mesh(triangle_mesh)
					 , parent_group(parent_group)
					 //, acceleration(acceleration)
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

void OptixMesh::render(const Render::ShaderPtr &active_program)
{
	static bool bad_practice = true;
	if ( bad_practice ) {
		bad_practice = false;
		uni_object_to_world		= std::shared_ptr<Render::Uniform>( new Render::Uniform(active_program->getVS(), "Object_to_World") );
		uni_world_to_view		= std::shared_ptr<Render::Uniform>( new Render::Uniform(active_program->getVS(), "World_to_View") );
		uni_view_to_clip		= std::shared_ptr<Render::Uniform>( new Render::Uniform(active_program->getVS(), "View_to_Clip") );
		uni_normal_to_view		= std::shared_ptr<Render::Uniform>( new Render::Uniform(active_program->getVS(), "Normal_to_View") );
	}
	active_program->bind();

	object_to_world = glm::translate(position) * glm::mat4_cast(orientation) * glm::scale(scale);
	auto &world_to_view = FirstPersonCamera::getSingleton()->getWorldToViewMatrix();
	auto &view_to_clip = FirstPersonCamera::getSingleton()->getViewToClipMatrix();
	auto normal_to_view = transpose(inverse(mat3(world_to_view * object_to_world)));

	uni_object_to_world->	bind(object_to_world);
	uni_world_to_view->		bind(world_to_view);
	uni_view_to_clip->		bind(view_to_clip);
	uni_normal_to_view->	bind(normal_to_view);

	int counter = 0;
	for(auto it=textures.begin(); it!=textures.end(); ++it)
	{
		glActiveTexture(GL_TEXTURE0+it->first);
		it->second.first->bind();
		it->second.second->bind((int)it->first);
		counter++;
	}
	
	//if(material)
		//material->bind_id(active_program->getFS());

	triangle_mesh->getVao()->bind();

	glDrawElements(GL_TRIANGLES, triangle_mesh->getIbo()->size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));
}

void OptixMesh::updateTransformFromMatrix( const glm::mat4 &m )
{
	glm::mat4 model = glm::transpose(m);
	transform->setMatrix(false, glm::value_ptr(model), nullptr );
	acceleration->markDirty();
}

void OptixMesh::updateTransformFromPosOriScale()
{
	//glm::mat4 model = glm::translate(mesh->getPosition()) * glm::mat4_cast(mesh->getOrientation()) * glm::scale(mesh->getScale());
	glm::mat4 model = glm::translate(position) * glm::mat4_cast(orientation) * glm::scale(scale);
	model = glm::transpose(model);
	transform->setMatrix(false, glm::value_ptr(model), nullptr );
	acceleration->markDirty();
	parent_group->getAcceleration()->markDirty();
}

void OptixMesh::updateOptixMaterial()
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

void OptixMesh::setupMaterial()
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


void OptixMesh::setTexture(int slot, const Render::Tex2DPtr &tex, const std::string &uni_name) 
{ 
}


