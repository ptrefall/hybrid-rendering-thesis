#include "SceneManager.h"
#include "Light.h"

using namespace Scene;

SceneManager::SceneManager()
{
}

void SceneManager::render(const Render::ShaderPtr &active_program)
{
	for(auto it = begin(scene); it!=end(scene); ++it )
		(*it)->render(active_program);
}

void SceneManager::bindLights(const Render::ShaderPtr &active_program)
{
	for(auto it = begin(lights); it!=end(lights); ++it )
		(*it)->bind(active_program);
}

void SceneManager::add(const SceneNodePtr &node)
{
	scene.push_back(node);
}

void SceneManager::addList(const std::vector<SceneNodePtr> &nodeList)
{
	scene.insert( scene.end(), nodeList.begin(), nodeList.end() );
}

void SceneManager::add(const LightPtr &light)
{
	lights.push_back(light);
}
