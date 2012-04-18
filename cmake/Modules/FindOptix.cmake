
FIND_PATH(OPTIX_DIR optixu/optixpp_namespace.h
      ${CMAKE_SOURCE_DIR}/depends/Optix/
    )

FIND_LIBRARY(OPTIX_LIB
  NAMES optix.1
  PATHS ${CMAKE_SOURCE_DIR}/depends/lib
    )
	
FIND_LIBRARY(OPTIXU_LIB
  NAMES optixu.1
  PATHS ${CMAKE_SOURCE_DIR}/depends/lib
    )
	