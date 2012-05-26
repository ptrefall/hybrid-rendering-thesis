#pragma once

#include <glm\glm.hpp>

#include <memory>
#include <string>
#include <stack>
#include <vector>
#include <unordered_map>

namespace Render
{
	class Material; typedef std::shared_ptr<Material> MaterialPtr;
}

namespace Scene
{
	class SceneNode; typedef std::shared_ptr<SceneNode> SceneNodePtr;
	class Mesh; typedef std::shared_ptr<Mesh> MeshPtr;
}

namespace File
{
	namespace BART
	{
		static void eatWhitespace(FILE *f)
		{
		   char ch=getc(f);
		   while(ch==' ' || ch=='\t' || ch=='\n' || ch=='\f' || ch=='\r')
			  ch=getc(f);
		   ungetc(ch,f);
		}

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

		enum eTransformType{ STATIC_TRANSFORM, ANIMATED_TRANSFORM };

		struct active_def
		{
			std::string tformName;
			glm::mat4 tformMatrix;
			std::stack<glm::mat4> tformStack;
			// Keep track of transform hiearchy so we know when to pop a static tform
			std::stack<eTransformType> tformTypeStack;
		
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

		struct anim_def
		{
			float startTime;
			float endTime;
			int numkeys;
		};
	}
}