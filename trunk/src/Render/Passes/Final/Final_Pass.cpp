#include "Final_Pass.h"

#include "../GBuffer/GBuffer_Pass.h"
#include "../../../Scene/proto_camera.h"

using namespace Render;

Final_Pass::Final_Pass(const GBuffer_PassPtr &g_buffer_pass, const File::ShaderLoaderPtr &shader_loader, unsigned int w, unsigned int h)
	: g_buffer_pass(g_buffer_pass), shader_loader(shader_loader), w(w), h(h)
{
	////////////////////////////////////////
	// LOAD FBO
	////////////////////////////////////////
	fbo = std::make_shared<FBO>(w,h);
	
	//Add render targets
	fbo->add(GL_COLOR_ATTACHMENT0, std::make_shared<RT>(GL_RGBA, w,h));					//Diffuse

	//Add render textures
	fbo->add(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, "TEX_DIFF", std::make_shared<Tex2D>(T2DTexParams(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 4, w,h)));//Diffuse

	//Check that everything is ok
	fbo->check();
	fbo->unbind();

	quad = std::make_shared<Scene::Quad>(w,h);
	shader = shader_loader->load("present.vs", std::string(), "finalColor.fs");
	camPos = std::make_shared<Uniform>(shader->getFS(), "CamPos_vs");
}

void Final_Pass::begin()
{
	shader_loader->push_bind(shader);
	//fbo->bind();

	//glClearColor(0.f,0.f,0.f,1.f);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	g_buffer_pass->bind(shader->getFS());

	camPos->bind(glm::vec3(Scene::FirstPersonCamera::getSingleton()->getWorldToViewMatrix() * glm::vec4(Scene::FirstPersonCamera::getSingleton()->getPos(), 1.0)));

	if(materials.empty() == false)
	{
		for(auto it=materials.begin(); it!=materials.end(); ++it) //for(auto &material : materials)
			(*it)->bind_data(shader->getFS());
	}

    if(tex)
	{
		glActiveTexture(GL_TEXTURE0);
		tex->bind();
		if(tex_uniform)
			tex_uniform->bind(0);
	}

	/*GLint nViewport[4];
	glGetIntegerv(GL_VIEWPORT, nViewport);
	temp_w = nViewport[2];
	temp_h = nViewport[3];
	glViewportIndexedf(0,0,0,(float)w,(float)h);

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, buffers);*/
}

void Final_Pass::render()
{
	quad->render(0);
}

void Final_Pass::end()
{
	shader_loader->pop_bind();
	//fbo->unbind();

	g_buffer_pass->unbind();

    if(tex)
		tex->unbind();

	//glViewportIndexedf(0,0,0,(float)temp_w,(float)temp_h);

	//GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
	//glDrawBuffers(1, buffers);
}

void Final_Pass::bind(unsigned int active_program)
{
	fbo->bind_rt(active_program, 0);
}

void Final_Pass::unbind()
{
	fbo->unbind_rt();
}

void Final_Pass::reshape(unsigned int w, unsigned int h) 
{ 
	this->w = w;
	this->h = h; 
}

void Final_Pass::reloadShaders()
{
	puts("reload");
	shader = shader_loader->load("deferredRendering.vs", std::string(), "deferredRendering.fs");
}

void Final_Pass::setRayTexture(const Render::Tex2DPtr &tex, const Render::UniformPtr &uniform)
{
    this->tex = tex;
    this->tex_uniform = uniform;
}
