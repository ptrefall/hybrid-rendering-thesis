#include "Spacejet.h"
#include "proto_camera.h"
#include "Light.h"

#include "../Render/ATTRIB.h"
#include "../Render/ShaderConstants.h"

#include <glm/ext.hpp>

#include <vector>

using namespace Scene;
using namespace glm;

Spacejet::Spacejet(MeshDataPtr data)
	: Mesh(data)
{
}

void Spacejet::init(const File::ShaderLoaderPtr &shader_loader)
{
	this->shader_loader = shader_loader;
	spacejet_shader = shader_loader->load("deferredSpacejetShading.vs", std::string(), "deferredSpacejetShading.fs");
	uni_object_to_world		= std::make_shared<Render::Uniform>(spacejet_shader->getVS(), "Object_to_World");
	uni_world_to_view		= std::make_shared<Render::Uniform>(spacejet_shader->getVS(), "World_to_View");
	uni_view_to_clip		= std::make_shared<Render::Uniform>(spacejet_shader->getVS(), "View_to_Clip");
	uni_normal_to_view		= std::make_shared<Render::Uniform>(spacejet_shader->getVS(), "Normal_to_View");
}

void Spacejet::render(const Render::ShaderPtr &active_program)
{
	shader_loader->push_bind(spacejet_shader);

	object_to_world = glm::translate( position) /* * glm::mat4_cast(node_orientation)*/ * glm::scale(scale);
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
	
	if(material)
		material->bind_id(active_program->getFS());

	vao->bind();

	glDrawElements(GL_TRIANGLES, ibo->size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));
	
	for(auto it=textures.begin(); it!=textures.end(); ++it)
	{
		glActiveTexture(GL_TEXTURE0+it->first);
		it->second.first->unbind();
	}

	shader_loader->pop_bind();
}

void Spacejet::setTexture(int slot, const Render::Tex2DPtr &tex, const std::string &uni_name) 
{ 
	textures[slot] = std::pair<Render::Tex2DPtr, Render::UniformPtr>(tex, std::make_shared<Render::Uniform>(spacejet_shader->getFS(), uni_name));
}
