#include "OptixMesh.h"
#include "proto_camera.h"
#include "Light.h"

#include "../Render/ATTRIB.h"
#include "../Render/ShaderConstants.h"


#include <glm/ext.hpp>
#include <vector>

using namespace Scene;
using namespace glm;

OptixMesh::OptixMesh(MeshDataPtr data, optix::Context rtContext, const std::string &ptx_dir)
	: Mesh(data)
{
	int num_indices = data->indices.size();
	int num_triangles = data->indices.size() / 3;
	int num_vertices = data->vertices.size() / 3;
	int num_normals = data->normals.size() / 3;

	rtModel = rtContext->createGeometry();
	rtModel->setPrimitiveCount( num_triangles );
	rtModel->setIntersectionProgram( rtContext->createProgramFromPTXFile( ptx_dir+"triangle_mesh_small.cu.ptx", "mesh_intersect" ) );
	rtModel->setBoundingBoxProgram( rtContext->createProgramFromPTXFile( ptx_dir+"triangle_mesh_small.cu.ptx", "mesh_bounds" ) );
	
	//int num_vertex_attributes = 3; // allways has verts
	//if(data->hasNormals() ) num_vertex_attributes += 3;
	//if(data->hasBitangents() ) num_vertex_attributes += 3;
	//if(data->hasTexCoords() ) num_vertex_attributes += 2;
	//if(data->hasColors() ) num_vertex_attributes += 4;

	optix::Buffer vertex_buffer = rtContext->createBufferFromGLBO(RT_BUFFER_INPUT, vbo->getHandle() );
    vertex_buffer->setFormat(RT_FORMAT_USER);
    vertex_buffer->setElementSize(3*sizeof(float));
    vertex_buffer->setSize(num_vertices + num_normals);
    rtModel["vertex_buffer"]->setBuffer(vertex_buffer);

    optix::Buffer index_buffer = rtContext->createBufferFromGLBO(RT_BUFFER_INPUT, ibo->getHandle() );
	index_buffer->setFormat(RT_FORMAT_INT3);
    index_buffer->setSize( num_triangles );
    rtModel["index_buffer"]->setBuffer(index_buffer);

	rtModel["normal_offset"]->setInt( num_normals );

	std::string vs = "#version 330 core\n"
	"#define DIFFUSE  0\n"
	"#define POSITION	1\n"
	"#define NORMAL   2\n"
	"#define TEXCOORD	3\n"
	
	"uniform mat4 Object_to_World;\n"
	"uniform mat4 World_to_View;\n"
	"uniform mat4 View_to_Clip;\n"
	"uniform mat3 Normal_to_View;\n"
	
	"layout(location = POSITION) in vec3 Position_os;\n"	//object space
	"layout(location = NORMAL)   in vec3 Normal_os;\n"	//object space
	"layout(location = TEXCOORD) in vec2 TexCoord;\n"

	"out gl_PerVertex\n"
	"{\n"
	" vec4 gl_Position;\n"
	"};\n"

	"out block\n"
	"{"
	" vec4 position_ws;\n"	//world space
	" vec4 position_vs;\n" 	//view space
	" vec3 normal_vs;\n" 	//view space
	" vec2 texcoord;\n"
	"} Vertex;\n"

	"void main( void )\n"
	"{"
	"  Vertex.texcoord      = TexCoord;\n"
	"  Vertex.normal_vs     = normalize(Normal_to_View * Normal_os);\n"		//Object space to View space
	
	"  Vertex.position_ws   = Object_to_World * vec4(Position_os, 1.0);\n"	    //Object space to World space
	"  Vertex.position_vs   = World_to_View * Vertex.position_ws;\n"           //World  space to View  space
	"  gl_Position          = View_to_Clip  * Vertex.position_vs;\n"			//View   space to Clip  space
	// MODEL =      Object_to_World
	// VIEW =       World_to_View
	// PROJECTION = View_to_Clip
	//"  gl_Position          = View_to_Clip * World_to_View * Object_to_World * vec4(Position_os, 1.0);\n"			//View   space to Clip  space
	"}\n";

	std::string fs = "#version 330\n"
		"#define DIFFUSE  0\n"
		"#define POSITION	1\n"
		"#define NORMAL   2\n"
		"#define TEXCOORD	3\n"
		"in block\n"
		"{\n"
		"  vec4 position_ws;\n"	//world space
		"  vec4 position_vs;\n" 	//view space
		"  vec3 normal_vs;\n"    //view space
		"  vec2 texcoord;\n"
		"} Vertex;\n"
		"\n"
		"layout(location = 0, index = 0) out vec4 out_FragColor;\n"
		"void main()\n"
		"{\n"
		"out_FragColor = vec4(Vertex.normal_vs*0.5+0.5, 0.0);\n"
		"}\n";

	boring_shader = std::shared_ptr<Render::Shader>( new Render::Shader(vs, "", fs) );
	uni_object_to_world		= std::shared_ptr<Render::Uniform>( new Render::Uniform(boring_shader->getVS(), "Object_to_World") );
	uni_world_to_view		= std::shared_ptr<Render::Uniform>( new Render::Uniform(boring_shader->getVS(), "World_to_View") );
	uni_view_to_clip		= std::shared_ptr<Render::Uniform>( new Render::Uniform(boring_shader->getVS(), "View_to_Clip") );
	uni_normal_to_view		= std::shared_ptr<Render::Uniform>( new Render::Uniform(boring_shader->getVS(), "Normal_to_View") );
}

void OptixMesh::render(const Render::ShaderPtr &active_program)
{
	boring_shader->bind();

	object_to_world = glm::translate(position) * glm::mat4_cast(orientation) * glm::scale(scale);
	auto &world_to_view = FirstPersonCamera::getSingleton()->getWorldToViewMatrix();
	auto &view_to_clip = FirstPersonCamera::getSingleton()->getViewToClipMatrix();
	auto normal_to_view = transpose(inverse(mat3(world_to_view * object_to_world)));

	uni_object_to_world->	bind(object_to_world);
	uni_world_to_view->		bind(world_to_view);
	uni_view_to_clip->		bind(view_to_clip);
	uni_normal_to_view->	bind(normal_to_view);

	int counter = 0;
	for(auto it=textures.begin(); it!=textures.end(); ++it)
	{
		glActiveTexture(GL_TEXTURE0+it->first);
		it->second.first->bind();
		it->second.second->bind((int)it->first);
		counter++;
	}
	
	//if(material)
		//material->bind_id(active_program->getFS());

	vao->bind();

	glDrawElements(GL_TRIANGLES, ibo->size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));
}


void OptixMesh::setTexture(int slot, const Render::Tex2DPtr &tex, const std::string &uni_name) 
{ 
}

