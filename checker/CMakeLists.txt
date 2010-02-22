# Copyright 2010, Greg Olszewski and Lloyd Hilaiel.
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
#  3. Neither the name of Greg Olszewski and Lloyd Hilaiel nor the names of its
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

# set up a paths
SET (binDir ${CMAKE_CURRENT_BINARY_DIR}/../${ORDERLY_DIST_NAME}/bin)
SET (exeName orderly_check)

# create a directories
FILE(MAKE_DIRECTORY ${binDir})

SET (SRCS checker.c)

# use the library we build, duh.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../${ORDERLY_DIST_NAME}/include)
LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../${ORDERLY_DIST_NAME}/lib)

ADD_EXECUTABLE(${exeName} ${SRCS})

TARGET_LINK_LIBRARIES(${exeName} orderly)

# copy the binary into the output directory
GET_TARGET_PROPERTY(binPath ${exeName} LOCATION)

ADD_CUSTOM_COMMAND(TARGET ${exeName} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${binPath} ${binDir})

IF (NOT WIN32)
  INSTALL(TARGETS ${exeName} RUNTIME DESTINATION bin)
ENDIF ()
