#include "ParseA.h"

#include "..\..\File\BARTLoader2.h"

#include <glm\glm.hpp>
#include <iostream>

using namespace Parser;
using namespace BART;

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
void ParseA::parse(FILE *f, File::BART::anim_def &anim)
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
		parseAnimParams(f, anim);
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
void ParseA::parseAnimParams(FILE *fp, File::BART::anim_def &anim)
{
   float start,end;
   int num_frames;
   if(fscanf(fp,"%f %f %d",&start,&end,&num_frames)!=3)
      throw std::runtime_error("Error: could not parse animations parameters.");

   /* add animations parameters here e.g., viSetupAnimParams(start,end,num_frames); */
   setupAnimParams(start, end, num_frames, anim);
}

void ParseA::setupAnimParams( float start, float end, int num_frames, File::BART::anim_def &anim ) 
{
	std::cout << "AnimParams start end numframes: " << start << ", " << end << ", " << num_frames << std::endl;
	anim.startTime = start;
	anim.endTime = end;
	anim.numkeys = num_frames;
}
