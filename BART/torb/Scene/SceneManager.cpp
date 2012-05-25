#include "SceneManager.h"

#include <algorithm>

using namespace Scene;

SceneManager::SceneManager()
{
}

void SceneManager::render(protowizard::ProtoGraphics &proto)
{
	std::for_each(begin(scene), end(scene), [&](SceneNodePtr node)
	{
		node->render(proto);
	});
}

void SceneManager::add(const SceneNodePtr &node)
{
	scene.push_back(node);
}

void SceneManager::addList(const std::vector<SceneNodePtr> &nodeList)
{
	scene.insert( scene.end(), nodeList.begin(), nodeList.end() );
}
