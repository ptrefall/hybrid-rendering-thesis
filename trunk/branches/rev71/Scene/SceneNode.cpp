#include "SceneNode.h"

using namespace Scene;
using namespace glm;

SceneNode::SceneNode()
{
	position = vec3(0.0f, 0.0f, 0.0f);
}

void SceneNode::setTexture(const Render::Tex2DPtr &tex, const Render::SamplerPtr &sampler) 
{ 
	this->tex = tex;
	tex_sampler = sampler;
}

void SceneNode::setTexture(const Render::Tex2DArrayPtr &tex_array, const Render::SamplerPtr &sampler) 
{ 
	this->tex_array = tex_array;
	tex_sampler = sampler;
}