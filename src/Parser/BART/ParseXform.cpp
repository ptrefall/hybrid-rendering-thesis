#include "ParseXform.h"

#include "..\..\File\BARTLoader2.h"

#include <glm\ext.hpp>

#include <stdexcept>

using namespace Parser;
using namespace BART;

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
void ParseXform::parse(FILE *f, File::BART::active_def &active, const std::function<void(const std::string&, const glm::mat4&)> &pushNodeFunc)
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

		File::BART::eatWhitespace(f);
		ch=(char)getc(f);
		if(ch!='{')
			throw std::runtime_error("Error: { expected.");

		/* add a static transform here e.g.,viAddStaticXform(scale,rot,deg,trans); */
		active.xformName = std::string("static");

		// T*R*S
		glm::mat4 thisTransform = glm::mat4(1.0f);
		thisTransform = glm::translate( thisTransform, trans );
		thisTransform = glm::rotate( thisTransform, deg, rot );
		thisTransform = glm::scale( thisTransform, scale );

		active.xformTypeStack.push( File::BART::STATIC_TRANSFORM );
		active.xformStack.push( active.xformMatrix );
		active.xformMatrix *= thisTransform;

		pushNodeFunc( active.xformName, thisTransform);	
	}
	else   /* keyframe animated transform */
	{
		if(fscanf(f,"%s",name)!=1)
			throw std::runtime_error("Error: could not read transform name.");

		File::BART::eatWhitespace(f);
		ch=(char)getc(f);
		if(ch!='{')
			throw std::runtime_error("Error: { expected.");

		/* add an animated transform here
		* e.g., viAddXform(name);
		*/
		active.xformName = std::string(name);
		active.xformTypeStack.push( File::BART::ANIMATED_TRANSFORM );

		pushNodeFunc( active.xformName, glm::mat4(1.f) );	
	}
	
}

void ParseXform::end(File::BART::active_def &active, const std::function<void()> &popNodeFunc)
{
	if ( active.xformTypeStack.top() == File::BART::STATIC_TRANSFORM )
	{
		if ( active.xformStack.size() == 0 ) 
			throw std::runtime_error("no more to pop from matrix stack... will underflow...");

		active.xformMatrix = active.xformStack.top();
		active.xformStack.pop();
	}

	popNodeFunc();
	active.xformTypeStack.pop();
}
