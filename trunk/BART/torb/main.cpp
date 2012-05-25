#pragma comment (lib, "protowizard.lib")
#pragma comment (lib, "glfw.lib")
#pragma comment (lib, "opengl32.lib")

#include <proto/protographics.h>
#include "ini_parser.h"

#include "BARTLoader/BARTLoader.h"
#include "Scene/SceneManager.h"

int main()
{
	protowizard::ProtoGraphics proto;

	ini::Parser protoIni("proto.ini");
	std::string protoAssets = protoIni.getString("init", "assetsdir", "F:\\repos\\github\\ProtoWizard\\bin\\assets\\");

	if ( !proto.init(640,480, protoAssets) ) 
	{
		return 1;
	}

	ini::Parser config("config.ini");
	std::string sceneDir = config.getString("load", "dir", "procedural");
	std::string sceneMain = config.getString("load", "scene", "balls.nff");
	BARTLoader* loader = BARTLoader::create(&proto, sceneDir, sceneMain);

	Scene::SceneManager sceneMgr;
	const auto &sceneNodes = loader->getSceneNodes();
	sceneMgr.addList( sceneNodes );

	proto.setFrameRate(60);
	proto.setColor(0.75f, 0.75f, 0.75f);
	while( proto.isWindowOpen() ) {
		const glm::vec3 bgColor = loader->getBgColor();
		proto.cls( bgColor.r, bgColor.g, bgColor.b );

		sceneMgr.render( proto );

		float speed = 0.25f;
		speed += proto.getMouseWheel() * 0.05f;
		if ( speed < 0.01f ) speed = 0.01f;
		proto.getCamera()->update( proto.keystatus(protowizard::KEY::LEFT), proto.keystatus(protowizard::KEY::RIGHT), proto.keystatus(protowizard::KEY::DOWN), proto.keystatus(protowizard::KEY::UP), (float)proto.getMouseX(), (float)proto.getMouseY(), proto.mouseDownLeft(), speed * proto.getMSPF() );

		proto.frame();

	}

	return 0;
}