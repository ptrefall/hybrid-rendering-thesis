#include "BARTLoader2.h"

#include "..\Parser\INIParser.h"
#include "BARTLoader\animation.h"
#include "..\Scene\Mesh.h"

#include "..\Parser\BART\ParseComment.h"
#include "..\Parser\BART\ParseViewpoint.h"
#include "..\Parser\BART\ParseLight.h"
#include "..\Parser\BART\ParseBackground.h"
#include "..\Parser\BART\ParseFill.h"
#include "..\Parser\BART\ParseCone.h"

#include <glm/glm.hpp>
#include "glm/ext.hpp"

#define _USE_MATH_DEFINES 
#include <math.h>

using namespace File;

BARTLoader2::BARTLoader2(const std::string &base_dir)
	: base_dir(base_dir)
{
}

std::vector<Scene::SceneNodePtr> BARTLoader2::load(const std::string& sceneFolder, const std::string& mainSceneFile)
{
	this->sceneFolder = sceneFolder;
	this->mainSceneFile = mainSceneFile;

	detailLevel = 0;
	mAnimations = nullptr;
	active.tformMatrix = glm::mat4(1.0f);

	pushNode("root");
	parseFile(base_dir + sceneFolder + mainSceneFile);

	flattenSceneGraph( sceneRoot, glm::mat4(1.f) );
	return sceneNodeList;
}

//////////////////////////////////////////////////////////////
// INTERNAL FUNCTIONS
//////////////////////////////////////////////////////////////

void BARTLoader2::pushNode(const std::string& name, const glm::mat4& localTransform ) 
{
	auto newNode = std::make_shared<InternalSceneNode>(name);

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


////////////
////////////
// PARSE OPERATORS
////////////
////////////

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
			Parser::BART::ParseFill::parse(f.get(), materialList); /* ok */
			active.extMaterial = materialList[materialList.size()-1];
			break;
		case 'c':   /* cylinder or cone */
			Parser::BART::ParseCone::parse(f.get(), active.extMaterial, coneList); /* ok */
			break;
		case 's':   /* sphere */
			parseSphere(f.get()); /* ok */
			break;
		case 'p':   /* polygon or patch */
			parsePoly(f.get());
			break;
		case 'i':   /* include another file */
			parseInclude(f.get());  /* ok */
			break;
		case 'd':   /* detail level of file (used to exclude objects from rendering) */
			parseDetailLevel(f.get()); /* ok */
			break;
		case 't':  /* textured triangle, or texture tripatch, or animated triangle */
			parseTextureStuff(f.get());
			break;
		case 'x':  /* transform */
			parseXform(f.get());
			break;
		case '}':
			endXform();
			break;
		case 'a':  /* global amb light or animation parameters */
			parseA(f.get());
			break;
		case 'k':  /* key frames for transform (or the camera) */
			parseKeyFrames(f.get());
			break;
		case 'm':  /* triangle mesh */
			parseMesh(f.get());
			break;
		default:    /* unknown */
			throw std::runtime_error("unknown NFF primitive code: " + ch);
		}
	}
}

/*----------------------------------------------------------------------
  parseSphere()
  Sphere.  A sphere is defined by a radius and center position:
  
    "s" center.x center.y center.z radius

Format:
    s %g %g %g %g

    If the radius is negative, then only the sphere's inside is visible
    (objects are normally considered one sided, with the outside visible).
----------------------------------------------------------------------*/
void BARTLoader2::parseSphere(FILE *fp)
{
    float radius;
    glm::vec3 center;
	
    if(fscanf(fp, "%f %f %f %f", &center.x, &center.y, &center.z, &radius) != 4)
       throw std::runtime_error("sphere syntax error");

	sphere_t sph = {center, radius, active.extMaterial};
	sphereList.push_back( sph );
}

/*----------------------------------------------------------------------
  parsePoly()
  Polygon.  A polygon is defined by a set of vertices.  With these databases,
    a polygon is defined to have all points coplanar.  A polygon has only
    one side, with the order of the vertices being counterclockwise as you
    face the polygon (right-handed coordinate system).  The first two edges
    must form a non-zero convex angle, so that the normal and side visibility
    can be determined. 
  Description:

    "p" total_vertices
    vert1.x vert1.y vert1.z
    [etc. for total_vertices vertices]

Format:
    p %d
    [ %g %g %g ] <-- for total_vertices vertices
----------------------------------------------------------------------
  Polygonal patch.  A patch is defined by a set of vertices and their normals.
    With these databases, a patch is defined to have all points coplanar.
    A patch has only one side, with the order of the vertices being
    counterclockwise as you face the patch (right-handed coordinate system).
    The first two edges must form a non-zero convex angle, so that the normal
    and side visibility can be determined.  Description:

    "pp" total_vertices
    vert1.x vert1.y vert1.z norm1.x norm1.y norm1.z
    [etc. for total_vertices vertices]

Format:
    pp %d
    [ %g %g %g %g %g %g ] <-- for total_vertices vertices
----------------------------------------------------------------------*/
void BARTLoader2::parsePoly(FILE *fp)
{
   std::vector<glm::vec3> vertCoords;
   std::vector<glm::vec3> vertNormals;
   std::vector<glm::vec2> texCoords;


   int ispatch = getc(fp);
   if(ispatch != 'p')
   {
      ungetc(ispatch, fp);
      ispatch = 0;
   }
   
   int nverts;
   if(fscanf(fp, "%d", &nverts) != 1)
		throw std::runtime_error("polygon or patch syntax error");
	
	vertCoords.resize( nverts );
	vertNormals.resize( nverts );
	texCoords.resize( nverts );

    /* read all the vertices */
    for(int q=0; q<nverts; q++)
    {
		glm::vec3 &vertPos = vertCoords[q];
		if(fscanf(fp, " %f %f %f",&vertPos.x, &vertPos.y, &vertPos.z) != 3)
			throw std::runtime_error("polygon or patch syntax error");
       
		if(ispatch)
		{
			glm::vec3 &vertNormal = vertNormals[q];
			if(fscanf(fp, " %f %f %f",&vertNormal.x, &vertNormal.y, &vertNormal.z) != 3)
				throw std::runtime_error("polygon or patch syntax error");
		}
    }

    if(ispatch)
    {
		//add a polygon patch here
		//e.g.,  viAddPolyPatch(nverts,verts,norms);	
		addPoly( vertCoords, vertNormals, texCoords );
    }
    else
    {
		// add a polygon here
		// e.g., viAddPolygon(nverts,verts);
	
		if ( vertCoords.size() == 4 )
		{
			// TODO: does this happen? yes. polygons require som strange parsing. need to
			// generate more faces. Look at Museum scene center for an example...
			vertCoords.push_back( vertCoords[0] );
			vertCoords.push_back( vertCoords[2] );

			texCoords.push_back( texCoords[0] );
			texCoords.push_back( texCoords[2] );
		}
		// Expecting triangles, so expect num verts to be divisable by 3.
		if( vertCoords.size() % 3 != 0 )
			throw std::runtime_error("polygon or patch vertex count was not dividable by 3!");
		
		calcNormals( vertCoords, vertNormals  );
		addPoly( vertCoords, vertNormals, texCoords );
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
	FILE *ifp;
	int detail_level;
	if(fscanf(fp,"%d %s",&detail_level,filename)!=2)
		std::runtime_error("Error: could not parse include.");
	
	auto includeName = std::string(filename);
	auto sceneNodeIt = active.includefileToNodeMap.find( includeName ); 
	
	auto it = active.includefileToNodeMap.find( includeName );
	if ( it != active.includefileToNodeMap.end() ) // seen before
	{
		InternalSceneNodePtr subtreeAtInc = (*sceneNodeIt).second;
		assert( subtreeAtInc->name == "null" );
		active.sceneNode->add( subtreeAtInc );
		recursiveSetMaterialState( active.sceneNode );
		//printf("attaching subtree to %s::%s\n", active.sceneNode->fileScope.c_str(), active.sceneNode->name.c_str() );
		//subtreeAtInc->visit(0);
	}
	else
	{
		if(detail_level<=detailLevel) /* skip file if our detail is less than the global detail */
		{
			active.fileScopeStack.push(includeName);
			pushNode("null");


			try {
				parseFile(base_dir+sceneFolder+filename);  /* parse the file recursively */
			} catch(std::exception &e) {
				throw std::runtime_error("Error: could not open include file: " + std::string(filename));
			}

			popNode();
			active.fileScopeStack.pop();
		}
		else
			std::cout << "Skipping include file: " << filename << std::endl;
	}
}

/*----------------------------------------------------------------------
  parseDetailLevel()
  Include another file (typically containing geometry)
  Description:  
    "d" detail_level

Format:
    d %d

    The detail level (DL) number is used to exclude objects
    from the scene so that different a scene can have different
    complexities (number of primitives in them).
    The include command (i) is
    the only one that have a detail number.
    If the detail level of an included file
    is less or equal to DL then that object is included, else
    we skip it. 
    Is 0 (zero) by default.
----------------------------------------------------------------------*/
void BARTLoader2::parseDetailLevel(FILE *fp)
{
   if(fscanf(fp,"%d",&detailLevel)!=1)
	   throw std::runtime_error("Error: could not parse detail level.");
}

/*----------------------------------------------------------------------
  parseTextureStuff()
  Decide if we got a texture with starts with "t " or a 
  textured triangle (or tri patch), which starts with "tt"
  Currently, we removed the "t"
----------------------------------------------------------------------*/
void BARTLoader2::parseTextureStuff(FILE *fp)
{
	int is_triangle=getc(fp);
	if(is_triangle=='t')
	{
		parseTexturedTriangle(fp);
	}
	else if(is_triangle=='p')
	{
		is_triangle=getc(fp);
		if(is_triangle=='a')    /*tpa = triangle, patch, animated */
		{
			parseAnimatedTriangle(fp);
		}
	}
	else
	{
		printf("Error: tt and ttp are valid codes (not t%c).\n",(char)is_triangle);
		exit(1);
	}
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
void BARTLoader2::parseTexturedTriangle(FILE *fp)
{
	glm::vec3 verts[3];
	glm::vec3 norms[3];
	glm::vec2 uv[3];
	char texturename[100];

	int is_patch=getc(fp);
	if(is_patch!='p')
	{
		ungetc(is_patch,fp);
		is_patch=0;
	}
   
	fscanf(fp,"%s",texturename);
	std::string texName = std::string(texturename);
	texName = base_dir + sceneFolder + texName.substr(0,texName.size()-3) + "dds";

	if ( texName != active.texture && active.triangleVertCoords.size() > 0 ) {
		// TODO make a triangle params struct mayhaps? With a ::clear method.
		addPoly( active.triangleVertCoords, active.triangleVertNormals, active.triangleTexCoords );
		active.triangleVertCoords.clear();
		active.triangleVertNormals.clear();
		active.triangleTexCoords.clear();
	}

	for(int q=0;q<3;q++)
	{
		if(fscanf(fp," %f %f %f",&verts[q].x,&verts[q].y,&verts[q].z)!=3)
			throw std::runtime_error("Error: could not parse textured triangle");

		if(is_patch)
		{
			if(fscanf(fp," %f %f %f",&norms[q].x,&norms[q].y,&norms[q].z)!=3)
				throw std::runtime_error("Error: could not parse textured triangle");
		}

		if(fscanf(fp," %f %f ",&uv[q].x,&uv[q].y)!=2)
			throw std::runtime_error("Error: could not parse textured triangle");
	}

	//add a textured triangle patch here e.g., viAddTexturedTriPatch(texturename,verts,norms,tu,tv);
	if(is_patch) addTexturedTrianglePatch( texName, verts, norms, uv );

	// add a textured triangle here e.g.,  viAddTexturedTriangle(texturename,verts,tu,tv);
	else addTexturedTriangle( texName, verts, uv );
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

void BARTLoader2::parseAnimatedTriangle(FILE *fp)
{
   int num_times;
   fscanf(fp,"%d",&num_times);

   float *times = new float[num_times];
   glm::vec3 *verts = new glm::vec3[3*num_times];
   glm::vec3 *norms = new glm::vec3[3*num_times];

	for(int q=0;q<num_times;q++)
	{
		if(fscanf(fp," %f",&times[q])!=1)
			throw std::runtime_error("Error: could not parse animated triangle (tpa)");

		for(int w=0;w<3;w++)
		{
			if(fscanf(fp," %f %f %f",&verts[q*3+w].x,&verts[q*3+w].y,&verts[q*3+w].z)!=3)
				throw std::runtime_error("Error: could not parse animated triangle (tpa)");
	 
			if(fscanf(fp," %f %f %f",&norms[q*3+w].x,&norms[q*3+w].y,&norms[q*3+w].z)!=3)
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

/*----------------------------------------------------------------------
  viParseXform()
  parse transform (either static or keyframe animated)
  Description:
  "xs" sx sy sz
       rx ry rz angle_deg
       tx ty tz
  {
    here follows objects, materials, new transforms, etc that
    are to be statically transformed with: T*R*S 
    i.e., first scaling (sx,sy,sz) then rotation of angle_deg degrees
    around the axis (rx,ry,rz), and finally translation (tx,ty,tz)
  }

  or

  "x" transform_name
  {
    here follows objects, materials, new transforms, etc that
    are animated
  }
  The actual keyframes must be found later in the file, and
  these are given with the "k" descriptor. 
  Everything inside the { } is transformed, and are thus
  in its own coordinate system (in a subtree).
----------------------------------------------------------------------*/
void BARTLoader2::parseXform(FILE *f)
{
	char name[100];
	char ch;

	int is_static = getc(f);
	if(is_static != 's') 
	{
		ungetc(is_static, f);
		is_static=0;
	}

	if(is_static) 
	{
		glm::vec3 scale, trans, rot;
		float deg;
      
		if(fscanf(f," %f %f %f %f %f %f %f %f %f %f",	&scale[0], &scale[1], &scale[2],
														&rot[0], &rot[1], &rot[2], &deg,
														&trans[0], &trans[1], &trans[2])!=10)
		{
			throw std::runtime_error("Error: could not read static transform.");
		}

		eatWhitespace(f);
		ch=(char)getc(f);
		if(ch!='{')
			throw std::runtime_error("Error: { expected.");

		/* add a static transform here e.g.,viAddStaticXform(scale,rot,deg,trans); */
		active.tformName = std::string("static");

		// T*R*S
		glm::mat4 thisTransform = glm::mat4(1.0f);
		thisTransform = glm::translate( thisTransform, trans );
		thisTransform = glm::rotate( thisTransform, deg, rot );
		thisTransform = glm::scale( thisTransform, scale );

		active.tformTypeStack.push( active_def::tformtype::STATIC_TRANSFORM );
		active.tformStack.push( active.tformMatrix );
		active.tformMatrix *= thisTransform;

		pushNode( active.tformName, thisTransform);	
	}
	else   /* keyframe animated transform */
	{
		if(fscanf(f,"%s",name)!=1)
			throw std::runtime_error("Error: could not read transform name.");

		eatWhitespace(f);
		ch=(char)getc(f);
		if(ch!='{')
			throw std::runtime_error("Error: { expected.");

		/* add an animated transform here
		* e.g., viAddXform(name);
		*/
		active.tformName = std::string(name);
		active.tformTypeStack.push( active_def::tformtype::ANIMATED_TRANSFORM );

		pushNode( active.tformName );	
	}
	
}
void BARTLoader2::endXform()
{
	if ( active.tformTypeStack.top() == active_def::tformtype::STATIC_TRANSFORM )
	{
		if ( active.tformStack.size() == 0 ) 
			throw std::runtime_error("no more to pop from matrix stack... will underflow...");

		active.tformMatrix = active.tformStack.top();
		active.tformStack.pop();
	}

	popNode();
	active.tformTypeStack.pop();
}

/*----------------------------------------------------------------------
  parseA()
  parse animation parameters and global ambient light

  Global ambient light description:  
    "am" red green blue

Format:
    am %g %g %d 

    There is one global ambient light source in the scene,
    and it can be set with, e.g., "am 0.5 0.5 0.5".
    Default value is "am 1.0 1.0 1.0".
----------------------------------------------------------------------*/
void BARTLoader2::parseA(FILE *f)
{
	int is_ambient = getc(f);
	if(is_ambient != 'm')
	{
		ungetc(is_ambient, f);
		is_ambient=0;
	}

	if(is_ambient)  /* we got "am" = global ambient light */
	{
		glm::vec3 amb;
		if(fscanf(f,"%f %f %f",&amb.r,&amb.g,&amb.b)!=3)
			throw std::runtime_error("Error: could not parse ambient light (am).\n");

		//TODO: set up your global ambient light here using amb
	}
	else
		parseAnimParams(f);
}

/*----------------------------------------------------------------------
  parseAnimParams()
  parse animation parameters
  Description:  
    "a" start_time end_time num_frames

Format:
    a %g %g %d 

    start_time indicates the start of the animation
    end_time   indicates the end of the animation
    num_frames is the number of frames in the animation.
      Note: the step time (from one frame to the next) is then
      (end_time-start_time)/(num_frames-1)
----------------------------------------------------------------------*/
void BARTLoader2::parseAnimParams(FILE *fp)
{
   float start,end;
   int num_frames;
   if(fscanf(fp,"%f %f %d",&start,&end,&num_frames)!=3)
      throw std::runtime_error("Error: could not parse animations parameters.");

   /* add animations parameters here e.g., viSetupAnimParams(start,end,num_frames); */
   setupAnimParams(start, end, num_frames);
}

/*----------------------------------------------------------------------
  parseKeyFrames()
  Format:
  "k" xform_name
  {
     type num_keyframes
     keyframe_data
  }

  Description:  
  The xform_name identifies the xform which is to be animated.

  For each type of keyframes (transl, rot, scale) there must
  be at least 4 keyframes, and the first and the last keyframes
  are only used internally to get starting tangents and similar stuff.
  There is also a fourth type of keyframes (visibility), which determines
  whether an object is visible or not at a certain time. This one needs
  at least one keyframe. 
  Each of the types may appear once in a keyframe description, i.e.,
  you can have a translation, scaling, rotation, and a visibility
  in the keyframe description (but not, say, two rotations).
  It is always the case that the total transform
  is T*R*S, where T=translation, R=rotation, and S=scaling.
  Is there is no, say, rotation, then R=I (identity matrix). This
  holds for the other as well.
  Note the order: the scaling is applied first, then rotation,
  the translation.

  Translation example:
  k the_ball
  {
    transl 5
    -0.50 0 -3.0 0  0 0 0
     0.00 0  0.0 0  0 0 0
     0.50 0  1.0 0  0 0 0
     1.00 0  0.0 0  0 0 0
     1.50 0 -3.0 0  0 0 0
  }
  In this example, we can only use times from 0.00 to 1.00
  Each row looks like this: time x y z tension continuity bias,
  where (x,y,z) is the translation at time, and tension, continuity, and
  bias, are the constants for interpolation at time.
 
  Rotation example:
  k the_ball2
  {
     rot 4
     -0.5  1 0 0 45  0 0 0
      0.0  1 0 0 0   0 0 0
      0.5  1 0 0 90  0 0 0
      1.0  0 1 1 10  0 0 0
  }
  Each row looks like this: time x y z degrees tension continuity bias, where  
  (x,y,z) is the rotation axis and degrees is the amound which is rotated
  around the axis at time, and tension, continuity, and
  bias, are the constants for interpolation at time.

  Scaling example:
  k the_ball3
  {
    scale 7
    -0.5 1 1 1 0 0 0
     0.0 1 1 1 0 0 0
     0.5 2 1 1 0 0 0
     1.0 1 2 1 0 0 0
     1.5 1 1 2 0 0 0
     2.0 1 1 1 0 0 0
     2.5 1 1 1 0 0 0
  }
  Each row looks like this: time x y z tension continuity bias, where  
  (x,y,z) is the scaling parameters, and tension, continuity, and
  bias, are the constants for interpolation at time.

  Visibility example:
  k the_ball4
  {
    visibility 2
    0.5 0
    2.0 1
  }
  Each row looks like this: time visbility_flag
  where visibility_flag is either 0 (invisible) or 1 (visible)
  From time=-infinity each object is assumed visible until
  the first visibility keyframe. At that time (0.5 in the example
  above) the visibility switches to what is given in that keyframe
  (0 in the example). At the next keyframe (time=2.0 in the example)
  the visibility may change again (changes to visible (1) above).
  The last visibility_flag determines the visibility until time=infinity.

  Note also that if the name of an animation is "camera", then
  the viewpoint should be animated after those key frames (only
  translation and rotation). Light sources can also be animated
  (only translation).
----------------------------------------------------------------------*/

void BARTLoader2::parseKeyFrames(FILE *fp)
{   
	char name[200];
	char motion[200];
	int  c;
	unsigned char visibility;
	int  ret, i, nKeyFrames;
	float time, x, y, z, angle, te, co, bi;
	PositionKey* PKeys;
	RotationKey* RKeys;
	Animation* animation;
	struct AnimationList* animationlist;

	if(fscanf(fp,"%s",name)!=1)
		throw std::runtime_error("Error: could not read name of animation.");

	eatWhitespace(fp);
	char ch=(char)getc(fp);
	if(ch!='{')
		throw std::runtime_error("Error: could not find a { in animation " + std::string(name));
   
	/* insert a new animation in the AnimationList */
	animationlist = (struct AnimationList*) calloc( 1, sizeof(struct AnimationList)); // TODO was calloc( 1, sizeof(struct blah...)
   
	/* put the newly allocated a list somewhere,
		* e.g., 
		* animationlist->next = gScene.mAnimations;
		* gScene.mAnimations = animationlist;
		* animation = &(animationlist->animation);
		* gScene.mAnimations was our global list of animations
	*/
	animationlist->next = mAnimations;
	mAnimations = animationlist;
	animation = &(animationlist->animation);

	animation->translations=NULL;
	animation->rotations=NULL;
	animation->scales=NULL;
	animation->name=(char *)malloc(sizeof(name)); 
	strcpy(animation->name,name);

	eatWhitespace(fp);
	while( (c = getc(fp)) != '}' )
	{
		ungetc(c, fp);
		if(fscanf(fp, "%s %d", motion, &nKeyFrames)!=2)
			throw std::runtime_error("Error: could not read name of motion or number of keyframes for animation.");
      
		if(nKeyFrames<4 && strcmp("visibility",motion))
			throw std::runtime_error("Error: there must be at least 4 keyframes for " + std::string(name));

		/* check whether the motion is a "transl" or a "rot" or a "scale" */
		if(strcmp(motion, "transl")==0)
		{
			PKeys = (PositionKey*) calloc(nKeyFrames, sizeof(PositionKey));
			for( i=0; i<nKeyFrames; i++ )
			{
				ret = fscanf(fp, " %f %f %f %f %f %f %f", &time, &x, &y, &z, 
				&te, &co, &bi);
				if(ret != 7)
					throw std::runtime_error("error in parsing translation keyframes for " + std::string(animation->name));

				PKeys[i].t = time;
				PKeys[i].P.x = x;
				PKeys[i].P.y = y;
				PKeys[i].P.z = z;
				PKeys[i].tension = te;
				PKeys[i].continuity = co;
				PKeys[i].bias = bi;
			}

			animation->translations = KB_PosInitialize(nKeyFrames, PKeys);
			free(PKeys);
		}
		else if(strcmp(motion, "rot")==0)
		{
			RKeys = (RotationKey*) calloc(nKeyFrames, sizeof(RotationKey));
			for( i=0; i<nKeyFrames; i++ )
			{
				ret = fscanf(fp," %f %f %f %f %f %f %f %f", &time, &x, &y, &z, 
				&angle, &te, &co, &bi);
				if(ret != 8)
					throw std::runtime_error("error in parsing rotation keyframes for " + std::string(animation->name));

				RKeys[i].t = time;
				RKeys[i].Rot.x = x;
				RKeys[i].Rot.y = y;
				RKeys[i].Rot.z = z;
				RKeys[i].Rot.angle = angle*M_PI/180.0;
				RKeys[i].tension = te;
				RKeys[i].continuity = co;
				RKeys[i].bias = bi;
			}
			animation->rotations = KB_RotInitialize(nKeyFrames, RKeys);
			free(RKeys);
		}
		else if(strcmp(motion, "scale")==0)
		{
			PKeys = (PositionKey*) calloc(nKeyFrames, sizeof(PositionKey));
			for( i=0; i<nKeyFrames; i++ )
			{
				ret = fscanf(fp, " %f %f %f %f %f %f %f", &time, &x, &y, &z, 
				&te, &co, &bi);
				if(ret != 7)
					throw std::runtime_error("error in parsing scale keyframes for " + std::string(animation->name));

				PKeys[i].t = time;
				PKeys[i].P.x = x;
				PKeys[i].P.y = y;
				PKeys[i].P.z = z;
				PKeys[i].tension = te;
				PKeys[i].continuity = co;
				PKeys[i].bias = bi;
			}
			animation->scales = KB_PosInitialize(nKeyFrames, PKeys);
			free(PKeys);
		}
		else if(strcmp(motion, "visibility")==0)
		{
			VisKey *viskeys=(VisKey*)  calloc(nKeyFrames, sizeof(VisKey));
			for( i=0; i<nKeyFrames; i++ )
			{ 	    
				ret = fscanf(fp, " %f %d", &time, &visibility);
				if(ret != 2)
					throw std::runtime_error("error in parsing visibility keyframes for " + std::string(animation->name));

				viskeys[i].time=time;
				viskeys[i].visibility=visibility;	    
			}
			animation->visibilities=viskeys;
			animation->numVisibilities+=nKeyFrames;
			free(viskeys);
		}
		else
			throw std::runtime_error("Error: unknown keyframe type (" + std::string(motion) + "). Must be transl, rot, or scale.");

		eatWhitespace(fp);
	}   
	std::cout << "finished parsing anim " << name << std::endl;
}

void BARTLoader2::parseMesh(FILE *fp)
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
	  active.texture = sceneFolder + "//"+ active.texture.substr(0,active.texture.size()-3) + "dds";
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

	addMesh(vertCoords, vertNormals, texCoords, fakeIndices);
}

////////
////////
// ADD OPERATORS
////////
////////

void BARTLoader2::addPoly( const std::vector<glm::vec3> &vertCoords,
              const std::vector<glm::vec3> &vertNormals,
              const std::vector<glm::vec2> &texCoords )
{
	// TODO
	//Scene::MeshPtr mesh = std::make_shared<Scene::Mesh>( vertCoords, vertNormals, texCoords );
	//poly_t p = {mesh, active.tformMatrix, active.extMaterial, active.texture};
	//polyList.push_back(p);
}

void BARTLoader2::addTexturedTrianglePatch( const std::string& texturename, glm::vec3* verts, glm::vec3* norms, glm::vec2* uv )
{
	active.texture = std::string(texturename);
	for ( int i=0; i<3; i++ )
	{
		active.triangleVertCoords.push_back( verts[i] );
		active.triangleVertNormals.push_back( norms[i] );
		active.triangleTexCoords.push_back( uv[i] );
	}
}

void BARTLoader2::addTexturedTriangle( const std::string& texturename, glm::vec3* verts, glm::vec2* uv )
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

void BARTLoader2::addMesh(	const std::vector<glm::vec3> &vertCoords,
							const std::vector<glm::vec3> &vertNormals,
							const std::vector<glm::vec2> &texCoords,
							const std::vector<unsigned int> &indices)
{
	if( active.sceneNode->name == "root" )
		throw std::runtime_error("Active SceneNode was root when parsing mesh!");

	auto mesh = std::make_shared<Scene::Mesh>( vertCoords, vertNormals, texCoords, indices );

	// TODO, load texture from. Save in a map.
	// could also just save all tex path's, then load all at once, and assign
	// Tex2DPtr's in end. 
	// active.texture
	mesh->setMaterial( active.extMaterial );
	active.sceneNode->addMesh( mesh );
}


///////
///////
// MISC OPERATORS
///////
///////

void BARTLoader2::calcNormals( const std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals )
{
	// Create dummy indices for ordered triangles.
	std::vector<unsigned int> indices;
	for (size_t i=0; i<vertices.size(); i++)
		indices.push_back(i);

	calcNormals(vertices, indices, normals );
}

void BARTLoader2::calcNormals(	const std::vector<glm::vec3> &vertices, 
								const std::vector<unsigned int> &indices, 
								std::vector<glm::vec3> &normals )
{
	if ( vertices.size() % 3 != 0 )
		throw std::runtime_error("not all triangles?");

	normals.resize( vertices.size() );
	for (int i=0; i<normals.size(); i++ )
		normals[i] = glm::vec3(0.f);

	for (size_t i=0; i<indices.size()/3; i++) 
	{
		const glm::vec3 &v0 = vertices[indices[3*i+0]];
		const glm::vec3 &v1 = vertices[indices[3*i+1]];
		const glm::vec3 &v2 = vertices[indices[3*i+2]];

		glm::vec3 n = glm::cross(v1 - v0, v2 - v0);
		n = glm::normalize(n);
		normals[indices[3*i+0]] += n;
		normals[indices[3*i+1]] += n;
		normals[indices[3*i+2]] += n;
	}

	for (int i=0; i<normals.size(); i++ )
		normals[i] = glm::normalize(normals[i]);
}

/* 
	needed for robots/city2.aff
	Since .aff files are only loaded once, load the state of a node associated
	with material state from that file
*/
void BARTLoader2::recursiveSetMaterialState( const InternalSceneNodePtr& node ) 
{
	if ( node->meshes.size() ) {
		recursiveSetMaterialState( node->children.front() );

		const Scene::MeshPtr& leaf = node->meshes.front();
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

void BARTLoader2::setupAnimParams( float start, float end, int num_frames ) 
{
	std::cout << "AnimParams start end numframes: " << start << ", " << end << ", " << num_frames << std::endl;
	anim.startTime = start;
	anim.endTime = end;
	anim.numkeys = num_frames;
}

void BARTLoader2::flattenSceneGraph( const InternalSceneNodePtr &node, const glm::mat4 &parentXform )
{
	glm::mat4 combinedXform = parentXform * node->tform; // local xform then global xform
	for(auto it=begin(node->meshes); it!=end(node->meshes); ++it )
	{
		auto &pMesh = *it;

		// copy ctor is less obvious imo... but, saves having to write getter's.
		//Scene::MeshPtr finalMesh = std::shared_ptr<Scene::Mesh>( new Scene::Mesh( pMesh->getVao(), pMesh->getVbo(), pMesh->getIbo() ) );
		auto finalMesh = std::make_shared<Scene::Mesh>(*pMesh.get());
		finalMesh->setModelMatrix( combinedXform ); // still need to use global xform.
		sceneNodeList.push_back( finalMesh );
	}

	for(auto it=begin(node->children); it!=end(node->children); ++it )
		flattenSceneGraph( *it, combinedXform );
}


///////
///////
// GET OPERATORS
///////
///////

void BARTLoader2::getVectors(FILE *fp,char *type, std::vector<glm::vec3>& vecs)
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

void BARTLoader2::getTextureCoords(FILE *fp,char *texturename,std::vector<glm::vec2>& txts)
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
		txts.push_back( tex );
	}      
}

// reads indices for TexCoords, Normals and Vertices. Doesnt load actual coordinates.
void BARTLoader2::getTriangles(FILE *fp,int *num_tris,std::vector<unsigned int>& indices, bool hasNorms, bool hasTexCoords)
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


///////
///////
// MISC STATIC FUNCTIONALITY
///////
///////

void BARTLoader2::eatWhitespace(FILE *f)
{
   char ch=getc(f);
   while(ch==' ' || ch=='\t' || ch=='\n' || ch=='\f' || ch=='\r')
      ch=getc(f);
   ungetc(ch,f);
}


////////////////////////////////////////////////////
////////////////////////////////////////////////////
// INTERNAL SCENE NODE
////////////////////////////////////////////////////
////////////////////////////////////////////////////

BARTLoader2::InternalSceneNode::InternalSceneNode(const std::string& name) 
	: name(name)
{
}

void BARTLoader2::InternalSceneNode::add( InternalSceneNodePtr child ) 
{
	children.push_back(child);
}

void BARTLoader2::InternalSceneNode::addMesh( Scene::MeshPtr& m ) 
{
	meshes.push_back(m);
}

void BARTLoader2::InternalSceneNode::visit(int spaces) 
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

	if ( meshes.size() ) 
		std::cout << " has geo ";
	std::cout << std::endl;

	for (auto it=children.begin(); it!=children.end(); ++it)
		(*it)->visit(spaces+1);
}