/*=========================================================================

Library:   TubeTK

Copyright 2010 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=========================================================================*/

#ifndef __itktubePDFSegmenterSVMIO_h
#define __itktubePDFSegmenterSVMIO_h

#include "itktubeMetaClassPDF.h"

#include "itktubePDFSegmenterSVM.h"

namespace itk
{

namespace tube
{

/**
*
* Reads and Writes PDFSegmenterSVMIO Files, typically designated .mnda
* files
*
* \author Stephen R. Aylward
*
* \date August 29, 2013
*
*/
template< class TImage, class TLabelMap >
class PDFSegmenterSVMIO
{
public:

  typedef PDFSegmenterSVMIO< TImage, TLabelMap >  PDFSegmenterIOType;

  typedef PDFSegmenterSVM< TImage, TLabelMap >    PDFSegmenterType;

  PDFSegmenterSVMIO( void );

  PDFSegmenterSVMIO( const char * _headerName );

  PDFSegmenterSVMIO( const typename
    PDFSegmenterType::Pointer & _filter );

  ~PDFSegmenterSVMIO( void );

  virtual void PrintInfo( void ) const;

  virtual void CopyInfo( const PDFSegmenterIOType & _filterIO );

  virtual void Clear( void );

  virtual bool InitializeEssential( const typename
    PDFSegmenterType::Pointer & _filter );

  void SetPDFSegmenter( const typename
    PDFSegmenterType::Pointer & _filter );

  const typename PDFSegmenterType::Pointer GetPDFSegmenter( void ) const;

  virtual bool CanRead( const char * _headerName = NULL ) const;

  virtual bool Read( const char * _headerName = NULL );

  virtual bool Write( const char * _headerName = NULL );

protected:

  typename PDFSegmenterType::Pointer  m_PDFSegmenter;

}; // End class PDFSegmenterSVMIO

} // End namespace tube

} // End namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itktubePDFSegmenterSVMIO.hxx"
#endif

#endif // End !defined(__itktubePDFSegmenterSVMIO_h)
