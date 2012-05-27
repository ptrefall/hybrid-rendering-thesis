#include "ParseMesh.h"

#include "..\..\File\BARTLoader2.h"
#include "..\..\File\AssetManager.h"
#include "..\..\Scene\Mesh.h"
#include "..\..\Kernel.h"
#include "..\..\Render\GBuffer.h"

#include <sstream>
#include <memory>

using namespace Parser;
using namespace BART;


void ParseMesh::parse(FILE *fp, const std::string &base_dir, const std::string &sceneFolder, File::BART::active_def &active, const File::AssetManagerPtr &asset_manager)
{
   char str[200];
   int num_tris = 0;
   std::vector<glm::vec3> verts;
   std::vector<glm::vec3> norms;
   std::vector<glm::vec2> txts;
   std::vector<unsigned int> indices; // contains indices into txts, norms, verts arrays.
   char texturename[200];

   if(fscanf(fp,"%s",str)!=1)
      throw std::runtime_error("Error: could not parse mesh (could not find 'vertices').");

   if(strcmp(str,"vertices"))
      throw std::runtime_error("Error: could not parse mesh (expected 'vertices').");

   getVectors(fp,"vertices",verts);

   fscanf(fp,"%s",str);
   if(!strcmp(str, "normals"))
   {
      getVectors(fp,"normals",norms);
      fscanf(fp,"%s",str);
   }
   if(!strcmp(str, "texturecoords"))
   {
      getTextureCoords(fp,texturename,txts);
      fscanf(fp,"%s",str);
	  active.texture = std::string(texturename);
	  active.texture = base_dir + sceneFolder + active.texture.substr(0,active.texture.size()-3) + "dds";
   } else {
	   active.texture = "";
   }
   
   if(!strcmp(str,"triangles"))
   {
      getTriangles(fp,&num_tris, indices, norms.size() > 0, txts.size() > 0);
   }
   else
      throw std::runtime_error("Error: expected 'triangles' in mesh.");

   /* add a mesh here
    * e.g.,viAddMesh(verts,num_verts,norms,num_norms,txts,num_txts,texturename,indices,num_tris);
    */
   // TODO use opengl indexed
	size_t coordIdx = 0;
	std::vector<glm::vec3> vertCoords;
	std::vector<glm::vec3> vertNormals;
	std::vector<glm::vec2> texCoords;
	std::vector<unsigned int> fakeIndices;
	for (int i=0; i<num_tris*3; i++)
	{
		glm::vec3 n0;
		glm::vec2 t0;

		// incrementing base index if optional data exists [tex] [norm] vert []-means optional
		if ( txts.size() > 0 ) {
			t0 = txts[ indices[coordIdx++] ]; 
		}
		if ( norms.size() > 0 ) {
			n0 = norms[ indices[coordIdx++] ]; 
		}
		glm::vec3 v0 = verts[ indices[coordIdx++] ];


		//protowizard::Vertex_VNT vtx( v0, n0, t0 );
		//vertices.push_back(vtx);
		vertCoords.push_back(v0);
		vertNormals.push_back(n0);
		texCoords.push_back(t0);
		fakeIndices.push_back(i);
	}

	addMesh(vertCoords, vertNormals, texCoords, fakeIndices, active, asset_manager);
}

void ParseMesh::getVectors(FILE *fp,char *type, std::vector<glm::vec3>& vecs)
{
	int num;
	if(fscanf(fp,"%d",&num)!=1)
		throw std::runtime_error("Error: could not parse mesh (expected 'num_" + std::string(type) + "').");   
   
	for(int i=0;i<num;i++)
	{
		glm::vec3 v;
		if(fscanf(fp,"%f %f %f ",&v.x,&v.y,&v.z)!=3)
		{
			std::stringstream ss;
			ss << "Error: could not read " << num << type << "of mesh.";
			throw std::runtime_error(ss.str());
		}

		vecs.push_back(v);
	}
}

void ParseMesh::getTextureCoords(FILE *fp,char *texturename,std::vector<glm::vec2>& txts)
{
	int num_txts;
	if(fscanf(fp,"%d",&num_txts)!=1)
		throw std::runtime_error("Error: could not parse mesh (expected 'num_txts').");   

	fscanf(fp,"%s",texturename);
	for(int i=0;i<num_txts;i++)
	{
		glm::vec2 tex;
		if(fscanf(fp,"%f %f",&tex.x,&tex.y)!=2)
		{
			std::stringstream ss;
			ss << "Error: could not read " << num_txts << " texturecoords of mesh.";
			throw std::runtime_error(ss.str());   
		}	 
		tex.y *= -1.f; // Flip y-for GL
		txts.push_back( tex );
	}      
}

// reads indices for TexCoords, Normals and Vertices. Doesnt load actual coordinates.
void ParseMesh::getTriangles(FILE *fp,int *num_tris,std::vector<unsigned int>& indices, bool hasNorms, bool hasTexCoords)
{
	int num;
	int v[3],n[3],t[3];
   
	if(fscanf(fp,"%d",&num)!=1)
		throw std::runtime_error("Error: could not parse mesh (expected 'num_triangles').");   

	int i=0;
	for(int q=0;q<num;q++)
	{
		if(fscanf(fp,"%d %d %d",&v[0],&v[1],&v[2])!=3)
		{
			std::stringstream ss;
			ss << "Error: could not read " << num << " vertex indices of mesh.";
			throw std::runtime_error(ss.str());
		}
	
		if(hasNorms)
		{
			if(fscanf(fp,"%d %d %d",&n[0],&n[1],&n[2])!=3)
			{
				std::stringstream ss;
				ss << "Error: could not read " << num << " normal indices of mesh.";
				throw std::runtime_error(ss.str());  
			}
		}
      
		if(hasTexCoords)
		{
			if(fscanf(fp,"%d %d %d",&t[0],&t[1],&t[2])!=3)
			{
				std::stringstream ss;
				ss << "Error: could not read " << num << " texcoord indices of mesh.";
				throw std::runtime_error(ss.str());   
			}
		}
      
		/* indices appear in this order: [texture] [normals] vertices. []=optional */
		// [t0 n0] v0
		// [t1 n1] v1
		// [t2 n2] v2
		for(int w=0;w<3;w++)
		{
			if(hasTexCoords) 
				indices.push_back( t[w] );

			if(hasNorms) 
				indices.push_back( n[w] );

			indices.push_back( v[w] );
		}
		//printf("vv: %d\n",v[w]);
   }
   *num_tris=num;
}

void ParseMesh::addMesh(	const std::vector<glm::vec3> &vertCoords,
							const std::vector<glm::vec3> &vertNormals,
							const std::vector<glm::vec2> &texCoords,
							const std::vector<unsigned int> &indices, 
							File::BART::active_def &active,
							const File::AssetManagerPtr &asset_manager)
{
	if( active.sceneNode->name == "root" )
		throw std::runtime_error("Active SceneNode was root when parsing mesh!");

	auto mesh = std::make_shared<Scene::Mesh>( vertCoords, vertNormals, texCoords, indices );
	
	if ( active.texture != "" ) {
		Render::UniformPtr tex_sampler = std::make_shared<Render::Uniform>(Kernel::getSingleton()->getGBuffer()->getShader()->getFS(), "diffuse_tex");
		auto tex2d = asset_manager->getTex2DAbsolutePath( active.texture, true );
		Render::SamplerPtr dummy_sampler; // TODO
		//auto tex2d = asset_manager->getTex2DAbsolutePath( active.texture ); 
		//mesh->setTexture( tex2d, nullptr, nullptr );

		// TODO, load texture from. Save in a map.
		// could also just save all tex path's, then load all at once, and assign
		// Tex2DPtr's in end. 
		// active.texture
		mesh->setTexture( tex2d, tex_sampler, dummy_sampler ); 
	}

	mesh->setMaterial( active.extMaterial );
	active.sceneNode->addMesh( mesh );
}
