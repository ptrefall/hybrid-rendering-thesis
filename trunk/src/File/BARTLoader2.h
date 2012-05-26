#pragma once

#include "../Render/Material.h"

#include <memory>
#include <string>
#include <stack>
#include <vector>
#include <unordered_map>

struct AnimationList;

namespace Scene
{
	class SceneNode; typedef std::shared_ptr<SceneNode> SceneNodePtr;
	class Mesh; typedef std::shared_ptr<Mesh> MeshPtr;
}

namespace File
{
	namespace BART
	{
		class InternalSceneNode;
		typedef std::shared_ptr<InternalSceneNode> InternalSceneNodePtr;
		class InternalSceneNode
		{
		public:
			InternalSceneNode(const std::string& name);
			void add( InternalSceneNodePtr child );
			void addMesh( Scene::MeshPtr& m );
			void visit(int spaces);

			std::string name;
			std::string fileScope;
			std::vector<InternalSceneNodePtr> children;
			std::vector<Scene::MeshPtr> meshes;
			glm::mat4 tform;
		};

		struct camera_def
		{
			glm::vec3 from, target, up;
			float fov;

			camera_def() {
				from = glm::vec3(0.f, 0.f, 0.f);
				target = glm::vec3(0.f, 0.f, 1.f);
				up = glm::vec3(0.f, 1.f, 0.f);
				fov = 60.f;
			}
		};

		struct active_def
		{
			std::string tformName;
			glm::mat4 tformMatrix;
			std::stack<glm::mat4> tformStack;
			// Keep track of transform hiearchy so we know when to pop a static tform
			struct tformtype
			{
				enum eTransformType{ STATIC_TRANSFORM, ANIMATED_TRANSFORM };
			};
			std::stack<tformtype::eTransformType> tformTypeStack;
		
			Render::MaterialPtr extMaterial; // found in .aff-s
			std::string texture;

			std::vector<glm::vec3> triangleVertCoords;
			std::vector<glm::vec3> triangleVertNormals;
			std::vector<glm::vec2> triangleTexCoords;

			std::unordered_map<std::string, InternalSceneNodePtr> includefileToNodeMap;

			std::stack<std::string> fileScopeStack;
			std::stack<InternalSceneNodePtr> nodeStack;
			InternalSceneNodePtr sceneNode;
			int nodeCount;
		};

		struct light_t
		{
			std::string name;
			glm::vec3 pos;
			glm::vec3 col;
		};

		struct cone_t
		{
			glm::vec3 a,b;
			float r1,r2;
			Render::MaterialPtr mat;
		};

		struct sphere_t
		{
			glm::vec3 pos;
			float radius;
			Render::MaterialPtr mat;
		};

		/* Polygon vertices don't have xforms. Vertices are in global space. */
		struct poly_t
		{
			// TODO geometry!
			glm::mat4 tform;
			Render::MaterialPtr mat;
			std::string texture;
		};
	}

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
		void pushNode(const std::string& name, const glm::mat4& localTransform = glm::mat4(1.f) );
		void popNode();
		
		void parseFile(const std::string &file_path);
		void parseInclude(FILE *fp);
		void parseXform(FILE *f);
		void endXform();
		void parseA(FILE *f);
		void parseKeyFrames(FILE *fp);
		void parseMesh(FILE *fp);

		void parseAnimParams(FILE *fp);

		void addTexturedTrianglePatch( const std::string& texturename, glm::vec3* verts, glm::vec3* norms, glm::vec2* uv );
		void addTexturedTriangle( const std::string& texturename, glm::vec3* verts, glm::vec2* uv );
		void addMesh( const std::vector<glm::vec3> &vertCoords, const std::vector<glm::vec3> &vertNormals, const std::vector<glm::vec2> &texCoords, const std::vector<unsigned int> &indices );

		void recursiveSetMaterialState( const BART::InternalSceneNodePtr& node );
		void setupAnimParams( float start, float end, int num_frames );
		void flattenSceneGraph( const BART::InternalSceneNodePtr &node, const glm::mat4 &parentXform );
		
		void getVectors(FILE *fp,char *type, std::vector<glm::vec3>& vecs);
		void getTextureCoords(FILE *fp,char *texturename,std::vector<glm::vec2>& txts);
		void getTriangles(FILE *fp,int *num_tris,std::vector<unsigned int>& indices, bool hasNorms, bool hasTexCoords);

		static void eatWhitespace(FILE *f);

		///////////////////////////////////
		// MISC INTERNAL PARSING DATA
		///////////////////////////////////

		BART::camera_def cam;

		// Loader/parser temporaries. Become stored into scene objects
		BART::active_def active;

		struct anim_def
		{
			float startTime;
			float endTime;
			int numkeys;
		} anim;

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
