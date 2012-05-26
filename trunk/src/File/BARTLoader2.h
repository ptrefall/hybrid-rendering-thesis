#pragma once

#include "BARTLoader\Declarations.h"

struct AnimationList;

namespace File
{
	class BARTLoader2;
	typedef std::shared_ptr<BARTLoader2> BARTLoader2Ptr;

	class BARTLoader2
	{
	public:
		BARTLoader2(const std::string &base_dir);
		
		std::vector<Scene::SceneNodePtr> load(const std::string& sceneFolder, const std::string& mainSceneFile);

	private:
		std::string base_dir;

		unsigned int detailLevel;
		std::string sceneFolder;
		std::string mainSceneFile;

		///////////////////////////////////
		// MISC INTERNAL PARSING FUNCTIONS
		///////////////////////////////////
		void pushNode(const std::string& name, const glm::mat4& localTransform );
		void popNode();
		
		void parseFile(const std::string &file_path);
		void parseInclude(FILE *fp);

		void recursiveSetMaterialState( const BART::InternalSceneNodePtr& node );
		void flattenSceneGraph( const BART::InternalSceneNodePtr &node, const glm::mat4 &parentXform );

		///////////////////////////////////
		// MISC INTERNAL PARSING DATA
		///////////////////////////////////
		BART::camera_def cam;
		BART::active_def active; // Loader/parser temporaries. Become stored into scene objects
		BART::anim_def anim;

		// Global scene parameters
		glm::vec3 bgcolor;
	
		// Scene objects
		AnimationList* mAnimations;
		std::vector<BART::sphere_t> sphereList;
		std::vector<BART::cone_t> coneList;
		std::vector<BART::poly_t> polyList;
		std::vector<BART::light_t> lightList;
		std::vector<Render::MaterialPtr> materialList; // TODO, store in AssetMgr

		std::vector<Scene::SceneNodePtr> sceneNodeList;
		BART::InternalSceneNodePtr sceneRoot;
	};
}
