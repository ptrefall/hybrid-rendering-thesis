#include "SceneManager.h"
#include "Light.h"

using namespace Scene;

SceneManager::SceneManager()
{
}

void SceneManager::render(const Render::ShaderPtr &active_program)
{
	for(auto node : scene)
		node->render(active_program);
}

void SceneManager::bindLights(const Render::ShaderPtr &active_program)
{
	for(auto light : lights)
		light->bind(active_program);
}

void SceneManager::add(const SceneNodePtr &node)
{
	scene.push_back(node);
}

void SceneManager::add(const LightPtr &light)
{
	lights.push_back(light);
}
