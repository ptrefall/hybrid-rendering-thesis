#include "OptixNode.h"

#include "OptixMesh.h"
using namespace Scene;

void setupMaterial(optix::GeometryInstance instance)
{
	glm::vec3 ambient = glm::vec3(0.2f, 0.2f,0.2f);
	glm::vec3 kd = glm::vec3(0.5f, 0.5f,0.5f);
	glm::vec3 ks = glm::vec3(0.5f, 0.5f,0.5f); 
	glm::vec3 ka = glm::vec3(0.8f, 0.8f,0.8f); 					 
	glm::vec3 reflectivity = glm::vec3(0.8f, 0.8f,0.8f);
	float shinyness = 10.f;
	optix::Variable var_kd =             instance->declareVariable("Kd");
	optix::Variable var_ks =             instance->declareVariable("Ks");
	optix::Variable var_ka =             instance->declareVariable("Ka");
	optix::Variable var_expv =           instance->declareVariable("phong_exp");
	optix::Variable var_reflectivity =   instance->declareVariable("reflectivity");
	optix::Variable var_ambient =        instance->declareVariable("ambient_light_color");
	var_ambient->set3fv( glm::value_ptr(ambient) );
	var_kd->set3fv( glm::value_ptr(kd) );
	var_ks->set3fv( glm::value_ptr(ks) );
	var_ka->set3fv( glm::value_ptr(ka) );
	var_reflectivity->set3fv( glm::value_ptr(reflectivity) );
	var_expv->setFloat(shinyness);

}

OptixNode::OptixNode(const OptixMeshPtr &mesh, optix::Material material)
	: mesh(mesh)
{
	optix::Geometry geo = mesh->getGeometry();
	optix::Context ctx = geo->getContext();
	instance = ctx->createGeometryInstance();
	instance->setGeometry(geo);
	instance->setMaterialCount(1);
	instance->setMaterial(0, material);
	setupMaterial(instance);

	/* create group to hold instance transform */
	optix::GeometryGroup geometrygroup = ctx->createGeometryGroup();
	geometrygroup->setChildCount(1);
	geometrygroup->setChild(0,instance);

	acceleration = ctx->createAcceleration("Bvh", "Bvh"); // classic
	geometrygroup->setAcceleration(acceleration);
	acceleration->markDirty();

	transform = ctx->createTransform();
	transform->setChild( geometrygroup );
		
	updateTransform();
}

void OptixNode::render(const Render::ShaderPtr &active_program)
{
	mesh->setPosition(position);
	mesh->setOrientation(orientation);
	mesh->setScale(scale);
	mesh->render(nullptr);
}

void OptixNode::updateTransform()
{
	glm::mat4 optix_model_matrix = glm::transpose(glm::translate(position) * glm::mat4_cast(orientation) * glm::scale(scale));
	transform->setMatrix(false, glm::value_ptr(optix_model_matrix), nullptr );
	acceleration->markDirty();
}