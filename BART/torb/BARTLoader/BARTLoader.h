#include <string>
#include <memory>
#include <vector>

#include <proto/protographics.h>
#include "../Scene/SceneNode.h"


class BARTLoader
{
public:
	//BARTScene(){} = 0
	virtual ~BARTLoader() {};

	virtual glm::vec3 getBgColor() = 0;

	virtual std::vector<Scene::SceneNodePtr> &getSceneNodes() = 0;

	static BARTLoader* create(protowizard::ProtoGraphics *protoGfx, const std::string& sceneFolder, const std::string& mainSceneFile);
};
