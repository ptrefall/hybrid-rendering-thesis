
FIND_PATH(GLUT_DIR glut.h
      ${CMAKE_SOURCE_DIR}/depends/GL
    )

FIND_LIBRARY(GLUT_STATIC_DEBUG_LIBRARY
  NAMES freeglut-static-mt-debug
  PATHS ${CMAKE_SOURCE_DIR}/depends/lib
    )
	
FIND_LIBRARY(GLUT_STATIC_RELEASE_LIBRARY
  NAMES freeglut-static-mt
  PATHS ${CMAKE_SOURCE_DIR}/depends/lib
    )
	