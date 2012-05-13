#pragma once

#include "SceneNode.h"

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

		void render(const Render::ShaderPtr &active_program);

		void add(const SceneNodePtr &node);

	private:
		std::vector<SceneNodePtr> scene;
	};
}
