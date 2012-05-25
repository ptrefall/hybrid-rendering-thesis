#include <string>
#include <memory>
#include <vector>

#include <proto/protographics.h>
#include "../Scene/SceneNode.h"

class BARTSceneImplementation;

class BARTScene
{
public:
	//BARTScene(){} = 0
	virtual ~BARTScene() {};

	virtual void draw()=0;

	virtual std::vector<Scene::SceneNodePtr> &getSceneNodes() = 0;

	static BARTScene* create(protowizard::ProtoGraphics *protoGfx, const std::string& sceneFolder, const std::string& mainSceneFile);
};
