cmake_minimum_required(VERSION 2.8)

set(NAME glfw_optix)
project(${NAME})

file(GLOB_RECURSE GL3W_HEADER ${CMAKE_SOURCE_DIR}/depends/include/GL3/*.h)
file(GLOB_RECURSE GL3W_SOURCE ${CMAKE_SOURCE_DIR}/depends/include/GL3/*.c)
file(GLOB_RECURSE ASSIMP_HEADER_H ${CMAKE_SOURCE_DIR}/depends/include/AssImp/*.h)
file(GLOB_RECURSE ASSIMP_HEADER ${CMAKE_SOURCE_DIR}/depends/include/AssImp/*.hpp)
file(GLOB_RECURSE ASSIMP_SOURCE_C ${CMAKE_SOURCE_DIR}/depends/include/AssImp/*.c)
file(GLOB_RECURSE ASSIMP_SOURCE ${CMAKE_SOURCE_DIR}/depends/include/AssImp/*.cpp)

file(GLOB FILE_HEADER ${CMAKE_SOURCE_DIR}/src/File/*.h)
file(GLOB FILE_SOURCE ${CMAKE_SOURCE_DIR}/src/File/*.cpp)
file(GLOB FILE_BART_HEADER ${CMAKE_SOURCE_DIR}/src/File/BARTLoader/*.h)
file(GLOB FILE_BART_SOURCE ${CMAKE_SOURCE_DIR}/src/File/BARTLoader/*.cpp)
file(GLOB PARSER_BART_HEADER ${CMAKE_SOURCE_DIR}/src/Parser/BART/*.h)
file(GLOB PARSER_BART_SOURCE ${CMAKE_SOURCE_DIR}/src/Parser/BART/*.cpp)
file(GLOB PARSER_HEADER ${CMAKE_SOURCE_DIR}/src/Parser/*.h)
file(GLOB PARSER_SOURCE ${CMAKE_SOURCE_DIR}/src/Parser/*.cpp)

file(GLOB 		  HYBRID_HEADER
	${CMAKE_SOURCE_DIR}/src/Render/ATTRIB.h
	${CMAKE_SOURCE_DIR}/src/Render/IBO.h
	${CMAKE_SOURCE_DIR}/src/Render/Shader.h
	${CMAKE_SOURCE_DIR}/src/Render/ShaderConstants.h
	${CMAKE_SOURCE_DIR}/src/Render/Tex2D.h
	${CMAKE_SOURCE_DIR}/src/Render/Tex2DArray.h
	${CMAKE_SOURCE_DIR}/src/Render/VAO.h
	${CMAKE_SOURCE_DIR}/src/Render/VBO.h
	${CMAKE_SOURCE_DIR}/src/Render/PBO.h
	${CMAKE_SOURCE_DIR}/src/Render/Uniform.h
	${CMAKE_SOURCE_DIR}/src/Render/Material.h
	
	${CMAKE_SOURCE_DIR}/src/Scene/Quad.h
	${CMAKE_SOURCE_DIR}/src/Scene/proto_camera.h
	${CMAKE_SOURCE_DIR}/src/Scene/SceneNode.h
	${CMAKE_SOURCE_DIR}/src/Scene/Mesh.h
	${CMAKE_SOURCE_DIR}/src/Scene/BARTMesh.h
	${CMAKE_SOURCE_DIR}/src/Scene/OptixInstance.h
	${CMAKE_SOURCE_DIR}/src/File/MeshLoader.h

	${CMAKE_SOURCE_DIR}/src/Raytracer/OptixTriMeshLoader.h
)

file(GLOB 		  HYBRID_SOURCE
	${CMAKE_SOURCE_DIR}/src/Render/ATTRIB.cpp
	${CMAKE_SOURCE_DIR}/src/Render/IBO.cpp
	${CMAKE_SOURCE_DIR}/src/Render/Shader.cpp
	${CMAKE_SOURCE_DIR}/src/Render/ShaderConstants.cpp
	${CMAKE_SOURCE_DIR}/src/Render/Tex2D.cpp
	${CMAKE_SOURCE_DIR}/src/Render/Tex2DArray.cpp
	${CMAKE_SOURCE_DIR}/src/Render/VAO.cpp
	${CMAKE_SOURCE_DIR}/src/Render/VBO.cpp
	${CMAKE_SOURCE_DIR}/src/Render/PBO.cpp
	${CMAKE_SOURCE_DIR}/src/Render/Uniform.cpp
	${CMAKE_SOURCE_DIR}/src/Render/Material.cpp
	
	${CMAKE_SOURCE_DIR}/src/Scene/Quad.cpp
	${CMAKE_SOURCE_DIR}/src/Scene/proto_camera.cpp
	
	${CMAKE_SOURCE_DIR}/src/Scene/SceneNode.cpp
	${CMAKE_SOURCE_DIR}/src/Scene/Mesh.cpp
	${CMAKE_SOURCE_DIR}/src/Scene/BARTMesh.cpp
	${CMAKE_SOURCE_DIR}/src/Scene/OptixInstance.cpp
	${CMAKE_SOURCE_DIR}/src/File/MeshLoader.cpp

	${CMAKE_SOURCE_DIR}/src/Raytracer/OptixTriMeshLoader.cpp
)

file(GLOB_RECURSE OPTIX_SCRIPTS ${CMAKE_SOURCE_DIR}/bin/resources/glfw_optix/*.cu)
file(GLOB ROOT_HEADER src/*.h)
file(GLOB ROOT_SOURCE src/*.cpp)

SOURCE_GROUP("Header Files\\Depends\\AssImp" FILES ${ASSIMP_HEADER_H} ${ASSIMP_HEADER})
SOURCE_GROUP("Source Files\\Depends\\AssImp" FILES ${ASSIMP_SOURCE_C} ${ASSIMP_SOURCE})

SOURCE_GROUP("Resource Files\\Optix\\" FILES ${OPTIX_SCRIPTS})
SOURCE_GROUP("Header Files\\HybridRender" FILES ${HYBRID_HEADER})
SOURCE_GROUP("Source Files\\HybridRender" FILES ${HYBRID_SOURCE})

SOURCE_GROUP("Header Files\\HybridRender\\File" FILES ${FILE_HEADER})
SOURCE_GROUP("Source Files\\HybridRender\\File" FILES ${FILE_SOURCE})

SOURCE_GROUP("Header Files\\HybridRender\\File" FILES ${FILE_BART_HEADER})
SOURCE_GROUP("Source Files\\HybridRender\\File" FILES ${FILE_BART_SOURCE})

SOURCE_GROUP("Header Files\\HybridRender\\File" FILES ${PARSER_BART_HEADER})
SOURCE_GROUP("Source Files\\HybridRender\\File" FILES ${PARSER_BART_SOURCE})

set(RESOURCES
	${OPTIX_SCRIPTS}
)

set(HDRS
	${GL3W_HEADER}
	${ASSIMP_HEADER_H} 
	${ASSIMP_HEADER}
	${FILE_HEADER}
	${FILE_BART_HEADER}
	${PARSER_BART_HEADER}
	${PARSER_HEADER}
	${HYBRID_HEADER}
	${ROOT_HEADER}
)

set(SRCS
	${GL3W_SOURCE}
	${ASSIMP_SOURCE_C} 
	${ASSIMP_SOURCE}
	${FILE_SOURCE}
	${FILE_BART_SOURCE}
	${PARSER_BART_SOURCE}
	${PARSER_SOURCE}
	${HYBRID_SOURCE}
	${ROOT_SOURCE}
)


find_package(GLFW REQUIRED)
find_package(CUDA REQUIRED)
find_package(Optix REQUIRED)
find_package(DevIL REQUIRED)

include_directories(
	${GLFW_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/depends/include/
	${CMAKE_SOURCE_DIR}/depends/include/CUDA/
	${CMAKE_SOURCE_DIR}/src/
	${CMAKE_SOURCE_DIR}/bin/resources/glfw_optix/
)

ADD_DEFINITIONS(-DASSIMP_BUILD_BOOST_WORKAROUND -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DASSIMP_BUILD_NO_IFC_IMPORTER)

add_executable(${NAME} ${RESOURCES} ${HDRS} ${SRCS})

target_link_libraries(${NAME}
	OpenGL32
	${CUDA_LIB}
	${OPTIX_LIB}
	${OPTIXU_LIB}
	debug ${GLFW_STATIC_DEBUG_LIBRARY} optimized ${GLFW_STATIC_RELEASE_LIBRARY}
	${IL_LIBRARY}
	${ILU_LIBRARY}
	${ILUT_LIBRARY}
)

IF(MSVC)	   
	TARGET_LINK_LIBRARIES(${NAME} WinMM)
ENDIF()


include( ${CMAKE_MODULE_PATH}/compile2ptx.cmake )

COMPILE_CU2PTX( ${OPTIX_SCRIPTS} )

