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

set(NAME HybridMethods)
project(${NAME})

file(GLOB_RECURSE GL3W_HEADER ${CMAKE_SOURCE_DIR}/depends/include/GL3/*.h)
file(GLOB_RECURSE GLUT_HEADER ${CMAKE_SOURCE_DIR}/depends/include/GL/*.h)
file(GLOB 		  FILE_HEADER ./File/*.h)
file(GLOB		  FILE_BART_HEADER ./File/BARTLoader/*.h)
file(GLOB_RECURSE PARSER_HEADER ./Parser/*.h)
file(GLOB_RECURSE RENDER_HEADER ./Render/*.h)
file(GLOB_RECURSE RAYTRACER_HEADER ./Raytracer/*.h)
file(GLOB_RECURSE SCENE_HEADER ./Scene/*.h)
file(GLOB ROOT_HEADER *.h)

file(GLOB_RECURSE GL3W_SOURCE ${CMAKE_SOURCE_DIR}/depends/include/GL3/*.c)
file(GLOB_RECURSE GLUT_SOURCE ${CMAKE_SOURCE_DIR}/depends/include/GL/src/*.c)
file(GLOB		  FILE_SOURCE ./File/*.cpp)
file(GLOB		  FILE_BART_SOURCE ./File/BARTLoader/*.cpp)
file(GLOB_RECURSE PARSER_SOURCE ./Parser/*.cpp)
file(GLOB_RECURSE RENDER_SOURCE ./Render/*.cpp)
file(GLOB_RECURSE RAYTRACER_SOURCE ./Raytracer/*.cpp)
file(GLOB_RECURSE SCENE_SOURCE ./Scene/*.cpp)
file(GLOB ROOT_SOURCE *.cpp)

SOURCE_GROUP("Header Files\\Depends\\Gl3w" FILES ${GL3W_HEADER})
SOURCE_GROUP("Header Files\\Depends\\Glut" FILES ${GLUT_HEADER})
SOURCE_GROUP("Header Files\\File" FILES ${FILE_HEADER})
SOURCE_GROUP("Header Files\\File\\BARTLoader" FILES ${FILE_BART_HEADER})
SOURCE_GROUP("Header Files\\Parser" FILES ${PARSER_HEADER})
SOURCE_GROUP("Header Files\\Render" FILES ${RENDER_HEADER})
SOURCE_GROUP("Header Files\\Raytracer" FILES ${RAYTRACER_HEADER})
SOURCE_GROUP("Header Files\\Scene" FILES ${SCENE_HEADER})

SOURCE_GROUP("Source Files\\Depends\\Gl3w" FILES ${GL3W_SOURCE})
SOURCE_GROUP("Source Files\\Depends\\Glut" FILES ${GLUT_SOURCE})
SOURCE_GROUP("Source Files\\File" FILES ${FILE_SOURCE})
SOURCE_GROUP("Source Files\\File\\BARTLoader" FILES ${FILE_BART_SOURCE})
SOURCE_GROUP("Source Files\\Parser" FILES ${PARSER_SOURCE})
SOURCE_GROUP("Source Files\\Render" FILES ${RENDER_SOURCE})
SOURCE_GROUP("Source Files\\Raytracer" FILES ${RAYTRACER_SOURCE})
SOURCE_GROUP("Source Files\\Scene" FILES ${SCENE_SOURCE})

set(HDRS
  ${GL3W_HEADER}
  ${GLUT_HEADER}
  ${FILE_HEADER}
  ${FILE_BART_HEADER}
  ${PARSER_HEADER}
  ${RENDER_HEADER}
  ${RAYTRACER_HEADER}
  ${ROOT_HEADER}
  ${SCENE_HEADER}
)

set(SRCS
  ${GL3W_SOURCE}
  ${GLUT_SOURCE}
  ${FILE_SOURCE}
  ${FILE_BART_SOURCE}
  ${PARSER_SOURCE}
  ${RENDER_SOURCE}
  ${RAYTRACER_SOURCE}
  ${SCENE_SOURCE}
  ${ROOT_SOURCE}
)

add_executable(${NAME} ${HDRS} ${SRCS})

find_package(OpenGL REQUIRED)
find_package(CUDA REQUIRED)
find_package(Optix REQUIRED)
find_package(DevIL REQUIRED)

add_definitions(-DFREEGLUT_STATIC)

target_link_libraries(${NAME}
	OpenGL32
	WinMM
	${CUDA_LIB}
	${OPTIX_LIB}
	${OPTIXU_LIB}
	${IL_LIBRARY}
	${ILU_LIBRARY}
	${ILUT_LIBRARY}
)
