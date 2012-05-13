#include "SceneManager.h"

#include <algorithm>

using namespace Scene;

SceneManager::SceneManager()
{
}

void SceneManager::render(const Render::ShaderPtr &active_program)
{
	std::for_each(begin(scene), end(scene), [&](SceneNodePtr node)
	{
		node->render(active_program);
	});
}

void SceneManager::add(const SceneNodePtr &node)
{
	scene.push_back(node);
}
