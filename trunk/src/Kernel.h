#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <string>

namespace Render
{
	class GBuffer; typedef std::shared_ptr<GBuffer> GBufferPtr;
	class DeferredRender; typedef std::shared_ptr<DeferredRender> DeferredRenderPtr;
}
namespace Raytracer
{
	class OptixRender; typedef std::shared_ptr<OptixRender> OptixRenderPtr;
}
namespace File
{
	class ShaderLoader; typedef std::shared_ptr<ShaderLoader> ShaderLoaderPtr;
	class TextureLoader; typedef std::shared_ptr<TextureLoader> TextureLoaderPtr;
	class MaterialLoader; typedef std::shared_ptr<MaterialLoader> MaterialLoaderPtr;
}
namespace Scene
{
	class SceneManager; typedef std::shared_ptr<SceneManager> SceneManagerPtr;
	class FirstPersonCamera; typedef std::shared_ptr<FirstPersonCamera> FirstPersonCameraPtr;
}

class Kernel; typedef std::shared_ptr<Kernel> KernelPtr;
class Kernel
{
public:
	static KernelPtr getSingleton();
	~Kernel();

	void config(const std::string &resource_dir);
	void init(int argc, char** argv);
	void initScene(); //Will be deprecated at some point...

	void render();
	void reshape(int w, int h);

	void input(unsigned char key, int x, int y);
	void input(int key, int x, int y);
	void motion(int x, int y);

public:
	const std::string &getResourceDir() const { return resource_dir; }
	int getWidth() const { return width; }
	int getHeight() const { return height; }
	int getDepth() const { return depth; }
	int getRefreshRate() const { return refresh_rate; }
	int getFullscreen() const { return fullscreen; }
	int getGameMode() const { return game_mode; }
	std::string getGameModeString() const;
	int getOpenGLVersionMajor() const;
	int getOpenGLVersionMinor() const;
	std::string getOpenGLVersionString() const;

public:
	Render::GBufferPtr getGBuffer() const { return g_buffer; }
	Render::DeferredRenderPtr getRenderer() const { return renderer; }
	Raytracer::OptixRenderPtr getRaytracer() const { return raytracer; }
	File::ShaderLoaderPtr getShaderLoader() const { return shader_loader; }
	File::TextureLoaderPtr getTextureLoader() const { return tex_loader; }
	File::MaterialLoaderPtr getMaterialLoader() const { return mat_loader; }
	Scene::SceneManagerPtr getSceneManager() const { return scene; }
	Scene::FirstPersonCameraPtr getActiveCamera() const { return camera; }

private:
#if(_MSC_VER >= 1700)
	friend class std::_Ref_count_obj<Kernel>;
#else
	friend class std::tr1::_Ref_count_obj<Kernel>;
#endif
	static KernelPtr singleton;
	Kernel();

	std::string resource_dir;

	int width;
	int height;
	int depth;
	int refresh_rate;
	int fullscreen;
	int game_mode;

private:
	Render::GBufferPtr g_buffer;
	Render::DeferredRenderPtr renderer;
    Raytracer::OptixRenderPtr raytracer;
	File::ShaderLoaderPtr shader_loader;
	File::TextureLoaderPtr tex_loader;
	File::MaterialLoaderPtr mat_loader;
	Scene::SceneManagerPtr scene;
    Scene::FirstPersonCameraPtr camera;
	glm::ivec2 mouse;
};
