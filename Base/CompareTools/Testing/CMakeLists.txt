##############################################################################
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

include( Midas3FunctionAddTest )
set( MIDAS_REST_URL http://midas3.kitware.com/midas/api/rest )
set( MIDAS_KEY_DIR ${TubeTK_SOURCE_DIR}/MIDAS_Keys )

set( TEMP ${TubeTK_BINARY_DIR}/Temporary )

set( CompareImages_EXE
  ${TubeTK_LAUNCHER} $<TARGET_FILE:CompareImages> )

set( CompareJsonFiles_EXE
  ${TubeTK_LAUNCHER} $<TARGET_FILE:CompareJsonFiles> )

Midas3FunctionAddTest( NAME CompareImages-Test1
  COMMAND ${CompareImages_EXE}
    -t MIDAS{ES0015_Large.mha.md5}
    -b MIDAS{ES0015_Large.mha.md5} )

Midas3FunctionAddTest( NAME CompareImages-Test2
  COMMAND ${CompareImages_EXE}
    -n 1000
    -r 1
    -i 50
    -t MIDAS{im0001.crop.mha.md5}
    -b MIDAS{im0001_n10.crop.mha.md5} )

Midas3FunctionAddTest( NAME CompareImages-Test3
  COMMAND ${CompareImages_EXE}
    -t MIDAS{im0001.mha.md5}
    -b MIDAS{im0001_n10.crop.mha.md5} )
set_tests_properties( CompareImages-Test3 PROPERTIES WILL_FAIL true )

Midas3FunctionAddTest( NAME CompareImages-Test4
  COMMAND ${CompareImages_EXE}
    -t MIDAS{im0001.crop.mha.md5}
    -b MIDAS{im0001_n10.crop.mha.md5},MIDAS{im0001.crop.mha.md5} )

Midas3FunctionAddTest( NAME CompareImages-Test5
  COMMAND ${CompareImages_EXE}
    -t MIDAS{im0001.crop.mha.md5}
    -b MIDAS{im0001_n10.crop.mha.md5},MIDAS{im0001_n20.crop.mha.md5} )
set_tests_properties( CompareImages-Test5 PROPERTIES WILL_FAIL true )

Midas3FunctionAddTest( NAME CompareImages-Test6
  COMMAND ${CompareImages_EXE}
    -t MIDAS{im0001_n10.crop.mha.md5}
    -b ${TEMP}/filedoesnotexist.mha )
set_tests_properties( CompareImages-Test6 PROPERTIES WILL_FAIL true )

Midas3FunctionAddTest( NAME CompareImages-Test7
  COMMAND ${CompareImages_EXE}
    -c 1.0e-4
    -t MIDAS{ResampleImageTest2.mha.md5}
    -b MIDAS{ResampleImageTest2SpacingOff.mha.md5} )

if( TubeTK_USE_JsonCpp )
  add_test( NAME CompareJsonFiles-Test1
    COMMAND ${CompareJsonFiles_EXE}
      -t ${CMAKE_CURRENT_SOURCE_DIR}/CompareJsonFilesTestTest1.json
      -b ${CMAKE_CURRENT_SOURCE_DIR}/CompareJsonFilesTestBaseline1.json )

  add_test( NAME CompareJsonFiles-Test2
    COMMAND ${CompareJsonFiles_EXE}
      -t ${CMAKE_CURRENT_SOURCE_DIR}/CompareJsonFilesTestTest2.json
      -b ${CMAKE_CURRENT_SOURCE_DIR}/CompareJsonFilesTestBaseline1.json )
  set_tests_properties( CompareJsonFiles-Test2 PROPERTIES WILL_FAIL true )

  add_test( NAME CompareJsonFiles-Test3
    COMMAND ${CompareJsonFiles_EXE}
      -t ${CMAKE_CURRENT_SOURCE_DIR}/CompareJsonFilesTestTest2.json
      -b ${CMAKE_CURRENT_SOURCE_DIR}/CompareJsonFilesTestBaseline2.json )
endif( TubeTK_USE_JsonCpp )
