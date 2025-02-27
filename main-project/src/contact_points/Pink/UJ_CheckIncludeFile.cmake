# -*- sh -*-
# - Check if the include file exists.
# CHECK_INCLUDE_FILE(INCLUDE VARIABLE)
# - macro which checks the include file exists.
#  INCLUDE  - name of include file
#  VARIABLE - variable to return result
#   
# an optional third argument is the CFlags to add to the compile line 
# or you can use CMAKE_REQUIRED_FLAGS
#
# The following variables may be set before calling this macro to
# modify the way the check is run:
#
#  CMAKE_REQUIRED_FLAGS = string of compile command line flags
#  CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=bar)
#  CMAKE_REQUIRED_INCLUDES = list of include directories
#
MACRO(CHECK_INCLUDE_FILE INCLUDE VARIABLE)
#  IF("${VARIABLE}" MATCHES "^${VARIABLE}$")
    IF(CMAKE_REQUIRED_INCLUDES)
      SET(CHECK_INCLUDE_FILE_C_INCLUDE_DIRS "-DINCLUDE_DIRECTORIES=${CMAKE_REQUIRED_INCLUDES}")
    ELSE(CMAKE_REQUIRED_INCLUDES)
      SET(CHECK_INCLUDE_FILE_C_INCLUDE_DIRS)
    ENDIF(CMAKE_REQUIRED_INCLUDES)
    SET(MACRO_CHECK_INCLUDE_FILE_FLAGS ${CMAKE_REQUIRED_FLAGS})
    SET(CHECK_INCLUDE_FILE_VAR ${INCLUDE})
    CONFIGURE_FILE(UJ_CheckIncludeFile.c.in
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckIncludeFile.c IMMEDIATE)
    MESSAGE(STATUS "Looking for ${INCLUDE}")
    IF(${ARGC} EQUAL 3)
      SET(CMAKE_C_FLAGS_SAVE ${CMAKE_C_FLAGS})
      SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARGV2}")
    ENDIF(${ARGC} EQUAL 3)

    TRY_COMPILE(${VARIABLE}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckIncludeFile.c
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      CMAKE_FLAGS 
      -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_INCLUDE_FILE_FLAGS}
      "${CHECK_INCLUDE_FILE_C_INCLUDE_DIRS}"
      OUTPUT_VARIABLE OUTPUT) 

    IF(${ARGC} EQUAL 3)
      SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS_SAVE})
    ENDIF(${ARGC} EQUAL 3)

    IF(${VARIABLE})
      MESSAGE(STATUS "Looking for ${INCLUDE} - found")
      SET(${VARIABLE} 1 CACHE INTERNAL "Have include ${INCLUDE}")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log 
        "Determining if the include file ${INCLUDE} "
        "exists passed with the following output:\n"
        "${OUTPUT}\n\n")
    ELSE(${VARIABLE})
      MESSAGE(STATUS "Looking for ${INCLUDE} - not found")
      SET(${VARIABLE} "" CACHE INTERNAL "Have include ${INCLUDE}")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log 
        "Determining if the include file ${INCLUDE} "
        "exists failed with the following output:\n"
        "${OUTPUT}\n\n")
    ENDIF(${VARIABLE})
#  ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
ENDMACRO(CHECK_INCLUDE_FILE)
