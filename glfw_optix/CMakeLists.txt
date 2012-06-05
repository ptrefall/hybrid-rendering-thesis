cmake_minimum_required(VERSION 2.8)

set(NAME glfw_optix)
project(${NAME})

file(GLOB_RECURSE OPTIX_SCRIPTS ${CMAKE_SOURCE_DIR}/bin/resources/glfw_optix/*.cu)
file(GLOB ROOT_HEADER include/*.h)
file(GLOB ROOT_SOURCE src/*.cpp)

SOURCE_GROUP("Resource Files\\Optix\\" FILES ${OPTIX_SCRIPTS})
SOURCE_GROUP("Header Files\\Scene" FILES ${SCENE_HEADER})
SOURCE_GROUP("Source Files\\Scene" FILES ${SCENE_SOURCE})

set(RESOURCES
	${OPTIX_SCRIPTS}
)

set(HDRS

	${ROOT_HEADER}
)

set(SRCS
	${ROOT_SOURCE}
)


find_package(GLFW REQUIRED)
find_package(CUDA REQUIRED)
find_package(Optix REQUIRED)

include_directories(
	${GLFW_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/depends/include/
	${CMAKE_SOURCE_DIR}/depends/include/CUDA/
)

ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)

add_executable(${NAME} ${RESOURCES} ${HDRS} ${SRCS})

target_link_libraries(${NAME}
	OpenGL32
	${CUDA_LIB}
	${OPTIX_LIB}
	${OPTIXU_LIB}
	${GLFW_LIB}
)

IF(MSVC)
	TARGET_LINK_LIBRARIES(${NAME} WinMM)
ENDIF()


include( ${CMAKE_MODULE_PATH}/compile2ptx.cmake )

COMPILE_CU2PTX( ${OPTIX_SCRIPTS} )

