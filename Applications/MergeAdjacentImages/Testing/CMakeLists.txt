#############################################################################
#
# Library:   TubeTK
#
# Copyright 2010 Kitware Inc. 28 Corporate Drive,
# Clifton Park, NY, 12065, USA.
#
# All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
##############################################################################

include_regular_expression( "^.*$" )

include( Midas3FunctionAddTest )
set( MIDAS_REST_URL http://midas3.kitware.com/midas/api/rest )
set( MIDAS_KEY_DIR ${TubeTK_SOURCE_DIR}/MIDAS_Keys )

set( TEMP ${TubeTK_BINARY_DIR}/Temporary )

set( PROJ_EXE
 ${TubeTK_LAUNCHER} $<TARGET_FILE:${MODULE_NAME}> )

set( CompareImages_EXE
 ${TubeTK_LAUNCHER} $<TARGET_FILE:CompareImages> )

set( CompareTextFiles_EXE
 ${TubeTK_LAUNCHER} $<TARGET_FILE:CompareTextFiles> )

# Test1
Midas3FunctionAddTest( NAME ${MODULE_NAME}-Test1
             COMMAND ${PROJ_EXE}
               -a
               --saveTransform ${TEMP}/${MODULE_NAME}Test1.tfm
               MIDAS{ES0015_Large_Left.mha.md5}
               MIDAS{ES0015_Large_Right.mha.md5}
               ${TEMP}/${MODULE_NAME}Test1.mha )

Midas3FunctionAddTest( NAME ${MODULE_NAME}-Test1-Compare
             COMMAND ${CompareImages_EXE}
               -t ${TEMP}/${MODULE_NAME}Test1.mha
               -b MIDAS{ES0015_Large.mha.md5} )
set_property( TEST ${MODULE_NAME}-Test1-Compare
                      APPEND PROPERTY DEPENDS ${MODULE_NAME}-Test1 )

Midas3FunctionAddTest( NAME ${MODULE_NAME}-Test1-Tfm-Compare
             COMMAND ${CompareTextFiles_EXE}
               -t ${TEMP}/${MODULE_NAME}Test1.tfm
               -b MIDAS{${MODULE_NAME}Test1.tfm.md5}
               -d 0.1 )
set_property( TEST ${MODULE_NAME}-Test1-Tfm-Compare
                      APPEND PROPERTY DEPENDS ${MODULE_NAME}-Test1 )

# Test2
Midas3FunctionAddTest( NAME ${MODULE_NAME}-Test2
             COMMAND ${PROJ_EXE}
               -i 0
               -b 50,50
               MIDAS{ES0015_Large.mha.md5}
               MIDAS{ES0015_Large_Wo_offset.mha.md5}
               ${TEMP}/${MODULE_NAME}Test2.mha )

Midas3FunctionAddTest( NAME ${MODULE_NAME}-Test2-Compare
             COMMAND ${CompareImages_EXE}
               -t ${TEMP}/${MODULE_NAME}Test2.mha
               -b MIDAS{${MODULE_NAME}Test2.mha.md5} )
set_property( TEST ${MODULE_NAME}-Test2-Compare
                      APPEND PROPERTY DEPENDS ${MODULE_NAME}-Test2 )

# Test3
Midas3FunctionAddTest( NAME ${MODULE_NAME}-Test3
             COMMAND ${PROJ_EXE}
               -i 0
               -k 10
               -b 50,50
               MIDAS{ES0015_Large.mha.md5}
               MIDAS{ES0015_Large_Wo_offset.mha.md5}
               ${TEMP}/${MODULE_NAME}Test3.mha )

Midas3FunctionAddTest( NAME ${MODULE_NAME}-Test3-Compare
             COMMAND ${CompareImages_EXE}
               -t ${TEMP}/${MODULE_NAME}Test3.mha
               -b MIDAS{${MODULE_NAME}Test3.mha.md5} )
set_property( TEST ${MODULE_NAME}-Test3-Compare
                      APPEND PROPERTY DEPENDS ${MODULE_NAME}-Test3 )

# Test4
Midas3FunctionAddTest( NAME ${MODULE_NAME}-Test4
             COMMAND ${PROJ_EXE}
               -b 50,50
               -M
               -i 0
               MIDAS{ES0015_Large.mha.md5}
               MIDAS{ES0015_Large_Wo_offset.mha.md5}
               ${TEMP}/${MODULE_NAME}Test4.mha )

Midas3FunctionAddTest( NAME ${MODULE_NAME}-Test4-Compare
             COMMAND ${CompareImages_EXE}
               -t ${TEMP}/${MODULE_NAME}Test4.mha
               -b MIDAS{${MODULE_NAME}Test4.mha.md5} )
set_property( TEST ${MODULE_NAME}-Test4-Compare
                      APPEND PROPERTY DEPENDS ${MODULE_NAME}-Test4 )
