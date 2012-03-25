#include "SceneManager.h"

#include <algorithm>

using namespace Scene;

SceneManager::SceneManager()
{
}

void SceneManager::render()
{
	std::for_each(begin(scene), end(scene), [](SceneNodePtr node)
	{
		node->render();
	});
}

void SceneManager::add(const SceneNodePtr &node)
{
	scene.push_back(node);
}
