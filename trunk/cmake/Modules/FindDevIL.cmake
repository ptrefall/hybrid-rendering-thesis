
FIND_PATH(IL_DIR il.h
      ${CMAKE_SOURCE_DIR}/depends/IL
    )

FIND_LIBRARY(IL_LIBRARY
  NAMES DevIL
  PATHS ${CMAKE_SOURCE_DIR}/depends/lib
    )
	
FIND_LIBRARY(ILU_LIBRARY
  NAMES ILU.lib
  PATHS ${CMAKE_SOURCE_DIR}/depends/lib
    )
	
FIND_LIBRARY(ILUT_LIBRARY
  NAMES ILUT.lib
  PATHS ${CMAKE_SOURCE_DIR}/depends/lib
    )
	