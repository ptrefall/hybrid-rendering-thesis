#include <string>
#include <memory>
#include <vector>

#include "../../Scene/SceneNode.h"

namespace File
{
	class BARTLoader
	{
	public:
		//BARTScene(){} = 0
		virtual ~BARTLoader() {};

		virtual glm::vec3 getBgColor() = 0;

		//virtual CameraData getCameraData() = 0; or each individual pos, lookAt, up, vFov, ...?

		virtual std::vector<Scene::SceneNodePtr> &getSceneNodes() = 0;

		static BARTLoader* create(const std::string& sceneFolder, const std::string& mainSceneFile);
	};
}
