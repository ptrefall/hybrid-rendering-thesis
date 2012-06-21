#include "BARTLoader2.h"

#include "..\Parser\INIParser.h"
#include "..\Scene\BARTMesh.h"
#include "..\Render\Material.h"

#include "..\Parser\BART\ParseComment.h"
#include "..\Parser\BART\ParseViewpoint.h"
#include "..\Parser\BART\ParseLight.h"
#include "..\Parser\BART\ParseBackground.h"
#include "..\Parser\BART\ParseFill.h"
#include "..\Parser\BART\ParseCone.h"
#include "..\Parser\BART\ParseSphere.h"
#include "..\Parser\BART\ParsePoly.h"
#include "..\Parser\BART\ParseDetailLevel.h"
#include "..\Parser\BART\ParseTextureStuff.h"
#include "..\Parser\BART\ParseXform.h"
#include "..\Parser\BART\ParseA.h"
#include "..\Parser\BART\ParseKeyFrames.h"
#include "..\Parser\BART\ParseMesh.h"

using namespace File;

BARTLoader2::BARTLoader2(const AssetManagerPtr &asset_manager, const std::string &base_dir)
	: asset_manager(asset_manager), base_dir(base_dir)
{
}

std::vector<Scene::BARTMeshPtr> &BARTLoader2::load(const std::string& sceneFolder, const std::string& mainSceneFile)
{
	this->sceneFolder = sceneFolder;
	this->mainSceneFile = mainSceneFile;

	detailLevel = 0;
	mAnimations = nullptr;
	active.tformMatrix = glm::mat4(1.0f);

	pushNode("root", glm::mat4(1.0f));
	parseFile(base_dir + sceneFolder + mainSceneFile);

	flattenSceneGraph_r( sceneRoot, glm::mat4(1.f) );
	return sceneNodeList;
}

//////////////////////////////////////////////////////////////
// INTERNAL FUNCTIONS
//////////////////////////////////////////////////////////////

void BARTLoader2::pushNode(const std::string& name, const glm::mat4& localTransform ) 
{
	auto newNode = std::make_shared<BART::InternalSceneNode>(name);

	if ( name == "root" ) 
		active.fileScopeStack.push("root");

	std::string includeName = active.fileScopeStack.top();

	if ( name == "root" )
	{
		active.sceneNode = newNode;
		sceneRoot = newNode;
		active.nodeCount = 0;
	} 
	// TODO. seems like a stupid solution. could make a distinct container/namespace type for .aff's?
	else if (name == "null") 
	{ 
		active.sceneNode->add( newNode );
		newNode->tform = glm::mat4(1.f);
		active.sceneNode = newNode;
		active.includefileToNodeMap[includeName] = active.sceneNode;
	}
	else 
	{
		active.sceneNode->add( newNode );
		active.sceneNode = newNode;
	}

	active.sceneNode->tform = localTransform;
	active.sceneNode->fileScope = includeName;
	active.nodeStack.push( active.sceneNode );

	active.nodeCount++;
}

void BARTLoader2::popNode()
{
	active.nodeStack.pop();
	active.sceneNode = active.nodeStack.top();
}

/*----------------------------------------------------------------------
  viParseFile()
  Description:
    parses the animation file
----------------------------------------------------------------------*/
void BARTLoader2::parseFile(const std::string &file_path)
{
	std::unique_ptr<FILE, int(*)(FILE*)> f(fopen(file_path.c_str(), "r"), fclose);
	if(f == nullptr)
		throw std::runtime_error("Could not open .aff file");

	char ch;

	while((ch=(char)getc(f.get()))!=EOF)
	{  
		switch(ch)
		{
		case ' ':   /* white space */
		case '\t':
		case '\n':
		case '\f':
		case '\r':
			continue;
		case '#':   /* comment */
		case '%':   /* comment */
			Parser::BART::ParseComment::parse(f.get()); /* ok */
			break;
		case 'v':   /* view point */
			Parser::BART::ParseViewpoint::parse(f.get(), cam); /* ok */
			break;
		case 'l':   /* light source */
			Parser::BART::ParseLight::parse(f.get(), lightList); /* ok */
			break;
		case 'b':   /* background color */
			Parser::BART::ParseBackground::parse(f.get(), bgcolor); /* ok */
			break;
		case 'f':   /* fill material */
			Parser::BART::ParseFill::parse(f.get(), materialList, active); /* ok */
			break;
		case 'c':   /* cylinder or cone */
			Parser::BART::ParseCone::parse(f.get(), active.extMaterial, coneList); /* ok */ 
			break;
		case 's':   /* sphere */
			Parser::BART::ParseSphere::parse(f.get(), active.extMaterial, sphereList); /* ok */
			break;
		case 'p':   /* polygon or patch */
			Parser::BART::ParsePoly::parse(f.get());
			break;
		case 'i':   /* include another file */
			parseInclude(f.get());  /* ok */
			break;
		case 'd':   /* detail level of file (used to exclude objects from rendering) */
			Parser::BART::ParseDetailLevel::parse(f.get(), detailLevel); /* ok */
			break;
		case 't':  /* textured triangle, or texture tripatch, or animated triangle */
			Parser::BART::ParseTextureStuff::parse(f.get(), base_dir, sceneFolder, active);
			break;
		case 'x':  /* transform */
			Parser::BART::ParseXform::parse(f.get(), active, [this](const std::string& name, const glm::mat4& localTransform) { this->pushNode(name, localTransform); });
			break;
		case '}':
			Parser::BART::ParseXform::end(active, [this]() { this->popNode(); });
			break;
		case 'a':  /* global amb light or animation parameters */
			Parser::BART::ParseA::parse(f.get(), anim);
			break;
		case 'k':  /* key frames for transform (or the camera) */
			Parser::BART::ParseKeyFrames::parse(f.get(), mAnimations);
			break;
		case 'm':  /* triangle mesh */
			Parser::BART::ParseMesh::parse(f.get(), file_path, base_dir, sceneFolder, active, asset_manager);
			break;
		default:    /* unknown */
			throw std::runtime_error("unknown NFF primitive code: " + ch);
		}
	}
}

/*----------------------------------------------------------------------
  parseInclude()
  Include another file (typically containing geometry)
  Description:  
    "i" detail_level filename

Format:
    i %d %s

    The file name may not include any white spaces.
----------------------------------------------------------------------*/
void BARTLoader2::parseInclude(FILE *fp)
{
	char filename[100];
	int detail_level;
	if(fscanf(fp,"%d %s",&detail_level,filename)!=2)
		std::runtime_error("Error: could not parse include.");
	
	auto includeName = std::string(filename);
	auto sceneNodeIt = active.includefileToNodeMap.find( includeName ); 
	
	auto it = active.includefileToNodeMap.find( includeName );
	if ( it != active.includefileToNodeMap.end() ) // seen before
	{
		auto subtreeAtInc = (*sceneNodeIt).second;
		assert( subtreeAtInc->name == "null" );
		active.sceneNode->add( subtreeAtInc );
		setMaterialState_r( active.sceneNode );
		//printf("attaching subtree to %s::%s\n", active.sceneNode->fileScope.c_str(), active.sceneNode->name.c_str() );
		//subtreeAtInc->visit(0);
	}
	else
	{
		if(detail_level<=detailLevel) /* skip file if our detail is less than the global detail */
		{
			active.fileScopeStack.push(includeName);
			pushNode("null", glm::mat4(1.0f));


			try {
				parseFile(base_dir+sceneFolder+filename);  /* parse the file recursively */
			} catch(std::exception &e) {
				throw std::runtime_error("Error: could not parse: " + std::string(filename) + " reason: "+ e.what());
			}

			popNode();
			active.fileScopeStack.pop();
		}
		else
			std::cout << "Skipping include file: " << filename << std::endl;
	}
}

/* 
	needed for robots/city2.aff
	Since .aff files are only loaded once, load the state of a node associated
	with material state from that file
*/
void BARTLoader2::setMaterialState_r( const BART::InternalSceneNodePtr& node ) 
{
	if ( node->children.size() )
	{
		setMaterialState_r( node->children.front() );

		//const Scene::BARTMeshPtr& leaf = node->meshes.front();
		// TODO:
		//auto it = MeshMatMap.find(leaf);
		//if ( it != MeshMatMap.end() ) {
		//	active.extMaterial;
		//} else {
		//	// should allways be found.
		//}

		// TODO
		//active.texture = leaf.texture;
	}
}


void BARTLoader2::flattenSceneGraph_r( const BART::InternalSceneNodePtr &node, const glm::mat4 &parentXform /*bool flattenTransform*/)
{
	glm::mat4 combinedXform = parentXform * node->tform; // first apply local- then parent xform
	
	if ( node->mesh.get() != nullptr )
	{
		auto finalMesh = std::make_shared<Scene::BARTMesh>( node->mesh );
		finalMesh->setMaterial( node->material );
		finalMesh->setObjectToWorldMatrix( combinedXform ); // still need to use global xform.
		sceneNodeList.push_back( finalMesh );
	}

	for(auto it=begin(node->children); it!=end(node->children); ++it )
		flattenSceneGraph_r( *it, combinedXform );
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
// INTERNAL SCENE NODE
////////////////////////////////////////////////////
////////////////////////////////////////////////////

File::BART::InternalSceneNode::InternalSceneNode(const std::string& name) 
	: name(name)
{
}

void File::BART::InternalSceneNode::add( InternalSceneNodePtr child ) 
{
	children.push_back(child);
}

void File::BART::InternalSceneNode::setMeshMaterial( const Scene::MeshDataPtr &mesh, const Render::MaterialPtr &material )
{
	if ( this->mesh.get() != nullptr ) {
		throw std::runtime_error("tried set on InternalSceneNode that already has a mesh");
	}
	this->mesh = mesh;
	this->material = material;
}

void File::BART::InternalSceneNode::visit(int spaces) 
{
	for(int i=0; i<spaces; i++)
		std::cout << " ";

	if ( name == "null" ) 
	{
		std::cout << "scope " << fileScope << std::endl;
		spaces +=3;
	}
	else
		std::cout << "visit " << name;

	if ( mesh.get() != nullptr ) 
		std::cout << " has geo ";
	std::cout << std::endl;

	for (auto it=children.begin(); it!=children.end(); ++it)
		(*it)->visit(spaces+1);
}