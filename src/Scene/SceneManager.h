#pragma once

#include "SceneNode.h"
#include "../File/AssetManager.h"
#include "../File/BARTLoader2.h"
#include "../File/ShaderLoader.h"
#include "../File/TextureLoader.h"
#include "../File/MaterialLoader.h"
#include "../File/MeshLoader.h"

#include <memory>
#include <vector>

namespace Render
{
	class GBuffer_Pass; typedef std::shared_ptr<GBuffer_Pass> GBuffer_PassPtr;
	class Final_Pass; typedef std::shared_ptr<Final_Pass> Final_PassPtr;
	class Bloom_Pass; typedef std::shared_ptr<Bloom_Pass> Bloom_PassPtr;
	class Raytrace_Pass; typedef std::shared_ptr<Raytrace_Pass> Raytrace_PassPtr;
}

namespace Scene
{
	class Light;
	typedef std::shared_ptr<Light> LightPtr;

	class SceneManager;
	typedef std::shared_ptr<SceneManager> SceneManagerPtr;

	class SceneManager
	{
	public:
		SceneManager(const File::ShaderLoaderPtr &shader_loader, unsigned int width, unsigned int height, const std::string &resource_dir);
		~SceneManager();

		void render();
		void bindLights(const Render::ShaderPtr &active_program);

		void reshape(int width, int height);

		void add(const SceneNodePtr &node);
		void addList(const std::vector<SceneNodePtr> &nodeList);
		void add(const LightPtr &light);

		void initScene(	const File::AssetManagerPtr &asset_manager, 
						const File::MaterialLoaderPtr &mat_loader, 
						const File::MeshLoaderPtr &mesh_loader,
						const File::BARTLoader2Ptr &bart_loader);

	public:
		Render::GBuffer_PassPtr getGBufferPass() const { return g_buffer_pass; }
		Render::Raytrace_PassPtr getRaytracePass() const { return raytrace_pass; }
		Render::Bloom_PassPtr getBloomPass() const { return bloom_pass; }
		Render::Final_PassPtr getFinalPass() const { return final_pass; }

	private:
		File::ShaderLoaderPtr shader_loader;
		unsigned int width,height;
		std::string resource_dir;

		std::vector<SceneNodePtr> scene;
		std::vector<LightPtr> lights;

		Render::GBuffer_PassPtr g_buffer_pass;
		Render::Final_PassPtr final_pass;
		Render::Bloom_PassPtr bloom_pass;
		Render::Raytrace_PassPtr raytrace_pass;
	};
}
