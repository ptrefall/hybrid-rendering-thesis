#include "BARTScene.h"

#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#define _USE_MATH_DEFINES 
#include <math.h>
#include <cstdio>
#include <string>
#include <vector>
#include <stack>

#include "../src/animation.h"
#include "../src/texture.h"

#include <proto/protographics.h>
#include <proto/shapes/mesh.h>
#include <proto/vertex_types.h>

struct material_t
{
	glm::vec3 col;
};

struct material_ext_t
{
	glm::vec3 amb,dif,spc;
	float phong_pow;
	float transmittance;
	float ior;
};

struct sphere_t
{
	glm::vec3 pos;
	float radius;
	material_t mat;
};

struct cone_t
{
	glm::vec3 a,b;
	float r1,r2;
	material_t mat;
};

struct poly_t
{
	protowizard::MeshPtr mesh;
	glm::mat4 xform;
	material_t mat;
	std::string texture;
};

struct mesh_t
{
	protowizard::MeshPtr mesh;
	glm::mat4 xform;
	material_ext_t mat;
	std::string texture;
};

namespace tformtype
{
	enum eTransformType{ STATIC_TRANSFORM, ANIMATED_TRANSFORM };
}

class BARTSceneImplementation : public BARTScene
{
private:
	int gDetailLevel;
	protowizard::ProtoGraphicsPtr proto;
	std::string sceneFolder;
	std::string mainSceneFile;

	// Global scene parameters
	glm::vec3 bgcolor;
	AnimationList* mAnimations; // TODO. verify correctness

	struct camera_def
	{
		glm::vec3 from, target, up;
		float fov;
	} cam;

	// Loader/parser temporaries. Become stored into scene objects
	struct active_def
	{
		std::string tformName;
		glm::mat4 tformMatrix;
		std::stack<glm::mat4> tformStack;
		// Keep track of transform hiearchy so we know when to pop a static tform
		std::stack<tformtype::eTransformType> tformTypeStack;

		material_t material; // found in .nff-s
		material_ext_t extMaterial; // found in .aff-s
		std::string texture;

		std::vector<protowizard::Vertex_VNT> triangleVerts;
	} active;

	// Scene objects
	std::vector<sphere_t> sphereList;
	std::vector<cone_t> coneList;
	std::vector<poly_t> polyList;
	std::vector<mesh_t> meshList;

public:
BARTSceneImplementation( protowizard::ProtoGraphicsPtr proto, const std::string& sceneFolder, const std::string& mainSceneFile ) : 
	  gDetailLevel(0), proto(proto), sceneFolder(sceneFolder), mainSceneFile(mainSceneFile)
{
	// Root. needed for addPoly when no prior tform spec'd
	active.tformMatrix = glm::mat4(1.0f);
	//active.tformStack.push(active.tformMatrix);
}


virtual ~BARTSceneImplementation() {}

	
void addTexturedTrianglePatch( const std::string& texturename, glm::vec3* verts, glm::vec3* norms, glm::vec2* uv )
{
	active.texture = std::string(texturename);
	using protowizard::Vertex_VNT;
	active.triangleVerts.push_back(Vertex_VNT( verts[0], norms[0], uv[0]) );
	active.triangleVerts.push_back(Vertex_VNT( verts[1], norms[1], uv[1]) );
	active.triangleVerts.push_back(Vertex_VNT( verts[2], norms[2], uv[2]) );
}
void addTexturedTriangle( const std::string& texturename, glm::vec3* verts, glm::vec2* uv )
{
	active.texture = std::string(texturename);
	using protowizard::Vertex_VNT;
	glm::vec3 n = glm::normalize( glm::cross( verts[1]-verts[0], verts[2]-verts[0] ) );
	active.triangleVerts.push_back(Vertex_VNT( verts[0], n, uv[0]) );
	active.triangleVerts.push_back(Vertex_VNT( verts[1], n, uv[1]) );
	active.triangleVerts.push_back(Vertex_VNT( verts[2], n, uv[2]) );
}
void addPoly( std::vector<protowizard::Vertex_VNT>& vertices )
{
	protowizard::MeshPtr m = std::make_shared<protowizard::Mesh>( vertices );
	poly_t p = {m, active.tformMatrix, active.material, active.texture};
	polyList.push_back(p);
}
void addMesh( std::vector<protowizard::Vertex_VNT>& vertices )
{
	protowizard::MeshPtr m = std::make_shared<protowizard::Mesh>( vertices );
	mesh_t mesh_and_mat = {m, active.tformMatrix, active.extMaterial, active.texture};
	meshList.push_back( mesh_and_mat );
}

/*----------------------------------------------------------------------
  parseViewpoint()
  Description:
    "v"
    "from" Fx Fy Fz
    "at" Ax Ay Az
    "up" Ux Uy Uz
    "angle" angle
    "hither" hither
    "resolution" xres yres

  Format:

    v
    from %g %g %g
    at %g %g %g
    up %g %g %g
    angle %g
    hither %g
    resolution %d %d

  The parameters are:

    From:  the eye location in XYZ.
    At:    a position to be at the center of the image, in XYZ world
	   coordinates.  A.k.a. "lookat".
    Up:    a vector defining which direction is up, as an XYZ vector.
    Angle: in degrees, defined as from the center of top pixel row to
	   bottom pixel row and left column to right column.
           In AFF, if the width is different from the height, then
           we interpret the Angle as the FOV in the y-direction (from top to bottom),
           and we set the aspect ratio to width/height.
    Resolution: in pixels, in x and in y.

  Note that no assumptions are made about normalizing the data (e.g. the
  from-at distance does not have to be 1).  Also, vectors are not
  required to be perpendicular to each other.

  For all databases some viewing parameters are always the same:
    Yon is "at infinity."
    Aspect ratio is 1.0.

  A view entity must be defined before any objects are defined (this
  requirement is so that NFF files can be used by hidden surface machines).
----------------------------------------------------------------------*/
void parseViewpoint(FILE *fp)
{
   float hither;
   int resx;
   int resy;
	
   if(fscanf(fp, " from %f %f %f", &cam.from.x, &cam.from.y, &cam.from.z) != 3)
      goto fmterr;
   
   if(fscanf(fp, " at %f %f %f", &cam.target.x, &cam.target.y, &cam.target.z) != 3)
      goto fmterr;
   
   if(fscanf(fp, " up %f %f %f", &cam.up.x, &cam.up.y, &cam.up.z) != 3)
      goto fmterr;
   
   if(fscanf(fp, " angle %f", &cam.fov) != 1)
      goto fmterr;
   
   if(fscanf(fp, " hither %f", &hither) !=1)
      goto fmterr;
   if(hither<0.0001) hither=1.0f;
      
   if(fscanf(fp, " resolution %d %d", &resx, &resy) !=2)
      goto fmterr;
 
   /* init your view point here: 
    * e.g, viInitViewpoint(from, at, up, fov_angle, hither, resx, resy);
    */
  return;

 fmterr:
   printf("Parser view syntax error");
   exit(1);
}

/*----------------------------------------------------------------------
  parseLight()
  Positional light.  A light is defined by XYZ position and an optional color.
  For animated lights we need to be able to identify light sources. We do
  this by giving them a name.
  Description:
    "l" X Y Z [R G B]
    "la" name X Y Z [R G B]   #animated light
  Format:
    l %g %g %g [%g %g %g]
    la %s %g %g %g [%g %g %g]

    All light entities must be defined before any objects are defined (this
    requirement is so that NFF files can be used by hidden surface machines).
    Lights have a non-zero intensity of no particular value [this definition
    may change soon, with the addition of an intensity and/or color].
    The name of an animated light must not contain any white spaces.
----------------------------------------------------------------------*/
void parseLight(FILE *fp)
{
   glm::vec4 pos;
   glm::vec4 col;
   int num;
   int is_animated;
   char name[100];
   strcpy(name,"");

   is_animated = getc(fp);
   if(is_animated != 'a')
   {
      ungetc(is_animated, fp);
      is_animated=0;
   }

   if(is_animated)  /* if it's an animated light, read its name */
   {
      fscanf(fp,"%s",name);
   }

   if(fscanf(fp, "%f %f %f ",&pos.x, &pos.y, &pos.z) != 3)
   {
      printf("Light source position syntax error");
      exit(1);
   }
   pos.w=1.0f;

   /* read optional color of light */
   num=fscanf(fp, "%f %f %f ",&col.r, &col.g, &col.b);
   if(num==0)
   {
      col = glm::vec4(1.0f,1.0f,1.0f,1.0f);
   }
   else if(num!=3)
   {
      printf("Light source color syntax error");
      exit(1);
   }
   col.a=1.0f;

   /* add light source here:
    * e.g. viAddLight(name,pos,col);
    */
}


/*----------------------------------------------------------------------
	parseBackground()
	Background color.  A color is simply RGB with values between 0 and 1
	Description:
	"b" R G B

	Format:
	b %g %g %g

	If no background color is set, assume RGB = {0,0,0}.
----------------------------------------------------------------------*/
void parseBackground(FILE* f)
{
	if(fscanf(f, "%f %f %f",&bgcolor.r, &bgcolor.g, &bgcolor.b) != 3)
	{
		printf("background color syntax error");
		exit(1);
	}
}


/*----------------------------------------------------------------------
  parseFill()
  Fill color and shading parameters. 
  Description:
     "f" red green blue Kd Ks Shine T index_of_refraction
     "fm" amb_r amb_g amb_b
          diff_r diff_g diff_b
          spec_r spec_g spec_b
          Shine T index_of_refraction
  Format:
    f %g %g %g %g %g %g %g %g
    fm %g %g %g  %g %g %g  %g %g %g  %g %g %g

    RGB is in terms of 0.0 to 1.0.

    Kd is the diffuse component, Ks the specular, Shine is the Phong cosine
    power for highlights, T is transmittance (fraction of light passed per
    unit).  Usually, 0 <= Kd <= 1 and 0 <= Ks <= 1, though it is not required
    that Kd + Ks == 1.  Note that transmitting objects ( T > 0 ) are considered
    to have two sides for algorithms that need these (normally objects have
    one side).

    The "fm" (fill material) version (not part of NFF) is a simple
    extension of the material description: it involves RGB for
    the ambient, the diffuse, and the specular component (instead of RGB,
    Ks, Ld) plus Shine, T, and index_of_refraction.

    The fill color is used to color the objects following it until a new color
    is assigned.
----------------------------------------------------------------------*/
void parseFill(FILE *fp)
{
	float kd, ks, phong_pow, t, ior;
	glm::vec3 col;
	int moreparams;

	moreparams = getc(fp);
	if(moreparams != 'm')
	{
		ungetc(moreparams, fp);
		moreparams = 0;
	}

	if(moreparams)  /* parse the extended material description */
	{
		glm::vec3 amb,dif,spc;
		if(fscanf(fp,"%f %f %f",&amb.x, &amb.y, &amb.z) != 3)
		{
			printf("fill material ambient syntax error");
			exit(1);
		}
		if(fscanf(fp,"%f %f %f",&dif.x, &dif.y, &dif.z) != 3)
		{
			printf("fill material diffuse syntax error");
			exit(1);
		}
		if(fscanf(fp,"%f %f %f",&spc.x, &spc.y, &spc.z) != 3)
		{
			printf("fill material specular syntax error");
			exit(1);
		}
		if (fscanf(fp, "%f %f %f", &phong_pow, &t, &ior) != 3)
		{
			printf("fill material (phong, transmittance, IOR) syntax error");
			exit(1);
		}
		/* add your extended material here
		* e.g., viAddExtendedMaterial(amb,dif,spc,4.0*phong_pow,t,ior);
		*/
		active.extMaterial.amb = amb;
		active.extMaterial.dif = dif;
		active.extMaterial.spc = spc;
		active.extMaterial.phong_pow = phong_pow;
		active.extMaterial.transmittance = t;
		active.extMaterial.ior = ior;
	}
	else   /* parse the old NFF description of a material */
	{
		if (fscanf(fp, "%f %f %f",&col.x, &col.y, &col.z) != 3)
		{
			printf("fill color syntax error");
			exit(1);
		}
       
		if (fscanf(fp, "%f %f %f %f %f", &kd, &ks, &phong_pow, &t, &ior) != 5)
		{
			printf("fill material syntax error");
			exit(1);
		}
       
		/* add the normal NFF material here 
		* e.g., viAddMaterial(col,kd,ks,4.0*phong_pow,t,ior);
		*/
		active.material.col = col;
	}
}


/*----------------------------------------------------------------------
  parseCone()
  Cylinder or cone.  A cylinder is defined as having a radius and an axis
    defined by two points, which also define the top and bottom edge of the
    cylinder.  A cone is defined similarly, the difference being that the apex
    and base radii are different.  The apex radius is defined as being smaller
    than the base radius.  Note that the surface exists without endcaps.  The
    cone or cylinder description:

    "c"
    base.x base.y base.z base_radius
    apex.x apex.y apex.z apex_radius

  Format:
    c
    %g %g %g %g
    %g %g %g %g

    A negative value for both radii means that only the inside of the object is
    visible (objects are normally considered one sided, with the outside
    visible).  Note that the base and apex cannot be coincident for a cylinder
    or cone.
----------------------------------------------------------------------*/
void parseCone(FILE *fp)
{
   glm::vec3 base_pt;
   glm::vec3 apex_pt;
   float r0,r1;
       
   if(fscanf(fp, " %f %f %f %f %f %f %f %f", &base_pt.x,&base_pt.y, &base_pt.z,&r0,
	     &apex_pt.x,&apex_pt.y, &apex_pt.z, &r1) !=8)
    {
       printf("cylinder or cone syntax error\n");
       exit(1);
    }
    if(r0 < 0.0)
    {
       r0 = -r0;
       r1 = -r1;
    }

    cone_t cone = {base_pt, apex_pt, r0, r1, active.material};
	coneList.push_back( cone );
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
void parseSphere(FILE *fp)
{
    float radius;
    glm::vec3 center;
	
    if(fscanf(fp, "%f %f %f %f", &center.x, &center.y, &center.z, &radius) != 4)
    {
       printf("sphere syntax error");
       exit(1);
    }

	sphere_t sph = {center, radius, active.material};
	sphereList.push_back( sph );
}	

void calcNormals( std::vector<protowizard::Vertex_VNT>& vertices )
{
	if ( vertices.size() % 3 != 0 ) {
		throw "not all triangles?";
		return;
	}

	for (size_t i=0; i<vertices.size(); i+=3) {
		glm::vec3 &v0 = vertices[i+0].v;
		glm::vec3 &v1 = vertices[i+1].v;
		glm::vec3 &v2 = vertices[i+2].v;

		glm::vec3 v0v1 = v1-v0;
		glm::vec3 v0v2 = v2-v0;
		glm::vec3 n = glm::cross(v0v1, v0v2);

		vertices[i+0].n = n;
		vertices[i+1].n = n;
		vertices[i+2].n = n;
	}
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
void parsePoly(FILE *fp)
{
   std::vector< protowizard::Vertex_VNT > vertices;

   int ispatch = getc(fp);
   if(ispatch != 'p')
   {
      ungetc(ispatch, fp);
      ispatch = 0;
   }
   
   int nverts;
   if(fscanf(fp, "%d", &nverts) != 1)
		goto fmterr;
	   
   vertices.resize( nverts );

    /* read all the vertices */
    for(int q=0; q<nverts; q++)
    {
		glm::vec3 &vertPos = vertices[q].v;
		if(fscanf(fp, " %f %f %f",&vertPos.x, &vertPos.y, &vertPos.z) != 3)
			goto fmterr;
       
		if(ispatch)
		{
			glm::vec3 &vertNormal = vertices[q].n;
			if(fscanf(fp, " %f %f %f",&vertNormal.x, &vertNormal.y, &vertNormal.z) != 3)
				goto fmterr;
		}
    }

    if(ispatch)
    {
		//add a polygon patch here
		//e.g.,  viAddPolyPatch(nverts,verts,norms);	
		addPoly( vertices );
    }
    else
    {
		// add a polygon here
		// e.g., viAddPolygon(nverts,verts);
	
		if ( vertices.size() == 4 )
		{
			vertices.push_back( vertices[0] );
			vertices.push_back( vertices[2] );
		}
		if ( vertices.size() % 3 != 0 ) return;
		
		calcNormals( vertices );
		addPoly( vertices );
    }
	
    return;
fmterr:
    printf("polygon or patch syntax error\n");
    exit(1);
memerr:
    printf("can't allocate memory for polygon or patch\n");
    exit(1);
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
void parseInclude(FILE *fp)
{
	char filename[100];
	FILE *ifp;
	int detail_level;
	if(fscanf(fp,"%d %s",&detail_level,filename)!=2)
	{
		printf("Error: could not parse include.\n");
		exit(0);
	}

	if(detail_level<=gDetailLevel) /* skip file if our detail is less than the global detail */
	{
		if(ifp=fopen( (sceneFolder+"//"+filename).c_str() ,"r"))
		{
			viParseFile(ifp);  /* parse the file recursively */
			fclose(ifp);
		}
		else
		{
			printf("Error: could not open include file: <%s>.\n",filename);
			exit(1);
		}
	}
	else
	{
		printf("Skipping include file: %s\n",filename);
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
void parseDetailLevel(FILE *fp)
{
   if(fscanf(fp,"%d",&gDetailLevel)!=1)
   {
      printf("Error: could not parse detail level.\n");
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
void parseTexturedTriangle(FILE *fp)
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
	texName = sceneFolder + "//"+ texName.substr(0,texName.size()-3) + "dds";

	if ( texName != active.texture && active.triangleVerts.size() > 0 ) {
		addMesh( active.triangleVerts );
		active.triangleVerts.clear();
	}

	for(int q=0;q<3;q++)
	{
		if(fscanf(fp," %f %f %f",&verts[q].x,&verts[q].y,&verts[q].z)!=3)
			goto parseErr;

		if(is_patch)
		{
			if(fscanf(fp," %f %f %f",&norms[q].x,&norms[q].y,&norms[q].z)!=3)
			goto parseErr;
		}

		if(fscanf(fp," %f %f ",&uv[q].x,&uv[q].y)!=2)
			goto parseErr;
	}

	if(is_patch) {
		//add a textured triangle patch here e.g., viAddTexturedTriPatch(texturename,verts,norms,tu,tv);
		addTexturedTrianglePatch( texName, verts, norms, uv );
	}
	else {
		// add a textured triangle here e.g.,  viAddTexturedTriangle(texturename,verts,tu,tv);
		addTexturedTriangle( texName, verts, uv );
	}

	return;
   
	parseErr:
	printf("Error: could not parse textured triangle\n");
	exit(1);
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

void parseAnimatedTriangle(FILE *fp)
{
   int num_times;
   glm::vec3 *verts;
   glm::vec3 *norms;
   
   fscanf(fp,"%d",&num_times);
   float* times=(float*)malloc(sizeof(float)*num_times);
   verts=(glm::vec3*)malloc(sizeof(glm::vec3)*3*num_times);
   norms=(glm::vec3*)malloc(sizeof(glm::vec3)*3*num_times);

   for(int q=0;q<num_times;q++)
   {
      if(fscanf(fp," %f",&times[q])!=1)
	 goto parseErr;

      for(int w=0;w<3;w++)
      {
	 if(fscanf(fp," %f %f %f",&verts[q*3+w].x,&verts[q*3+w].y,&verts[q*3+w].z)!=3)
	    goto parseErr;
	 
      
	 if(fscanf(fp," %f %f %f",&norms[q*3+w].x,&norms[q*3+w].y,&norms[q*3+w].z)!=3)
	    goto parseErr;	 
      }
   }

   /* add a animated triangle here
    * e.g., viAddAnimatedTriangle(num_times,times,verts,norms); 
    */
   return;
   
 parseErr:
   printf("Error: could not parse animated triangle (tpa)\n");
   exit(1);

}

/*----------------------------------------------------------------------
  parseTextureStuff()
  Decide if we got a texture with starts with "t " or a 
  textured triangle (or tri patch), which starts with "tt"
  Currently, we removed the "t"
----------------------------------------------------------------------*/
void parseTextureStuff(FILE *fp)
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

static void eatWhitespace(FILE *f)
{
   char ch=getc(f);
   while(ch==' ' || ch=='\t' || ch=='\n' || ch=='\f' || ch=='\r')
      ch=getc(f);
   ungetc(ch,f);
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

void parseKeyFrames(FILE *fp)
{   
	char name[200];
	char motion[200];
	char ch;
	int  c;
	int visibility;
	int  ret, i, nKeyFrames;
	float time, x, y, z, angle, te, co, bi;
	PositionKey* PKeys;
	RotationKey* RKeys;
	Animation* animation;
	struct AnimationList* animationlist;

	if(fscanf(fp,"%s",name)!=1)
	{
		printf("Error: could not read name of animation.\n");
		exit(1);
	}
	eatWhitespace(fp);
	ch=getc(fp);
	if(ch!='{')
	{
		printf("Error: could not find a { in animation %s.\n",name);
		exit(1);
	}
   
	/* insert a new animation in the AnimationList */
	animationlist= 
	(struct AnimationList*) calloc( 1, sizeof(struct AnimationList)); // TODO was calloc( 1, sizeof(struct blah...)
   
	/* put the newly allocated a list somewhere,
		* e.g., 
		* animationlist->next = gScene.mAnimations;
		* gScene.mAnimations = animationlist;
		* animation = &(animationlist->animation);
		* gScene.mAnimations was our global list of animations
	*/
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
		{
			printf("Error: could not read name of motion or number of keyframes for animation.\n");
			exit(1);
		}
      
		if(nKeyFrames<4 && strcmp("visibility",motion))
		{ 
			printf("Error: there must be at least 4 keyframes for %s.\n",name);
			exit(1);
		}

		/* check whether the motion is a "transl" or a "rot" or a "scale" */
		if(strcmp(motion, "transl")==0)
		{
			PKeys = (PositionKey*) calloc(nKeyFrames, sizeof(PositionKey));
			for( i=0; i<nKeyFrames; i++ )
			{
				ret = fscanf(fp, " %f %f %f %f %f %f %f", &time, &x, &y, &z, 
				&te, &co, &bi);
				if(ret != 7)
				{
				printf("error in parsing translation keyframes for %s\n",
				animation->name);
				exit(1);
				}
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
					{
						printf("error in parsing rotation keyframes for %s\n",
						animation->name);
						exit(1);
					}
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
				{
					printf("error in parsing scale keyframes for %s\n",
					animation->name);
					exit(1);
				}
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
				{
				printf("error in parsing visibility keyframes for %s\n",
				animation->name);
				exit(1);
				}
				viskeys[i].time=time;
				viskeys[i].visibility=visibility;	    
			}

			animation->visibilities=viskeys;
			animation->numVisibilities+=nKeyFrames;
		}
		else
		{
			printf("Error: unknown keyframe type (%s). Must be transl, rot, or scale.\n",motion);
			exit(1);
		}
		eatWhitespace(fp);
	}   
	printf("finished parsing anim %s\n", name);
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
void parseXform(FILE *f)
{
	char name[100];
	char ch;
	int is_static;

	is_static = getc(f);
	if(is_static != 's')
	{
		ungetc(is_static, f);
		is_static=0;
	}

	if(is_static)  /* is the transform a static one ? */
	{
		glm::vec3 scale, trans, rot;
		float deg;
      
		if(fscanf(f," %f %f %f %f %f %f %f %f %f %f", &scale[0], &scale[1], &scale[2],
		&rot[0], &rot[1], &rot[2], &deg,
		&trans[0], &trans[1], &trans[2])!=10)
		{
			printf("Error: could not read static transform.\n");
			exit(1);
		}
		eatWhitespace(f);
		ch=getc(f);
		if(ch!='{')
		{
			printf("Error: { expected.\n");
			exit(1);
		} 

		/* add a static transform here
		* e.g.,viAddStaticXform(scale,rot,deg,trans);
		*/
		active.tformName = std::string("static");

		// T*R*S
		glm::mat4 thisTransform = glm::mat4(1.0f);
		thisTransform = glm::translate( thisTransform, trans );
		thisTransform = glm::rotate( thisTransform, deg, rot );
		thisTransform = glm::scale( thisTransform, scale );

		active.tformStack.push( active.tformMatrix );
		active.tformMatrix *= thisTransform;

		active.tformTypeStack.push( tformtype::STATIC_TRANSFORM );
	}
	else   /* keyframe animated transform */
	{
		if(fscanf(f,"%s",name)!=1)
		{
			printf("Error: could not read transform name.\n");
			exit(1);
		}
		eatWhitespace(f);
		ch=getc(f);
		if(ch!='{')
		{
			printf("Error: { expected.\n");
			exit(1);
		}
		/* add an animated transform here
		* e.g., viAddXform(name);
		*/
		printf("begin anim xform %s\n", name);
		active.tformName = std::string(name);
		active.tformTypeStack.push( tformtype::ANIMATED_TRANSFORM );
	}

}

void viEndXform()
{
	if ( active.tformTypeStack.top() == tformtype::STATIC_TRANSFORM )
	{
		if ( active.tformStack.size() == 0 ) 
		{
			puts("no more to pop from matrix stack... will underflow...");
			exit(0);
		}
		active.tformMatrix = active.tformStack.top();
		active.tformStack.pop();
	}
	active.tformTypeStack.pop();
}

/*----------------------------------------------------------------------
	parseComment()
	Description:
	"#" [ string ]
	or 
	"%" [ string ]

	Format:
	# [ string ]
	or
	% [ string ]

	As soon as a "#" (or "%") character is detected, the rest of the line is
	considered a comment. 
----------------------------------------------------------------------*/
void parseComment(FILE* f)
{
	char str[1000];
	fgets(str, 1000, f);
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
static void parseAnimParams(FILE *fp)
{
   float start,end;
   int num_frames;
   if(fscanf(fp,"%f %f %d",&start,&end,&num_frames)!=3)
   {
      printf("Error: could not parse animations parameters.\n");
      exit(1);
   }
   /* add animations parameters here
    * e.g., viSetupAnimParams(start,end,num_frames);
    */
   printf("AnimParams: %f %f %d\n", start, end, num_frames);
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
void parseA(FILE *f)
{
	char name[100];
	char ch;
	int is_ambient;

	is_ambient = getc(f);
	if(is_ambient != 'm')
	{
		ungetc(is_ambient, f);
		is_ambient=0;
	}

	if(is_ambient)  /* we got "am" = global ambient light */
	{
		glm::vec3 amb;
		if(fscanf(f,"%f %f %f",&amb.r,&amb.g,&amb.b)!=3)
		{
			printf("Error: could not parse ambient light (am).\n");
			exit(1);
		}

		/* set up your global ambient light here using amb */
	}
	else
	{
		parseAnimParams(f);
	}
}

void getVectors(FILE *fp,char *type,int *num_vecs,glm::vec3 **vecs)
{
   int num;
   glm::vec3 *v=NULL;

   if(fscanf(fp,"%d",&num)!=1)
   {
      printf("Error: could not parse mesh (expected 'num_%s').\n",type);   
      exit(1);
   }

   v=(glm::vec3*)malloc(sizeof(glm::vec3)*num);
   if(v==NULL)
   {
      printf("Error: could not allocate memory for vertices of mesh.\n");
      exit(1); 
   }
   
   for(int q=0;q<num;q++)
   {
      if(fscanf(fp,"%f %f %f ",&v[q].x,&v[q].y,&v[q].z)!=3)
      {
	 printf("Error: could not read %d %s of mesh.\n",num,type);
	 exit(1);      
      }
   }
   *vecs=v;
   *num_vecs=num;
}

void getTextureCoords(FILE *fp,char *texturename,int *num,glm::vec2 **vecs)
{
   int q;
   int num_txts;
   glm::vec2 *txts;
   if(fscanf(fp,"%d",&num_txts)!=1)
   {
      printf("Error: could not parse mesh (expected 'num_txts').\n");   
      exit(1);
   }
   txts=(glm::vec2*)malloc(sizeof(glm::vec2)*num_txts);
   if(txts==NULL)
   {
      printf("Error: could not allocate memory for texturecoords of mesh.\n");
      exit(1); 
   }
   fscanf(fp,"%s",texturename);
   for(q=0;q<num_txts;q++)
   {
      if(fscanf(fp,"%f %f",&txts[q].x,&txts[q].y)!=2)
      {
	 printf("Error: could not read %d texturecoords of mesh.\n",num_txts);
	 exit(1);      
      }	 
   }      
   *num=num_txts;
   *vecs=txts;
}

// reads indices for TexCoords, Normals and Vertices. Doesnt load actual coordinates.
void getTriangles(FILE *fp,int *num_tris,unsigned short **indices,
			 glm::vec3 *verts,glm::vec3 *norms,glm::vec2 *txts)
{
	int num;
	int allocsize;
	unsigned short *idx;
	int v[3],n[3],t[3];
   
	allocsize=3;
	if(norms) allocsize+=3;
	if(txts) allocsize+=3; // can texcoords have 3 coords? or is this just to simplify code?
   
	if(fscanf(fp,"%d",&num)!=1)
	{
		printf("Error: could not parse mesh (expected 'num_triangles').\n");   
		exit(1);      
	}

	idx=(unsigned short *)malloc(num*allocsize*sizeof(unsigned short));
	if(idx==NULL)
	{
		printf("Error: could not allocate memory for indices of mesh.\n");
		exit(1); 
	}

	int i=0;
	for(int q=0;q<num;q++)
	{
		if(fscanf(fp,"%d %d %d",&v[0],&v[1],&v[2])!=3)
		{
			printf("Error: could not read %d vertex indices of mesh.\n",num);
			exit(1);   
		}
	
		if(norms)
		{
			if(fscanf(fp,"%d %d %d",&n[0],&n[1],&n[2])!=3)
			{
				printf("Error: could not read %d set of normal indices of mesh.\n",num);
				exit(1);   
			}
		}
      
		if(txts)
		{
			if(fscanf(fp,"%d %d %d",&t[0],&t[1],&t[2])!=3)
			{
				printf("Error: could not read %d texturecoord indices of mesh.\n",num);
				exit(1);   
			}
		}
      
		/* indices appear in this order: [texture] [normals] vertices. []=optional */
		// [t0 n0] v0
		// [t1 n1] v1
		// [t2 n2] v2
		for(int w=0;w<3;w++)
		{
			if(txts) idx[i++]=t[w];
			if(norms) idx[i++]=n[w];
			idx[i++]=v[w];
		}
		//printf("vv: %d\n",v[w]);

   }
   *indices=idx;
   *num_tris=num;
}

void parseMesh(FILE *fp)
{
   char str[200];
   int num_verts = 0;
   int num_norms = 0;
   int num_txts = 0;
   int num_tris = 0;
   glm::vec3 *verts=NULL,*norms=NULL;
   glm::vec2 *txts=NULL;
   unsigned short *indices; // contains indices into txts, norms, verts arrays.
   char texturename[200];

   if(fscanf(fp,"%s",str)!=1)
   {
      printf("Error: could not parse mesh (could not find 'vertices').\n");
      exit(1);
   }
   if(strcmp(str,"vertices"))
   {
      printf("Error: could not parse mesh (expected 'vertices').\n");
      exit(1);
   }
   getVectors(fp,"vertices",&num_verts,&verts);

   fscanf(fp,"%s",str);
   if(!strcmp(str, "normals"))
   {
      getVectors(fp,"normals",&num_norms,&norms);
      fscanf(fp,"%s",str);
   }
   if(!strcmp(str, "texturecoords"))
   {
      getTextureCoords(fp,texturename,&num_txts,&txts);
      fscanf(fp,"%s",str);
	  active.texture = std::string(texturename);
	  active.texture = sceneFolder + "//"+ active.texture.substr(0,active.texture.size()-3) + "dds";
   } else {
	   active.texture = "";
   }
   
   if(!strcmp(str,"triangles"))
   {
      getTriangles(fp,&num_tris,&indices,verts,norms,txts);
   }
   else
   {
      printf("Error: expected 'triangles' in mesh.\n");
      exit(1);
   }
   /* add a mesh here
    * e.g.,viAddMesh(verts,num_verts,norms,num_norms,txts,num_txts,texturename,indices,num_tris);
    */

   // TODO use opengl indexed
	size_t coordIdx = 0;
	std::vector<protowizard::Vertex_VNT> vertices;
	for (size_t i=0; i<num_tris*3; i++){
		glm::vec3 n0;
		glm::vec2 t0;

		// incrementing base index if optional data exists [tex] [norm] vert []-means optional
		if ( num_txts > 0 ) {
			t0 = txts[ indices[coordIdx++] ]; 
		}
		if ( num_norms > 0 ) {
			n0 = norms[ indices[coordIdx++] ]; 
		}
		glm::vec3 v0 = verts[ indices[coordIdx++] ];
		protowizard::Vertex_VNT vtx( v0, n0, t0 );
		vertices.push_back(vtx);
	}

   	addMesh(vertices);
}


/*----------------------------------------------------------------------
  viParseFile()
  Description:
    parses the animation file
----------------------------------------------------------------------*/
bool viParseFile(FILE *f)
{
   char ch;

   while((ch=getc(f))!=EOF)
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
	 parseComment(f); /* ok */
	 break;
      case 'v':   /* view point */
	 parseViewpoint(f); /* ok */
	 break;
      case 'l':   /* light source */
	 parseLight(f); /* ok */
	 break;
      case 'b':   /* background color */
	 parseBackground(f); /* ok */
	 break;
      case 'f':   /* fill material */
	 parseFill(f); /* ok */
	 break;
      case 'c':   /* cylinder or cone */
	 parseCone(f); /* ok */
	 break;
      case 's':   /* sphere */
	 parseSphere(f); /* ok */
	 break;
      case 'p':   /* polygon or patch */
	 parsePoly(f);
	 break;
      case 'i':   /* include another file */
	 parseInclude(f);  /* ok */
	 break;
      case 'd':   /* detail level of file (used to exclude objects from rendering) */
	 parseDetailLevel(f); /* ok */
	 break;
      case 't':  /* textured triangle, or texture tripatch, or animated triangle */
	 parseTextureStuff(f);
	 break;
      case 'x':  /* transform */
	 parseXform(f);
	 break;
      case '}':
	 viEndXform();
	 break;
      case 'a':  /* global amb light or animation parameters */
	 parseA(f);
	 break;
      case 'k':  /* key frames for transform (or the camera) */
	 parseKeyFrames(f);
	 break;
      case 'm':  /* triangle mesh */
	 parseMesh(f);
	 break;
      default:    /* unknown */
	 printf("unknown NFF primitive code: %c\n",ch);
	 exit(1);
      }
   }
   return true;
}

virtual void loadScene()
{
	FILE* f = fopen( (sceneFolder+"//"+mainSceneFile).c_str(), "r");
		
	if ( f == nullptr ) {
		printf("could not open .aff file\n");
		exit(1);
	}
	

	if( !viParseFile(f) ) {
		printf("could not parse file\n");
		exit(1);
	}
	fclose(f);

	glm::vec3 eyeUp = glm::normalize(cam.up);
	glm::vec3 eyeFwd = glm::normalize( cam.target - cam.from );
	glm::vec3 eyeStrafe = glm::cross(eyeFwd,eyeUp);
	eyeFwd = glm::cross(eyeStrafe,eyeUp); // straighten
	

	//proto->getCamera()->setCameraBasis( eyeStrafe, eyeUp, eyeFwd );
	proto->getCamera()->lookAt( cam.from, cam.target, cam.up );
	proto->getCamera()->setFov( cam.fov );
	proto->getCamera()->setNearDist(0.01f);
	proto->getCamera()->setFarDist(100.f);
}

virtual void draw()
{
	proto->cls( bgcolor.r, bgcolor.g, bgcolor.b );

	// TODO. decouple materials and geometry. have a unordered_map of <material, geo> pairs? less material dupes.

	for(size_t i=0; i<sphereList.size(); i++){
		proto->setColor( sphereList[i].mat.col );
		proto->drawSphere( sphereList[i].pos, sphereList[i].radius );
	}
	for(size_t i=0; i<coneList.size(); i++){
		proto->setColor( coneList[i].mat.col );
		proto->drawCone( coneList[i].a, coneList[i].b, coneList[i].r1 );
	}
	for(size_t i=0; i<polyList.size(); i++){
		proto->setColor( polyList[i].mat.col * 3.f );
		glm::mat4 &xform = polyList[i].xform;
		proto->setOrientation( xform );
		proto->drawMesh( polyList[i].mesh, false );
	}
	for(size_t i=0; i<meshList.size(); ++i){
		mesh_t& m = meshList[i];
		material_ext_t& mat = m.mat;
		//proto->setMaterial( mat.amb, mat.dif, mat.spc, mat.phong_pow );
		proto->setColor( (mat.amb+mat.dif+mat.spc) );
		
		bool isTwoSided = mat.transmittance > 0.f;
		if ( isTwoSided ) {
			proto->setBlend(true);
			proto->setColor( glm::vec3(0.75f) );
			proto->setAlpha( 1.0f - 0.5f*mat.transmittance );
		} else {
			proto->setBlend(false);
		}

		if ( m.texture != "" )
		{
			proto->setTexture(m.texture);
		}
		glm::mat4 &xform = m.xform;
		proto->setOrientation( xform );
		proto->drawMesh( m.mesh, isTwoSided );
	}
	proto->setOrientation( glm::mat4(1.0f) );
	proto->setBlend(false);

	float speed = 0.25f;
	speed += proto->getMouseWheel() * 0.25f;
	if ( speed < 0.01f ) speed = 0.1f;

	proto->getCamera()->update( proto->keystatus(protowizard::KEY::LEFT), proto->keystatus(protowizard::KEY::RIGHT), proto->keystatus(protowizard::KEY::UP), proto->keystatus(protowizard::KEY::DOWN), (float)proto->getMouseX(), (float)proto->getMouseY(), proto->mouseDownLeft(), speed * proto->getMSPF() );

}


}; // end of class Implementation

BARTScene* BARTScene::create(protowizard::ProtoGraphicsPtr proto, const std::string& sceneFolder, const std::string& mainSceneFile)
{
	return new BARTSceneImplementation(proto, sceneFolder, mainSceneFile);
}

