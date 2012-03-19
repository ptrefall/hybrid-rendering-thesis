###
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
# 
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#
# Note: Some of the libraries GMEngine may link to may have additional
# requirements or restrictions.
#
###

cmake_minimum_required(VERSION 2.8)
 
project(HybridMethods)

set(DependencyGl3w_HDRS
	${CMAKE_SOURCE_DIR}/depends/include/GL3/gl3.h
	${CMAKE_SOURCE_DIR}/depends/include/GL3/gl3w.h
)
SOURCE_GROUP("Header Files\\Depends\\Gl3w" FILES ${DependencyGl3w_HDRS})

set(DependencyGlut_HDRS
	${CMAKE_SOURCE_DIR}/depends/include/GL/freeglut.h
	${CMAKE_SOURCE_DIR}/depends/include/GL/freeglut_std.h
	${CMAKE_SOURCE_DIR}/depends/include/GL/freeglut_ext.h
	${CMAKE_SOURCE_DIR}/depends/include/GL/glut.h
)
SOURCE_GROUP("Header Files\\Depends\\Glut" FILES ${DependencyGlut_HDRS})

set(Dependency_HDRS
	${DependencyGl3w_HDRS}
	${DependencyGlut_HDRS}
)

set(DependencyGl3w_SRCS
	../depends/include/GL3/gl3w.c
)
SOURCE_GROUP("Source Files\\Depends\\Gl3w" FILES ${DependencyGl3w_SRCS})

set(Dependency_SRCS
  ${DependencyGl3w_SRCS}
)

set(Render_HDRS
  ./Render/DeferredRender.h
  ./Render/GBuffer.h
  ./Render/Shader.h
  ./Render/ShaderConstants.h
  ./Render/Tex2D.h
  ./Render/VAO.h
  ./Render/VBO.h
  ./Render/IBO.h
  ./Render/FBO.h
  ./Render/RT.h
  ./Render/ATTRIB.h
)
SOURCE_GROUP("Header Files\\Render" FILES ${Render_HDRS})

set(Render_SRCS
  ./Render/DeferredRender.cpp
  ./Render/GBuffer.cpp
  ./Render/Shader.cpp
  ./Render/Tex2D.cpp
  ./Render/VAO.cpp
  ./Render/VBO.cpp
  ./Render/IBO.cpp
  ./Render/FBO.cpp
  ./Render/RT.cpp
)
SOURCE_GROUP("Source Files\\Render" FILES ${Render_SRCS})

set(HDRS
  ${Render_HDRS}
)

set(SRCS
  ${Render_SRCS}
  main.cpp
)
 
add_executable(HybridMethods ${Dependency_HDRS} ${Dependency_SRCS} ${HDRS} ${SRCS})

find_package(OpenGL REQUIRED)
find_package(Glut REQUIRED)

add_definitions(-DFREEGLUT_STATIC)

target_link_libraries(HybridMethods
	OpenGL32
	debug ${GLUT_STATIC_DEBUG_LIBRARY} optimized ${GLUT_STATIC_RELEASE_LIBRARY}
)
