cmake_minimum_required(VERSION 2.8)

set(NAME glfw_optix)
project(${NAME})

file(GLOB_RECURSE GL3W_HEADER ${CMAKE_SOURCE_DIR}/depends/include/GL3/*.h)
file(GLOB_RECURSE GL3W_SOURCE ${CMAKE_SOURCE_DIR}/depends/include/GL3/*.c)

file(GLOB 		  HYBRID_HEADER
	${CMAKE_SOURCE_DIR}/src/Render/ATTRIB.h
	${CMAKE_SOURCE_DIR}/src/Render/IBO.h
	${CMAKE_SOURCE_DIR}/src/Render/Shader.h
	${CMAKE_SOURCE_DIR}/src/Render/ShaderConstants.h
	${CMAKE_SOURCE_DIR}/src/Render/Tex2D.h
	${CMAKE_SOURCE_DIR}/src/Render/VAO.h
	${CMAKE_SOURCE_DIR}/src/Render/VBO.h
	${CMAKE_SOURCE_DIR}/src/Render/PBO.h
	${CMAKE_SOURCE_DIR}/src/Scene/Quad.h
	${CMAKE_SOURCE_DIR}/src/Scene/proto_camera.h
)
file(GLOB 		  HYBRID_SOURCE
	${CMAKE_SOURCE_DIR}/src/Render/ATTRIB.cpp
	${CMAKE_SOURCE_DIR}/src/Render/IBO.cpp
	${CMAKE_SOURCE_DIR}/src/Render/Shader.cpp
	${CMAKE_SOURCE_DIR}/src/Render/ShaderConstants.cpp
	${CMAKE_SOURCE_DIR}/src/Render/Tex2D.cpp
	${CMAKE_SOURCE_DIR}/src/Render/VAO.cpp
	${CMAKE_SOURCE_DIR}/src/Render/VBO.cpp
	${CMAKE_SOURCE_DIR}/src/Render/PBO.cpp
	${CMAKE_SOURCE_DIR}/src/Scene/Quad.cpp
	${CMAKE_SOURCE_DIR}/src/Scene/proto_camera.cpp
)

file(GLOB_RECURSE OPTIX_SCRIPTS ${CMAKE_SOURCE_DIR}/bin/resources/glfw_optix/*.cu)
file(GLOB ROOT_HEADER src/*.h)
file(GLOB ROOT_SOURCE src/*.cpp)

SOURCE_GROUP("Resource Files\\Optix\\" FILES ${OPTIX_SCRIPTS})
SOURCE_GROUP("Header Files\\HybridRender" FILES ${HYBRID_HEADER})
SOURCE_GROUP("Source Files\\HybridRender" FILES ${HYBRID_SOURCE})

set(RESOURCES
	${OPTIX_SCRIPTS}
)

set(HDRS
	${GL3W_HEADER}
	${HYBRID_HEADER}
	${ROOT_HEADER}
)

set(SRCS
	${GL3W_SOURCE}
	${HYBRID_SOURCE}
	${ROOT_SOURCE}
)


find_package(GLFW REQUIRED)
find_package(CUDA REQUIRED)
find_package(Optix REQUIRED)

include_directories(
	${GLFW_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/depends/include/
	${CMAKE_SOURCE_DIR}/depends/include/CUDA/
	${CMAKE_SOURCE_DIR}/src/
	${CMAKE_SOURCE_DIR}/bin/resources/glfw_optix/
)

ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)

add_executable(${NAME} ${RESOURCES} ${HDRS} ${SRCS})

target_link_libraries(${NAME}
	OpenGL32
	${CUDA_LIB}
	${OPTIX_LIB}
	${OPTIXU_LIB}
	debug ${GLFW_STATIC_DEBUG_LIBRARY} optimized ${GLFW_STATIC_RELEASE_LIBRARY}
)

IF(MSVC)

	SET(VS_MULTITHREADED_DEBUG_IGNORE_LIBRARY_FLAGS
		"/NODEFAULTLIB:libcmt.lib"
	)
	#SET_TARGET_PROPERTIES(${NAME} PROPERTIES
		   #LINK_FLAGS_DEBUG
		   #${VS_MULTITHREADED_DEBUG_IGNORE_LIBRARY_FLAGS}
	#)
		   
	TARGET_LINK_LIBRARIES(${NAME} WinMM)
ENDIF()


include( ${CMAKE_MODULE_PATH}/compile2ptx.cmake )

COMPILE_CU2PTX( ${OPTIX_SCRIPTS} )

