#pragma comment (lib, "protowizard.lib")
#pragma comment (lib, "glfw.lib")
#pragma comment (lib, "opengl32.lib")

#include <proto/protographics.h>
#include "BARTScene.h"
#include "ini_parser.h"

protowizard::ProtoGraphicsPtr protoGfx;

int main()
{
	protoGfx = protowizard::ProtoGraphics::create();

	ini::Parser protoIni("proto.ini");
	std::string protoAssets = protoIni.getString("init", "assetsdir", "F:\\repos\\github\\ProtoWizard\\bin\\assets\\");

	if ( !protoGfx->init(640,480, protoAssets) ) 
	{
		return 1;
	}

	ini::Parser config("config.ini");
	std::string sceneDir = config.getString("load", "dir", "procedural");
	std::string sceneMain = config.getString("load", "scene", "balls.nff");
	BARTScene* scene = BARTScene::create(protoGfx, sceneDir, sceneMain);

	scene->init();

	while( protoGfx->isWindowOpen() ) {

		scene->draw();
		protoGfx->frame();

	}

	return 0;
}