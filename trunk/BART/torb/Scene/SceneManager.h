#pragma once

#include "SceneNode.h"

#include <proto/protographics.h>
#include <memory>
#include <vector>

namespace Scene
{
	class SceneManager;
	typedef std::shared_ptr<SceneManager> SceneManagerPtr;

	class SceneManager
	{
	public:
		SceneManager();

		void render(protowizard::ProtoGraphics &proto);

		void add(const SceneNodePtr &node);
		void addList(const std::vector<SceneNodePtr> &nodeList);

	private:
		std::vector<SceneNodePtr> scene;
	};
}
