#include <stdlib.h>
#include <stdio.h>
#include <math.h> 

#include <string>
#include <iostream>
#include <memory>

#define GLFW_NO_GLU
#define GLFW_INCLUDE_GL3
#include <GL/glfw3.h>
#include <GL3/gl3w.h>
#include <Optix/optixu/optixpp_namespace.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <Render/Tex2D.h>
#include <Render/Shader.h>
#include <Render/PBO.h>
#include <Scene/Quad.h>
#include <Scene/proto_camera.h>

#include "commonStructs.h"


// screen, buffer and tex all same size
const static int TEX_WIDTH = 512;
const static int TEX_HEIGHT =384;


void ang2mat(float hang,float vang,float mat[9])
{
   mat[0] = cos(hang); mat[1] = 0; mat[2]=-sin(hang);
   mat[6] = cos(vang)*sin(hang); mat[7] = sin(vang); mat[8]=cos(vang)*cos(hang);
   mat[3] = mat[7]*mat[2] - mat[8]*mat[1];
   mat[4] = mat[8]*mat[0] - mat[6]*mat[2];
   mat[5] = mat[6]*mat[1] - mat[7]*mat[0];
}

class OptixScene
{
public:
	struct CamVars
	{
		optix::Variable eyePos;
		optix::Variable u;
		optix::Variable v;
		optix::Variable w;
	} optixCamVars;

	OptixScene(std::string optix_dir, int width, int height) :
	  optix_dir(optix_dir), width(width), height(height)
	{
		init();
		createInstances( createGeometry(), createMaterial() );
	}

	void init()
	{
		/* Context */
		context = optix::Context::create();
		context->setRayTypeCount(2); /* shadow and radiance */
		context->setEntryPointCount(1);
		optix::Variable out_buffer = context->declareVariable("output_buffer");
		optix::Variable light_buffer = context->declareVariable("lights");
		context->declareVariable("max_depth")->setInt(10);
		context->declareVariable("radiance_ray_type")->setUint(0u);
		context->declareVariable("shadow_ray_type")->setUint(1u);
		context->declareVariable("scene_epsilon")->setFloat(1.e-4f);

		/* Lights buffer */
		BasicLight light;
		light.color.x = 0.9f;
		light.color.y = 0.9f;
		light.color.z = 0.9f;
		light.pos.x   = 0.0f;
		light.pos.y   = 20.0f;
		light.pos.z   = 20.0f;
		light.casts_shadow = 1;
		light.padding      = 0u;

		optix::Buffer light_buffer_obj = context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_USER, sizeof(BasicLight) );
		light_buffer_obj->setElementSize( sizeof(BasicLight) );
		light_buffer_obj->setSize(1);
		void* light_buffer_data = light_buffer_obj->map();
		((BasicLight*)light_buffer_data)[0] = light;
		light_buffer_obj->unmap();
		light_buffer->set(light_buffer_obj);

		/* Ray gen program */
		std::string path_to_ptx = optix_dir + "\\pinhole_camera.cu.ptx";
		optix::Program ray_gen_program = context->createProgramFromPTXFile( path_to_ptx, "pinhole_camera" );
		optixCamVars.eyePos = ray_gen_program->declareVariable("eye"); 
		optixCamVars.u = ray_gen_program->declareVariable("U"); 
		optixCamVars.v = ray_gen_program->declareVariable("V");
		optixCamVars.w = ray_gen_program->declareVariable("W");

		unsigned int screenDims[] = {width,height};
		ray_gen_program->declareVariable("rtLaunchDim")->set2uiv(screenDims);
		fTime = ray_gen_program->declareVariable("fTime");
		fTime->setFloat( (float)glfwGetTime() );
		context->setRayGenerationProgram(0, ray_gen_program);

		/* Miss program */
		path_to_ptx = optix_dir + "\\constantbg.cu.ptx";
		optix::Program miss_program = context->createProgramFromPTXFile( path_to_ptx, "miss" );
		glm::vec3 missColor(.3f, 0.1f, 0.2f);
		miss_program->declareVariable("bg_color")->set3fv(&missColor.x);
		context->setMissProgram(0, miss_program );

		/* Create shared GL/CUDA PBO */
		int element_size = 4 * sizeof(char);
		pbo = new Render::PBO(element_size * width * height, GL_STREAM_DRAW, true);
		//out_buffer_obj = context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_UNSIGNED_BYTE4, width, height);
		out_buffer_obj = context->createBufferFromGLBO(RT_BUFFER_OUTPUT, pbo->getHandle() );
		out_buffer_obj->setFormat(RT_FORMAT_UNSIGNED_BYTE4);
		out_buffer_obj->setSize(width,height);
		
		out_buffer->set(out_buffer_obj);

		
		fps_camera.lookAt( glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f) );
	}

	optix::Geometry createGeometry()
	{
		float     box_min[3];
		float     box_max[3];
		box_min[0] = box_min[1] = box_min[2] = -0.5f;
		box_max[0] = box_max[1] = box_max[2] =  0.5f;
		optix::Geometry box_geo = context->createGeometry();
		box_geo->setPrimitiveCount(1u);

		std::string path_to_ptx = optix_dir + "\\box.cu.ptx";
		optix::Program box_bounding_box_program = context->createProgramFromPTXFile( path_to_ptx, "box_bounds" );
		optix::Program box_intersection_program = context->createProgramFromPTXFile( path_to_ptx, "box_intersect" );
		box_geo->setBoundingBoxProgram(box_bounding_box_program);
		box_geo->setIntersectionProgram(box_intersection_program);
		box_geo->declareVariable("boxmin")->set3fv(box_min);
		box_geo->declareVariable("boxmax")->set3fv(box_max);

		return box_geo;
	}
	
	optix::Material createMaterial()
	{
		/* Create our hit programs to be shared among all materials */
		std::string path_to_ptx = optix_dir + "\\phong.cu.ptx";
		optix::Program closest_hit_program = context->createProgramFromPTXFile( path_to_ptx, "closest_hit_radiance" );
		optix::Program any_hit_program = context->createProgramFromPTXFile( path_to_ptx, "any_hit_shadow" );

		optix::Material mat = context->createMaterial();
	   /* Note that we are leaving anyHitProgram[0] and closestHitProgram[1] as NULL.
		 * This is because our radiance rays only need closest_hit and shadow rays only
		 * need any_hit */
		mat->setClosestHitProgram(0, closest_hit_program);
		mat->setAnyHitProgram(1, any_hit_program);
		return mat;
	}

	void createInstances(optix::Geometry box, optix::Material material)
	{
		static const int NUM_BOXES = 32;
		transforms.resize(NUM_BOXES);

		for ( int i = 0; i < NUM_BOXES; ++i )
		{
			/* Create this geometry instance */
			optix::GeometryInstance instance = context->createGeometryInstance();
			instance->setGeometry(box);
			instance->setMaterialCount(1);
			instance->setMaterial(0, material);
			
			/* Set variables to be consumed by material for this geometry instance */
			struct material_data_def
			{
				glm::vec3 ambient;
				glm::vec3 kd, ks, ka;
				glm::vec3 reflectivity;
				float expv;
			} material_data;
			float kd_slider = (float)i / (float)(NUM_BOXES-1);
			material_data.kd = glm::vec3(kd_slider, 0.0f, 1.0f-kd_slider);
			material_data.ks = glm::vec3(0.5f, 0.5f, 0.5f);
			material_data.ka = glm::vec3(0.8f, 0.8f, 0.8f);
			material_data.reflectivity = glm::vec3(0.8f, 0.8f, 0.8f);
			material_data.expv = 10.0f;
			material_data.ambient = glm::vec3(0.2f, 0.2f, 0.2f);

			optix::Variable kd = instance->declareVariable("Kd");
			optix::Variable ks = instance->declareVariable("Ks");
			optix::Variable ka = instance->declareVariable("Ka");
			optix::Variable expv = instance->declareVariable("phong_exp");
			optix::Variable reflectivity = instance->declareVariable("reflectivity");
			optix::Variable ambient = instance->declareVariable("ambient_light_color");
			kd->set3fv( &material_data.kd.x );
			ks->set3fv( &material_data.ks.x );
			ka->set3fv( &material_data.ka.x );
			reflectivity->set3fv( &material_data.reflectivity.x );
			expv->setFloat( material_data.expv );
			ambient->set3fv( &material_data.ambient.x );

			/* create group to hold instance transform */
			optix::GeometryGroup geometrygroup = context->createGeometryGroup();
			geometrygroup->setChildCount(1);
			geometrygroup->setChild(0,instance);

			/* create acceleration object for group and specify some build hints*/
			optix::Acceleration acceleration = context->createAcceleration("NoAccel", "NoAccel");
			geometrygroup->setAcceleration(acceleration);

			/* mark acceleration as dirty */
			acceleration->markDirty();

			transforms[i] = context->createTransform();
			transforms[i]->setChild( geometrygroup );
			float seperation = 1.1f;
			glm::mat4 xform = glm::transpose(glm::translate( glm::mat4(1.f), i*seperation - (NUM_BOXES-1)*seperation*.5f,  0.f, 0.f ) );
			transforms[i]->setMatrix( 0, glm::value_ptr(xform), 0 );

		}

		/* Place these geometrygroups as children of the top level object */
		optix::Group top_level_group = context->createGroup();
		top_level_group->setChildCount(NUM_BOXES);
		for(int i=0; i<NUM_BOXES; ++i){
			top_level_group->setChild(i, transforms[i] );
		}
		optix::Variable top_object = context->declareVariable("top_object");
		top_object->set( top_level_group );
		optix::Variable top_shadower = context->declareVariable("top_shadower");
		top_shadower->set(top_level_group);
		top_level_acceleration = context->createAcceleration("NoAccel", "NoAccel");
		top_level_group->setAcceleration(top_level_acceleration);

		/* mark acceleration as dirty */
		top_level_acceleration->markDirty();
	}

	optix::Variable getFTime()
	{
		return fTime;
	}

	optix::Buffer getOutBuffer()
	{
		return out_buffer_obj;
	}

	void updateCamera(bool key_left, bool key_right, bool key_back, bool key_fwd, 
					 glm::vec2 mouse_coords, bool mouse_button_down, float deltaTime)
	{
		fps_camera.update( key_left, key_right, key_back, key_fwd, mouse_coords, mouse_button_down, deltaTime );
		glm::vec3 eyePos = fps_camera.getPos();
		glm::vec3 u = fps_camera.getStrafeDirection();
		glm::vec3 v = fps_camera.getUpDirection();
		glm::vec3 w = fps_camera.getLookDirection();
		
		optixCamVars.eyePos->set3fv(&eyePos.x);
		optixCamVars.u->set3fv(&u.x);
		optixCamVars.v->set3fv(&v.x);
		optixCamVars.w->set3fv(&w.x);
	}

	void animate()
	{
		float t = fTime->getFloat();
		glm::mat4 identity(1.f);		
		int num = transforms.size();
		for (size_t i=0; i<transforms.size(); ++i){
			float a = 6.28f * i/(float)num;
			glm::vec3 pos( cos(a), sin(4.f*a+t)*0.25f, sin(a) );
			pos *= 5.f;

			glm::mat4 xform = glm::translate( identity, pos );
			xform = glm::transpose(xform);
			

			if ( i==0 ) {
				// T*R*S
				glm::mat4 xform(1.0f);
				xform = glm::translate(xform, glm::vec3(0.f, -3.f, 0.f) );
				xform = glm::scale(xform, 25.f, 1.f, 25.f);
				
				transforms[i]->setMatrix( true, glm::value_ptr(xform),  0 );
			} else {
				transforms[i]->setMatrix( false, glm::value_ptr(xform),  0 );
			}
		}

		top_level_acceleration->markDirty();
	}

	~OptixScene()
	{
		out_buffer_obj->unregisterGLBuffer();
		context->destroy();
	}
public:
	Render::PBO *pbo;
private:
	optix::Context context;
	optix::Buffer out_buffer_obj;
	optix::Variable fTime;
	std::vector<optix::Transform> transforms;
	optix::Acceleration top_level_acceleration;

	Scene::FirstPersonCamera fps_camera;
	std::string optix_dir;
	int width;
	int height;
};

class GLContext
{
public:
	GLContext(int width, int height) 
		: width(width)
		, height(height)
	{
		if (!glfwInit())
		{
			fprintf(stderr, "Failed to initialize GLFW: %s\n",
					glfwErrorString(glfwGetError()));
			exit(EXIT_FAILURE);
		}

		wnd = glfwOpenWindow(width, height, GLFW_WINDOWED, "Ray", NULL);

        if (!wnd)
        {
            fprintf(stderr, "Failed to open GLFW window: %s\n",
                    glfwErrorString(glfwGetError()));
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

		glfwMakeContextCurrent(wnd);

		glfwSetWindowCloseCallback(GLContext::closeWindow);
		glfwSetWindowUserPointer(wnd, this);

		//////////////////////////////////////////
		// GL3W INITIALIZING
		//////////////////////////////////////////
		GLenum gl3wInitErr = gl3wInit();
		if(gl3wInitErr)
			throw std::runtime_error("Failed to initialize OpenGL!");
		if(gl3wIsSupported(3, 3) == false)
			throw std::runtime_error("Opengl 3.3 is not supported!");
		wgl3wSwapIntervalEXT(0);

		initState();
	}

	~GLContext()
	{
		glfwTerminate();
	}

	void initState()
	{
		screenQuad = std::unique_ptr<Scene::Quad>( new Scene::Quad(0,0) );

		std::string vertex_src = \
			"#version 330\n"
			"layout(location = 1) in vec2 position;\n"
			"layout(location = 0) in vec2 texcoord;\n"
			"out vec2 t;\n"
			"void main ()\n"
			"{\n"
			"gl_Position = vec4( position.x, position.y, 0.0, 1.0);\n"
			"vec2 madd = vec2(0.5,0.5);\n"
			"vec2 pos_norm = position; // vertices go from 0,0 to 1,1\n"
			"t = (pos_norm * madd) + madd; // Scale to 0-1 range\n"
			"}\n";
		
		std::string fragment_src = \
			"#version 330\n"
			"in vec2 t;\n"
			"layout(location = 0, index = 0) out vec4 out_FragColor;\n"
			"uniform sampler2D tex0;\n"
			"void main()\n"
			"{\n"
			"out_FragColor = vec4(1.0) * texture(tex0,t.xy);\n"
			//"out_FragColor = vec4(t.x,t.y,1.0,1.0);\n"
			"}\n";
		screen_quad_shader = std::unique_ptr<Render::Shader>( new Render::Shader(vertex_src, "", fragment_src ) );

		Render::T2DTexParams params( GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE, 4, width, height, GL_CLAMP_TO_EDGE, (unsigned char*) nullptr ); 
		outputTex.init( params );
	}

	void pbo2Tex(optix::Buffer buffer)
	{
		outputTex.bind();

		int buffer_width = width;
		int buffer_height = height;

		optix::Context context = buffer->getContext();

		Render::PBO *pbo = myScene->pbo;
		pbo->bind(true);

		RTsize elementSize = buffer->getElementSize();
		RTformat buffer_format = buffer->getFormat();
		
		if      ((elementSize % 8) == 0) pbo->align(8);
		else if ((elementSize % 4) == 0) pbo->align(4);
		else if ((elementSize % 2) == 0) pbo->align(2);
		else                             pbo->align(1);

		if(buffer_format == RT_FORMAT_UNSIGNED_BYTE4) {
		  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, buffer_width, buffer_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
		} else if(buffer_format == RT_FORMAT_FLOAT4) {
		  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, buffer_width, buffer_height, 0, GL_RGBA, GL_FLOAT, 0);
		} else if(buffer_format == RT_FORMAT_FLOAT3) {
		  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, buffer_width, buffer_height, 0, GL_RGB, GL_FLOAT, 0);
		} else {
		  throw "Unknown buffer format";
		}

		pbo->unbind();
	}

	void destroy()
	{
		delete myScene;
	}

	static int closeWindow(GLFWwindow wnd)
	{
		GLContext *ptr = (GLContext*)glfwGetWindowUserPointer(wnd);
		ptr->destroy();
		return 1;
	}

	void display(OptixScene *myScene)
	{
		this->myScene = myScene;
		
		optix::Buffer imageBuffer = myScene->getOutBuffer();
		optix::Variable fTime = myScene->getFTime();
		optix::Context context = imageBuffer->getContext();

		context->validate();
		context->compile();

		bool running = true;
		while (running)
		{
			float time =(float)glfwGetTime() ;
			/*char titleBuf[512];
			sprintf(titleBuf, "t = %.2f", time );
			glfwSetWindowTitle(wnd, titleBuf );*/
			fTime->set1fv( &time );

			int mouse_x, mouse_y;
			glfwGetMousePos(wnd, &mouse_x, &mouse_y );
			myScene->updateCamera( glfwGetKey(wnd, 'A'), glfwGetKey(wnd, 'D'), glfwGetKey(wnd, 'S'), glfwGetKey(wnd, 'W'),
			                       glm::vec2((float)mouse_x, (float)mouse_y), glfwGetMouseButton(wnd,0), 1.f/60.f );
			myScene->animate();
			context->launch(0 /*entry point*/, width, height );

			glActiveTexture(GL_TEXTURE0 + 0);
			pbo2Tex(imageBuffer);
			//glClear(GL_COLOR_BUFFER_BIT);
			// display imagebuffer on a gl quad rendered using a vanilla vs, and a texture samplin' fs.
				
			outputTex.bind();
			screen_quad_shader->bind();
			int loc_tex0 = glGetUniformLocation( screen_quad_shader->getFS() , "tex0");
			glProgramUniform1i(screen_quad_shader->getFS(), loc_tex0, 0);
			
			screenQuad->render(0);

			glfwSwapBuffers();

			glfwPollEvents();
			if (!glfwIsWindow(wnd) ) {
				running = false;
			}
		}
	}

	private:
		int width, height;
		GLFWwindow wnd;

		Render::Tex2D outputTex;
		std::unique_ptr<Scene::Quad> screenQuad;
		std::unique_ptr<Render::Shader> screen_quad_shader;

		OptixScene *myScene;		
};

int main(int argc, char* argv[])
{
	std::string resource_dir = argv[0];
	resource_dir = resource_dir.substr(0, resource_dir.find_last_of("\\"));
	resource_dir = resource_dir.substr(0, resource_dir.find_last_of("\\"));
	resource_dir += "\\resources\\";

	std::string optix_dir = resource_dir + "\\glfw_optix\\";

	unsigned int width = TEX_WIDTH;
	unsigned int height = TEX_HEIGHT;

	GLContext myGLContext(width,height);
	
	try {
		OptixScene *myScene = new OptixScene(optix_dir, width, height);
		myGLContext.display( myScene );
	} catch( std::exception &e) {
		std::cout << e.what() << std::endl;
		//printf("%s\n", e.what() );
		system("pause");
	}

	glfwTerminate();

	return 0;
}

