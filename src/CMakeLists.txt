# Copyright 2009, Lloyd Hilaiel.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
# 
#  3. Neither the name of Lloyd Hilaiel nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

SET (SRCS orderly_node.c orderly_lex.c orderly_alloc.c orderly_buf.c
          orderly_parse.c orderly_writer.c orderly_reader.c)
SET (HDRS orderly_common.h orderly_lex.h orderly_parse.h)
SET (PUB_HDRS api/common.h api/node.h api/reader.h api/writer.h)

# set up some paths
SET (libDir ${CMAKE_CURRENT_BINARY_DIR}/../${ORDERLY_DIST_NAME}/lib)
SET (incDir ${CMAKE_CURRENT_BINARY_DIR}/../${ORDERLY_DIST_NAME}/include/orderly)

# set the output path for libraries
SET(LIBRARY_OUTPUT_PATH ${libDir})

ADD_LIBRARY(orderly_s STATIC ${SRCS} ${HDRS} ${PUB_HDRS})

ADD_LIBRARY(orderly SHARED ${SRCS} ${HDRS} ${PUB_HDRS})

#### setup shared library version number
SET_TARGET_PROPERTIES(orderly PROPERTIES
                      SOVERSION ${ORDERLY_MAJOR}
                      VERSION ${ORDERLY_MAJOR}.${ORDERLY_MINOR}.${ORDERLY_MICRO})

#### build up an sdk as a post build step

# create some directories
FILE(MAKE_DIRECTORY ${libDir})
FILE(MAKE_DIRECTORY ${incDir})

# copy public headers to output directory as a pre-build step
FOREACH (header ${PUB_HDRS})
  SET (header ${CMAKE_CURRENT_SOURCE_DIR}/${header})
  EXEC_PROGRAM(${CMAKE_COMMAND} ARGS -E copy_if_different ${header} ${incDir})
  ADD_CUSTOM_COMMAND(TARGET orderly_s PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${header} ${incDir}
      COMMAND ${CMAKE_COMMAND} -E echo copy_if_different ${header} ${incDir}      )
ENDFOREACH (header ${PUB_HDRS})

INCLUDE_DIRECTORIES(${incDir}/..)

IF(NOT WIN32)
  INSTALL(TARGETS orderly LIBRARY DESTINATION lib)
  INSTALL(TARGETS orderly_s ARCHIVE DESTINATION lib)
  INSTALL(DIRECTORY api/ DESTINATION include/orderly
          FILES_MATCHING PATTERN "*.h")
ENDIF()
