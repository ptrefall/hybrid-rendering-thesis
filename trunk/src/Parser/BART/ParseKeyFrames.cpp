#include "ParseKeyFrames.h"

#include "..\..\File\BARTLoader2.h"
#include "..\..\File\BARTLoader\animation.h"

#include <iostream>
#define _USE_MATH_DEFINES 
#include <math.h>

using namespace Parser;
using namespace BART;

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

void ParseKeyFrames::parse(FILE *f, AnimationList *&mAnimations)
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

	if(fscanf(f,"%s",name)!=1)
		throw std::runtime_error("Error: could not read name of animation.");

	File::BART::eatWhitespace(f);
	char ch=(char)getc(f);
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

	File::BART::eatWhitespace(f);
	while( (c = getc(f)) != '}' )
	{
		ungetc(c, f);
		if(fscanf(f, "%s %d", motion, &nKeyFrames)!=2)
			throw std::runtime_error("Error: could not read name of motion or number of keyframes for animation.");
      
		if(nKeyFrames<4 && strcmp("visibility",motion))
			throw std::runtime_error("Error: there must be at least 4 keyframes for " + std::string(name));

		/* check whether the motion is a "transl" or a "rot" or a "scale" */
		if(strcmp(motion, "transl")==0)
		{
			PKeys = (PositionKey*) calloc(nKeyFrames, sizeof(PositionKey));
			for( i=0; i<nKeyFrames; i++ )
			{
				ret = fscanf(f, " %f %f %f %f %f %f %f", &time, &x, &y, &z, 
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
				ret = fscanf(f," %f %f %f %f %f %f %f %f", &time, &x, &y, &z, 
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
				ret = fscanf(f, " %f %f %f %f %f %f %f", &time, &x, &y, &z, 
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
				ret = fscanf(f, " %f %d", &time, &visibility);
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

		File::BART::eatWhitespace(f);
	}   
	std::cout << "finished parsing anim " << name << std::endl;
}
