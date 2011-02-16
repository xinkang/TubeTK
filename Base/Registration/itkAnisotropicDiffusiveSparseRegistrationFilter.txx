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
#ifndef __itkAnisotropicDiffusiveSparseRegistrationFilter_txx
#define __itkAnisotropicDiffusiveSparseRegistrationFilter_txx

#include "itkAnisotropicDiffusiveSparseRegistrationFilter.h"

namespace itk
{

/**
 * Constructor
 */
template < class TFixedImage, class TMovingImage, class TDeformationField >
AnisotropicDiffusiveSparseRegistrationFilter
< TFixedImage, TMovingImage, TDeformationField >
::AnisotropicDiffusiveSparseRegistrationFilter()
{
  // Initialize attributes to NULL
  m_BorderSurface                               = 0;
  m_NormalMatrixImage                           = 0;
  m_WeightImage                                 = 0;

  // Lambda for exponential decay used to calculate weight from distance
  m_lambda = -0.01;
}

/**
 * PrintSelf
 */
template < class TFixedImage, class TMovingImage, class TDeformationField >
void
AnisotropicDiffusiveSparseRegistrationFilter
  < TFixedImage, TMovingImage, TDeformationField >
::PrintSelf( std::ostream& os, Indent indent ) const
{
  Superclass::PrintSelf( os, indent );
  if( m_BorderSurface )
    {
    os << indent << "Border surface:" << std::endl;
    m_BorderSurface->Print( os );
    }
  if( m_NormalMatrixImage )
    {
    os << indent << "Normal vector image:" << std::endl;
    m_NormalMatrixImage->Print( os, indent );
    }
  if( m_WeightImage )
    {
    os << indent << "Weight image:" << std::endl;
    m_WeightImage->Print( os, indent );
    }
  os << indent << "lambda: " << m_lambda << std::endl;
}

/**
 * Setup the pointers for the deformation component images
 */
template < class TFixedImage, class TMovingImage, class TDeformationField >
void
AnisotropicDiffusiveSparseRegistrationFilter
  < TFixedImage, TMovingImage, TDeformationField >
::InitializeDeformationComponentAndDerivativeImages()
{
  assert( this->GetComputeRegularizationTerm() );
  assert( this->GetOutput() );

  // The output will be used as the template to allocate the images we will
  // use to store data computed before/during the registration
  OutputImagePointer output = this->GetOutput();

  // Setup pointers to the deformation component images - we have the
  // TANGENTIAL component, which is the entire deformation field, and the
  // NORMAL component, which is the deformation vectors projected onto their
  // normals
  // There are four terms that share these two deformation component images
  this->SetDeformationComponentImage( SMOOTH_TANGENTIAL, this->GetOutput() );
  this->SetDeformationComponentImage( PROP_TANGENTIAL, this->GetOutput() );

  DeformationFieldPointer normalDeformationField = DeformationFieldType::New();
  this->AllocateSpaceForImage( normalDeformationField, output );
  this->SetDeformationComponentImage( SMOOTH_NORMAL, normalDeformationField );
  this->SetDeformationComponentImage( PROP_NORMAL, normalDeformationField );

  // Setup the first and second order deformation component images
  // The two TANGENTIAL and two NORMAL components share images, so we allocate
  // images only for the SMOOTH terms and set the pointers for the PROP terms
  // to zero.  They will be pointed to the correct images when the images are
  // actually filled in.
  for( int i = 0; i < this->GetNumberOfTerms(); i++ )
    {
    ScalarDerivativeImageArrayType firstOrderArray;
    TensorDerivativeImageArrayType secondOrderArray;
    if( i % 2 == 0 )
      {
      for( int j = 0; j < ImageDimension; j++ )
        {
        firstOrderArray[j] = ScalarDerivativeImageType::New();
        this->AllocateSpaceForImage( firstOrderArray[j], output );
        secondOrderArray[j] = TensorDerivativeImageType::New();
        this->AllocateSpaceForImage( secondOrderArray[j], output );
        }
      }
    else
      {
      for( int j = 0; j < ImageDimension; j++ )
        {
        firstOrderArray[j] = 0;
        secondOrderArray[j] = 0;
        }
      }
    this->SetDeformationComponentFirstOrderDerivativeArray( i,
                                                            firstOrderArray );
    this->SetDeformationComponentSecondOrderDerivativeArray( i,
                                                             secondOrderArray );
    }
}

/**
 * All other initialization done before the initialize / calculate change /
 * apply update loop
 */
template < class TFixedImage, class TMovingImage, class TDeformationField >
void
AnisotropicDiffusiveSparseRegistrationFilter
  < TFixedImage, TMovingImage, TDeformationField >
::SetupNormalMatrixAndWeightImages()
{
  assert( this->GetComputeRegularizationTerm() );
  assert( this->GetOutput() );

  // The output will be used as the template to allocate the images we will
  // use to store data computed before/during the registration
  OutputImagePointer output = this->GetOutput();

  // If a normal vector image or weight image was supplied by the user, check
  // that it matches the output
  if( ( m_NormalMatrixImage
        && !this->CompareImageAttributes( m_NormalMatrixImage, output ) )
    || ( m_WeightImage
         && !this->CompareImageAttributes( m_WeightImage, output ) ) )
      {
    itkExceptionMacro( << "Normal matrix image and/or weight image must have "
                       << "the same attributes as the output deformation "
                       << "field" );
    }

  // Whether or not we must compute the normal vector and/or weight images
  bool computeNormals = !m_NormalMatrixImage;
  bool computeWeights = !m_WeightImage;

  // Compute the normal vector and/or weight images if required
  if( computeNormals || computeWeights )
    {
    // Ensure we have a border surface to work with
    if( !this->GetBorderSurface() )
      {
      itkExceptionMacro( << "You must provide a border surface, or both a "
                         << "normal matrix image and a weight image" );
      }

    // Compute the normals for the surface
    this->ComputeBorderSurfaceNormals();

    // Allocate the normal vector and/or weight images
    if( computeNormals )
      {
      m_NormalMatrixImage = NormalMatrixImageType::New();
      this->AllocateSpaceForImage( m_NormalMatrixImage, output );
      }
    if( computeWeights )
      {
      m_WeightImage = WeightImageType::New();
      this->AllocateSpaceForImage( m_WeightImage, output );
      }

    // Actually compute the normal vectors and/or weights
    this->ComputeNormalMatrixAndWeightImages( computeNormals, computeWeights );
    }
}

/**
 * Compute the normals for the border surface
 */
template < class TFixedImage, class TMovingImage, class TDeformationField >
void
AnisotropicDiffusiveSparseRegistrationFilter
  < TFixedImage, TMovingImage, TDeformationField >
::ComputeBorderSurfaceNormals()
{
  assert( m_BorderSurface );
  vtkPolyDataNormals * normalsFilter = vtkPolyDataNormals::New();
  normalsFilter->ComputePointNormalsOn();
  normalsFilter->ComputeCellNormalsOff();
  //normalsFilter->SetFeatureAngle(30); // TODO
  normalsFilter->SetInput( m_BorderSurface );
  normalsFilter->Update();
  m_BorderSurface = normalsFilter->GetOutput();
  normalsFilter->Delete();

  // Make sure we now have the normals
  if ( !m_BorderSurface->GetPointData() )
    {
    itkExceptionMacro( << "Border surface does not contain point data" );
    }
  else if ( !m_BorderSurface->GetPointData()->GetNormals() )
    {
    itkExceptionMacro( << "Border surface point data does not have normals" );
    }
}

/**
 * Updates the border normals and the weighting factor w
 */
template < class TFixedImage, class TMovingImage, class TDeformationField >
void
AnisotropicDiffusiveSparseRegistrationFilter
  < TFixedImage, TMovingImage, TDeformationField >
::ComputeNormalVectorAndWeightImages( bool computeNormals, bool computeWeights )
{
  assert( this->GetComputeRegularizationTerm() );
  assert( m_BorderSurface->GetPointData()->GetNormals() );
  assert( m_NormalMatrixImage );
  assert( m_WeightImage );

  std::cout << "Computing normals and weights... " << std::endl;

  // Setup iterators over the normal vector and weight images
  NormalMatrixImageRegionType normalIt(
      m_NormalMatrixImage, m_NormalMatrixImage->GetLargestPossibleRegion() );
  WeightImageRegionType weightIt(m_WeightImage,
                                 m_WeightImage->GetLargestPossibleRegion() );

  // Get the normals from the polydata
  vtkPointLocator * pointLocator = vtkPointLocator::New();
  pointLocator->SetDataSet( m_BorderSurface );
  vtkSmartPointer< vtkDataArray > normalData
      = m_BorderSurface->GetPointData()->GetNormals();

  // The normal vector image will hold the normal of the closest point of the
  // surface polydata, and the weight image will be a function of the distance
  // between the voxel and this closest point

  itk::Point< double, ImageDimension >  imageCoordAsPoint;
  imageCoordAsPoint.Fill( 0 );
  double                                imageCoord[ImageDimension];
  double                                borderCoord[ImageDimension];
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    imageCoord[i] = 0;
    borderCoord[i] = 0;
    }
  vtkIdType                             id = 0;
  WeightType                            distance = 0;
  NormalVectorType                      normalVector;
  normalVector.Fill(0);
  NormalMatrixType                      normalMatrix;
  normalMatrix.Fill(0);
  WeightComponentType                   weight = 0;
  WeightType                            weightMatrix;
  weightMatrix.Fill(0);

  // Determine the normals of and the distances to the nearest border
  for( normalIt.GoToBegin(), weightIt.GoToBegin();
       !normalIt.IsAtEnd();
       ++normalIt, ++weightIt )
    {
    normalMatrix.Fill(0);
    weightMatrix.Fill(0);

    // Find the normal of the surface point that is closest to the current voxel
    m_NormalMatrixImage->TransformIndexToPhysicalPoint( normalIt.GetIndex(),
                                                        imageCoordAsPoint );
    for( unsigned int i = 0; i < ImageDimension; i++ )
      {
      imageCoord[i] = imageCoordAsPoint[i];
      }
    id = pointLocator->FindClosestPoint( imageCoord );
    normalVector = normalData->GetTuple( id );
    if( computeNormals )
      {
      for( int i = 0; i < ImageDimension; i++ )
        {
        normalMatrix(i,0) = normalVector[i];
        }
      normalIt.Set( normalMatrix );
      }

    // Calculate distance between the current coordinate and the border surface
    // coordinate
    m_BorderSurface->GetPoint( id, borderCoord );
    distance = 0.0;
    for( unsigned int i = 0; i < ImageDimension; i++ )
      {
      distance += pow( imageCoord[i] - borderCoord[i], 2 );
      }
    distance = sqrt( distance );

    // The weight image will temporarily store distances
    if( computeWeights )
      {
      weightMatrix(0,0) = distance;
      weightIt.Set( weightMatrix );
      }
    }

  // Clean up memory
  pointLocator->Delete();

  // Smooth the normals to handle corners (because we are choosing the
  // closest point in the polydata
  if( computeNormals )
    {
    //  typedef itk::RecursiveGaussianImageFilter
    //      < NormalVectorImageType, NormalVectorImageType >
    //      NormalSmoothingFilterType;
    //  typename NormalSmoothingFilterType::Pointer normalSmooth
    //      = NormalSmoothingFilterType::New();
    //  normalSmooth->SetInput( m_NormalVectorImage );
    //  double normalSigma = 3.0;
    //  normalSmooth->SetSigma( normalSigma );
    //  normalSmooth->Update();
    //  m_NormalVectorImage = normalSmooth->GetOutput();
    }

  // Smooth the distance image to avoid "streaks" from faces of the polydata
  // (because we are choosing the closest point in the polydata)
  if( computeWeights )
    {
//    double weightSmoothingSigma = 1.0;
//    typedef itk::SmoothingRecursiveGaussianImageFilter
//        < WeightImageType, WeightImageType > WeightSmoothingFilterType;
//    typename WeightSmoothingFilterType::Pointer weightSmooth
//        = WeightSmoothingFilterType::New();
//    weightSmooth->SetInput( m_WeightImage );
//    weightSmooth->SetSigma( weightSmoothingSigma );
//    weightSmooth->Update();
//    m_WeightImage = weightSmooth->GetOutput();

    // Iterate through the weight image and compute the weight from the
    WeightImageRegionType weightIt(
        m_WeightImage, m_WeightImage->GetLargestPossibleRegion() );
    for( weightIt.GoToBegin(); !weightIt.IsAtEnd(); ++weightIt )
      {
      weightMatrix = weightIt.Get();
      weight = this->ComputeWeightFromDistance( weightMatrix(0,0) );
      weightMatrix(0,0) = weight;
      weightIt.Set( weightMatrix );
      }
    }

  std::cout << "Finished computing normals and weights." << std::endl;
}

/**
 * Calculates the weighting between the anisotropic diffusive and diffusive
 * regularizations, based on a given distance from a voxel to the border
 */
template < class TFixedImage, class TMovingImage, class TDeformationField >
typename AnisotropicDiffusiveSparseRegistrationFilter
  < TFixedImage, TMovingImage, TDeformationField >
::WeightComponentType
AnisotropicDiffusiveSparseRegistrationFilter
  < TFixedImage, TMovingImage, TDeformationField >
::ComputeWeightFromDistance( const WeightComponentType distance ) const
{
  return exp( m_lambda * distance );
}


} // end namespace itk

#endif
