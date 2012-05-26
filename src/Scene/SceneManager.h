#pragma once

#include "SceneNode.h"

#include <memory>
#include <vector>

namespace Scene
{
	class Light;
	typedef std::shared_ptr<Light> LightPtr;

	class SceneManager;
	typedef std::shared_ptr<SceneManager> SceneManagerPtr;

	class SceneManager
	{
	public:
		SceneManager();

		void render(const Render::ShaderPtr &active_program);
		void bindLights(const Render::ShaderPtr &active_program);

		void add(const SceneNodePtr &node);
		void addList(const std::vector<SceneNodePtr> &nodeList);
		void add(const LightPtr &light);

	private:
		std::vector<SceneNodePtr> scene;
		std::vector<LightPtr> lights;
	};
}
