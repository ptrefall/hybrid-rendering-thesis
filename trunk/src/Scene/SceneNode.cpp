#include "SceneNode.h"

using namespace Scene;
using namespace Eigen;

SceneNode::SceneNode()
{
	position = Vector3f(0.0f, 0.0f, 0.0f);
}

void SceneNode::setTexture(const Render::Tex2DPtr &tex, const Render::UniformPtr &sampler) 
{ 
	this->tex = tex;
	tex_sampler = sampler;
}