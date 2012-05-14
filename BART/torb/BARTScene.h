#include <proto/protographics.h>
#include <string>

class BARTSceneImplementation;

class BARTScene
{
public:
	//BARTScene(){} = 0
	virtual ~BARTScene() {};

	virtual void draw()=0;

	static BARTScene* create(protowizard::ProtoGraphicsPtr protoGfx, const std::string& sceneFolder, const std::string& mainSceneFile);
};
