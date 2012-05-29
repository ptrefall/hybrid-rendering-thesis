#include "SceneNode.h"

using namespace Scene;
using namespace glm;

SceneNode::SceneNode()
{
	position = vec3(0.0f, 0.0f, 0.0f);
	scale = vec3(1.0f,1.0f,1.0f);
}

void SceneNode::setTexture(int slot, const Render::Tex2DPtr &tex, const Render::UniformPtr &uniform, const Render::SamplerPtr &sampler) 
{ 
	textures[slot] = std::pair<Render::Tex2DPtr, Render::UniformPtr>(tex, uniform);
}

void SceneNode::setTexture(int slot, const Render::Tex2DArrayPtr &tex_array, const Render::UniformPtr &uniform, const Render::SamplerPtr &sampler) 
{ 
}