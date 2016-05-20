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

# See https://github.com/KitwareMedical/TubeTK/wiki/Dependencies

# Sanity checks
set(expected_nonempty_vars github_protocol git_protocol)
foreach(varname ${expected_nonempty_vars})
  if("${${varname}}" STREQUAL "")
    message(FATAL_ERROR "Variable '${varname}' is empty")
  endif()
endforeach()

# Cppcheck
set( Cppcheck_URL ${github_protocol}://github.com/KitwareMedical/cppcheck.git )
set( Cppcheck_HASH_OR_TAG a9c9482d6e1b42457aedf8065e21523654f46124 )

# TubeTK Image Viewer
set( ImageViewer_URL
  ${github_protocol}://github.com/KitwareMedical/ImageViewer.git )
set( ImageViewer_HASH_OR_TAG 2f728d05fe159828e6221e1420b9c3a00295315b )

# JsonCpp
# ${svn_protocol}://svn.code.sf.net/p/jsoncpp/code/trunk/jsoncpp )
# http://midas3.kitware.com/midas/download/bitstream/366544/JsonCpp_r276.tar.gz
set( JsonCpp_URL ${git_protocol}://github.com/KitwareMedical/jsoncpp.git )
set( JsonCpp_HASH_OR_TAG 110d054227e9eb63faad48a1fb6a828ad0670e61 )

# KWStyle
set( KWStyle_URL
  ${git_protocol}://github.com/Kitware/KWStyle.git )
set( KWStyle_HASH_OR_TAG f9e849c91e0613cbb16e102b7da805f4acd018cf )

# LIBSVM
set( LIBSVM_URL
  ${git_protocol}://github.com/KitwareMedical/libsvm.git )
set( LIBSVM_HASH_OR_TAG 9bc3630f0f15fed7a5119c228c4d260574b4b6b2 )

###########################################################
# ITK Modules
###########################################################

# TubeTKITK: Source already available in TubeTK project
set( TubeTKITK_URL ${TubeTK_SOURCE_DIR}/ITKModules/TubeTKITK )
set( TubeTKITK_HASH_OR_TAG "")

# MinimalPathExtraction
set( MinimalPathExtraction_URL
  ${git_protocol}://github.com/InsightSoftwareConsortium/ITKMinimalPathExtraction.git )
set( MinimalPathExtraction_HASH_OR_TAG
  aed93f4ac350ee8d0c3411e885fff86095443b7a )

set( TubeTK_ITK_MODULES
  TubeTKITK
  MinimalPathExtraction
  )

###########################################################
###########################################################
# The following were copied from Slicer on 2/29/2016
###########################################################
###########################################################

# Common Toolkit
set( CTK_URL ${github_protocol}://github.com/commontk/CTK.git )
set( CTK_HASH_OR_TAG caaf2c8cdee08e95bc823ab92865e1e9153dcc04 )

# Insight Segmentation and Registration Toolkit
set( ITK_URL ${github_protocol}://github.com/InsightSoftwareConsortium/ITK.git )
set( ITK_HASH_OR_TAG 58a7203f9ec431f9fe71ac087d0bd7e02b495634 )

# Slicer Execution Model
set( SlicerExecutionModel_URL
  ${github_protocol}://github.com/Slicer/SlicerExecutionModel.git )
set( SlicerExecutionModel_HASH_OR_TAG 112076be4f7ee59cc67099f12f2c4c16719070da )

# Visualization Toolkit (3D Slicer fork)
set( VTK_URL ${github_protocol}://github.com/Slicer/VTK.git )
set( VTK_HASH_OR_TAG fe92273888219edca422f3a308761ddcd2882e2b)
