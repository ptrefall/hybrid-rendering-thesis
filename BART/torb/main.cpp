#pragma comment (lib, "protowizard.lib")
#pragma comment (lib, "glfw.lib")
#pragma comment (lib, "opengl32.lib")

#include <proto/protographics.h>
#include "BARTScene.h"
#include "ini_parser.h"

protowizard::ProtoGraphicsPtr protoGfx;

int main()
{
	const char* argv[] = {"F:\\repos\\github\\protoWizard\\bin\\MinSizeRes\\teit.exe"};

	protoGfx = protowizard::ProtoGraphics::create();
	if ( !protoGfx->init(640,480, argv) ) 
	{
		return 1;
	}

	ini::Parser config("config.ini");
	std::string sceneMain = config.getString("load", "scenefile", "balls.nff");
	BARTScene* scene = BARTScene::create(protoGfx, "kitchen", sceneMain);
	// TODO. need a way to set current dir, so include files can be found relative to sceneMain.aff

	scene->init();

	while( protoGfx->isWindowOpen() ) {

		scene->draw();
		protoGfx->frame();

	}

	return 0;
}