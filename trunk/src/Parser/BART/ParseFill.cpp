#include "ParseFill.h"

#include "..\..\Render\Material.h"

#include "..\..\File\BARTLoader2.h"

#include <stdexcept>

using namespace Parser;
using namespace BART;

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
void ParseFill::parse(FILE *fp, std::vector<Render::MaterialPtr> &materials, File::BART::active_def &active)
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
			throw std::runtime_error("fill material ambient syntax error");
		if(fscanf(fp,"%f %f %f",&dif.x, &dif.y, &dif.z) != 3)
			throw std::runtime_error("fill material diffuse syntax error");
		if(fscanf(fp,"%f %f %f",&spc.x, &spc.y, &spc.z) != 3)
			throw std::runtime_error("fill material specular syntax error");
		if (fscanf(fp, "%f %f %f", &phong_pow, &t, &ior) != 3)
			throw std::runtime_error("fill material (phong, transmittance, IOR) syntax error");

		/* add your extended material here
		* e.g., viAddExtendedMaterial(amb,dif,spc,4.0*phong_pow,t,ior);
		*/
		addMaterial( amb, dif, spc, phong_pow, t, ior, materials, active );
	}
	else   /* parse the old NFF description of a material */
	{
		if (fscanf(fp, "%f %f %f",&col.x, &col.y, &col.z) != 3)
			throw std::runtime_error("fill color syntax error");
		if (fscanf(fp, "%f %f %f %f %f", &kd, &ks, &phong_pow, &t, &ior) != 5)
			throw std::runtime_error("fill material syntax error");
       
		/* add the normal NFF material here 
		* e.g., viAddMaterial(col,kd,ks,4.0*phong_pow,t,ior);
		*/
		// TODO is it correct to use kd and ks just as attenuations?
		addMaterial( glm::vec3(kd), col, glm::vec3(ks), phong_pow, t, ior, materials, active );
	}
}

void ParseFill::addMaterial( const glm::vec3& amb, const glm::vec3& dif, const glm::vec3& spc,
	              float phong_pow, float transmittance, float ior, std::vector<Render::MaterialPtr> &materials, 
				  File::BART::active_def &active )
{
	Render::MaterialParams params( 0, amb, dif, spc, phong_pow, transmittance, ior );
	active.extMaterial = std::make_shared<Render::Material>( params );
	materials.push_back( active.extMaterial );
}