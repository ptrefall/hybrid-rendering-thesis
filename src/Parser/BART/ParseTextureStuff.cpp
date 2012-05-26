#include "ParseTextureStuff.h"

#include "ParsePoly.h"

#include "..\..\File\BARTLoader2.h"

#include <glm\ext.hpp>

#include <stdexcept>

using namespace Parser;
using namespace BART;

/*----------------------------------------------------------------------
  parseTextureStuff()
  Decide if we got a texture with starts with "t " or a 
  textured triangle (or tri patch), which starts with "tt"
  Currently, we removed the "t"
----------------------------------------------------------------------*/

void ParseTextureStuff::parse(FILE* f, const std::string &base_dir, const std::string &sceneFolder, File::BART::active_def &active)
{
	int is_triangle = getc(f);
	if(is_triangle=='t')
	{
		parseTexturedTriangle(f, base_dir, sceneFolder, active);
	}
	else if(is_triangle=='p')
	{
		is_triangle=getc(f);
		if(is_triangle=='a')    /*tpa = triangle, patch, animated */
		{
			parseAnimatedTriangle(f);
		}
	}
	else
		throw std::runtime_error("Error: tt and ttp are valid codes (not t" + (char)is_triangle);
}

/*----------------------------------------------------------------------
  parseTexturedTriangle()
  A triangle with texture coordinates at each vertex.
  Can also be a textured triangle patch (with normals at each vertex).
  Description:  
    "tt" texturename
         vert0.x vert0.y vert0.z texcoord0.u texcoord0.v
         vert1.x vert1.y vert1.z texcoord1.u texcoord1.v
         vert2.x vert2.y vert2.z texcoord2.u texcoord2.v
    "ttp" texturename
          vert0.x vert0.y vert0.z norm0.x norm0.y norm0.z texcoord0.u texcoord0.v
          vert1.x vert1.y vert1.z norm1.x norm1.y norm1.z texcoord1.u texcoord1.v
          vert2.x vert2.y vert2.z norm2.x norm2.y norm2.z texcoord2.u texcoord2.v


Format:
    tt %s
       %g %g %g %g %g
       %g %g %g %g %g
       %g %g %g %g %g

    ttp %s 
        %g %g %g %g %g %g %g %g
        %g %g %g %g %g %g %g %g
        %g %g %g %g %g %g %g %g

    The texture name may not include any white spaces.
    Note that the texturing works like OpenGL REPEAT mode.
----------------------------------------------------------------------*/
void ParseTextureStuff::parseTexturedTriangle(FILE *f, std::string base_dir, std::string sceneFolder, File::BART::active_def &active)
{
	glm::vec3 verts[3];
	glm::vec3 norms[3];
	glm::vec2 uv[3];
	char texturename[100];

	int is_patch = getc(f);
	if(is_patch!='p')
	{
		ungetc(is_patch,f);
		is_patch=0;
	}
   
	fscanf(f,"%s",texturename);
	std::string texName = std::string(texturename);
	texName = base_dir + sceneFolder + texName.substr(0,texName.size()-3) + "dds";

	if ( texName != active.texture && active.triangleVertCoords.size() > 0 ) {
		// TODO make a triangle params struct mayhaps? With a ::clear method.
		Parser::BART::ParsePoly::addPoly( active.triangleVertCoords, active.triangleVertNormals, active.triangleTexCoords );
		active.triangleVertCoords.clear();
		active.triangleVertNormals.clear();
		active.triangleTexCoords.clear();
	}

	for(int q=0;q<3;q++)
	{
		if(fscanf(f," %f %f %f",&verts[q].x,&verts[q].y,&verts[q].z)!=3)
			throw std::runtime_error("Error: could not parse textured triangle");

		if(is_patch)
		{
			if(fscanf(f," %f %f %f",&norms[q].x,&norms[q].y,&norms[q].z)!=3)
				throw std::runtime_error("Error: could not parse textured triangle");
		}

		if(fscanf(f," %f %f ",&uv[q].x,&uv[q].y)!=2)
			throw std::runtime_error("Error: could not parse textured triangle");
	}

	//add a textured triangle patch here e.g., viAddTexturedTriPatch(texturename,verts,norms,tu,tv);
	if(is_patch) addTexturedTrianglePatch( texName, verts, norms, uv, active );

	// add a textured triangle here e.g.,  viAddTexturedTriangle(texturename,verts,tu,tv);
	else addTexturedTriangle( texName, verts, uv, active );
}

/*----------------------------------------------------------------------
  parseAnimatedTriangle()
  an animated triangle patch
  Description:  
    "tpa" texture_name filename

Format:
    tpa %d
        %g
        %g %g %g  %g %g %g 
        %g %g %g  %g %g %g 
        %g %g %g  %g %g %g 
	%g
        %g %g %g  %g %g %g 
        %g %g %g  %g %g %g 
        %g %g %g  %g %g %g 
        .
        .
        .
       
    tpa num_times
        time0
        vert0_time0.x vert0_time0.y vert0_time0.z norm0_time0.x norm0_time0.y norm0_time0.y 
        vert1_time0.x vert1_time0.y vert1_time0.z norm1_time0.x norm1_time0.y norm1_time0.y 
        vert2_time0.x vert2_time0.y vert2_time0.z norm2_time0.x norm2_time0.y norm2_time0.y 
        time1
        vert0_time1.x vert0_time1.y vert0_time1.z norm0_time1.x norm0_time1.y norm0_time1.y 
        vert1_time1.x vert1_time1.y vert1_time1.z norm1_time1.x norm1_time1.y norm1_time1.y 
        vert2_time1.x vert2_time1.y vert2_time1.z norm2_time1.x norm2_time1.y norm2_time1.y 
        .
        .
        .

	
   Definition: this animated triangle patch depends on the time;
     1) if time<time0 then use the vertices and normals from time0,
        i.e., use the first triangle patch in the list
     2) if time>time_{num_times-1} then use the vertices and normals
        from time_{num_times-1}, i.e., the last triangle patch in
        the list.
     3) otherwise find two subsequent triangle patches with times time_a
        and time_b, such that time_a <= time <= time_b. Then interpolate
        linearly between these two triangle patches to find the 
        animated triangle patch. See viGetAnimatedTriangle() in render.c
----------------------------------------------------------------------*/

void ParseTextureStuff::parseAnimatedTriangle(FILE *f)
{
   int num_times;
   fscanf(f,"%d",&num_times);

   float *times = new float[num_times];
   glm::vec3 *verts = new glm::vec3[3*num_times];
   glm::vec3 *norms = new glm::vec3[3*num_times];

	for(int q=0;q<num_times;q++)
	{
		if(fscanf(f," %f",&times[q])!=1)
			throw std::runtime_error("Error: could not parse animated triangle (tpa)");

		for(int w=0;w<3;w++)
		{
			if(fscanf(f," %f %f %f",&verts[q*3+w].x,&verts[q*3+w].y,&verts[q*3+w].z)!=3)
				throw std::runtime_error("Error: could not parse animated triangle (tpa)");
	 
			if(fscanf(f," %f %f %f",&norms[q*3+w].x,&norms[q*3+w].y,&norms[q*3+w].z)!=3)
				throw std::runtime_error("Error: could not parse animated triangle (tpa)");
		}
	}

	//TODO: Use these variables for something?

	delete[] times;
	delete[] verts;
	delete[] norms;

   /* add a animated triangle here
    * e.g., viAddAnimatedTriangle(num_times,times,verts,norms); 
    */
}

void ParseTextureStuff::addTexturedTrianglePatch( const std::string& texturename, glm::vec3* verts, glm::vec3* norms, glm::vec2* uv, File::BART::active_def &active )
{
	active.texture = std::string(texturename);
	for ( int i=0; i<3; i++ )
	{
		active.triangleVertCoords.push_back( verts[i] );
		active.triangleVertNormals.push_back( norms[i] );
		active.triangleTexCoords.push_back( uv[i] );
	}
}

void ParseTextureStuff::addTexturedTriangle( const std::string& texturename, glm::vec3* verts, glm::vec2* uv, File::BART::active_def &active )
{
	active.texture = std::string(texturename);
	glm::vec3 n = glm::normalize( glm::cross( verts[1]-verts[0], verts[2]-verts[0] ) );
	for ( int i=0; i<3; i++ )
	{
		active.triangleVertCoords.push_back( verts[i] );
		active.triangleVertNormals.push_back( n );
		active.triangleTexCoords.push_back( uv[i] );
	}
}
