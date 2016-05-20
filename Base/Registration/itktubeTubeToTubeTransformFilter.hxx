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

#ifndef __itktubeTubeToTubeTransformFilter_hxx
#define __itktubeTubeToTubeTransformFilter_hxx

#include "itktubeTubeToTubeTransformFilter.h"

#include <itkSpatialObjectFactory.h>

namespace itk
{

namespace tube
{

template< class TTransformType, unsigned int TDimension >
TubeToTubeTransformFilter< TTransformType, TDimension >
::TubeToTubeTransformFilter( void )
{
  m_Output = 0;
  m_OutputIndexToObjectTransform = 0;
  m_Transform = 0;

  SpatialObjectFactoryBase::RegisterDefaultSpatialObjects();
  SpatialObjectFactory< SpatialObject< TDimension > >::
    RegisterSpatialObject();
  SpatialObjectFactory< GroupType >::RegisterSpatialObject();
  SpatialObjectFactory< TubeType >::RegisterSpatialObject();
}

/**
 * Apply the transformation to the tube
 */
template< class TTransformType, unsigned int TDimension >
void
TubeToTubeTransformFilter< TTransformType, TDimension >
::Update( void )
{
  m_Output = GroupType::New();
  m_Output->CopyInformation( this->GetInput() );

  typedef typename SpatialObject< TDimension >::ChildrenListType
    ChildrenListType;
  ChildrenListType * children = this->GetInput()->GetChildren(0);
  typename ChildrenListType::const_iterator it = children->begin();
  while( it != children->end() )
    {
    this->UpdateLevel( *it, m_Output );
    ++it;
    }
  delete children;
}

/**
 * Apply the transformation to the tube
 */
template< class TTransformType, unsigned int TDimension >
void
TubeToTubeTransformFilter< TTransformType, TDimension >
::UpdateLevel( SpatialObject< TDimension > * inputSO,
  SpatialObject< TDimension > * parentSO )
{
  const std::string spatialObjectTypeName = inputSO->
    GetSpatialObjectTypeAsString();
  LightObject::Pointer outputLO = ObjectFactoryBase::CreateInstance(
    spatialObjectTypeName.c_str() );

  typename SpatialObject< TDimension >::Pointer outputSO =
    dynamic_cast< SpatialObject< TDimension > * >( outputLO.GetPointer() );
  if( outputSO.IsNull() )
    {
    itkExceptionMacro( << "Could not create an instance of "
      << spatialObjectTypeName << ". The usual cause of this error is not"
      << "registering the SpatialObject with SpatialFactory." );
    }

  // Correct for extra reference count from CreateInstance().
  outputSO->UnRegister();

  // We make the copy and sub-sample if it is a tube.
  TubeType * inputSOAsTube = dynamic_cast< TubeType * >(
    inputSO );
  if( inputSOAsTube != NULL )
    {
    TubeType * outputSOAsTube = dynamic_cast< TubeType * >(
      outputSO.GetPointer() );

    Point<double, TDimension> inputObjectPoint;
    Point<double, TDimension> worldPoint;
    Point<double, TDimension> transformedWorldPoint;
    Point<double, TDimension> inputPoint;
    Point<double, TDimension> outputPoint;
    CovariantVector< double, TDimension > normal1;
    CovariantVector< double, TDimension > normal2;

    inputSOAsTube->ComputeObjectToWorldTransform();

    typename TubeType::TransformType::Pointer inputIndexToObjectTransform =
      inputSOAsTube->GetIndexToObjectTransform();

    typename TubeType::TransformType::Pointer inputIndexToWorldTransform =
      inputSOAsTube->GetIndexToWorldTransform();

    typename TubeType::TransformType::Pointer inputObjectToWorldTransform =
      inputSOAsTube->GetObjectToWorldTransform();

    outputSOAsTube->CopyInformation( inputSOAsTube );
    outputSOAsTube->Clear();

    if( m_OutputIndexToObjectTransform.IsNotNull() )
      {
      typename TubeType::TransformType::Pointer tfm = TubeType::
        TransformType::New();
      tfm->SetIdentity();
      tfm->SetMatrix( m_OutputIndexToObjectTransform->GetMatrix() );
      tfm->SetOffset( m_OutputIndexToObjectTransform->GetOffset() );
      outputSOAsTube->SetObjectToParentTransform( tfm );
      outputSOAsTube->SetSpacing( m_OutputIndexToObjectTransform->
        GetScale() );
      }
    else
      {
      outputSOAsTube->SetSpacing( inputSOAsTube->GetSpacing() );
      }

    outputSOAsTube->ComputeObjectToWorldTransform();

    typename TubeType::TransformType::Pointer
      outputInverseIndexToWorldTransform = TubeType::TransformType::New();
    outputSOAsTube->GetIndexToWorldTransform()->GetInverse(
      outputInverseIndexToWorldTransform );

    typename TubeType::TransformType::Pointer
      outputInverseObjectToWorldTransform = TubeType::TransformType::New();
    outputSOAsTube->GetObjectToWorldTransform()->GetInverse(
      outputInverseObjectToWorldTransform );

    typedef typename TubeType::PointListType      TubePointListType;
    TubePointListType tubePointList = inputSOAsTube->GetPoints();
    typename TubePointListType::const_iterator tubePointIterator =
      tubePointList.begin();

    while( tubePointIterator != tubePointList.end() )
      {
      inputPoint = (*tubePointIterator).GetPosition();
      inputObjectPoint = inputIndexToObjectTransform
        ->TransformPoint( inputPoint );
      worldPoint = inputIndexToWorldTransform->TransformPoint(
        inputPoint );

      transformedWorldPoint = m_Transform->TransformPoint( worldPoint );

      outputPoint = outputInverseIndexToWorldTransform->
        TransformPoint( transformedWorldPoint );

      VesselTubeSpatialObjectPoint<TDimension> pnt;

      pnt.SetID( tubePointIterator->GetID() );
      pnt.SetColor( tubePointIterator->GetColor() );

      pnt.SetPosition( outputPoint );

      // get both normals
      typename TubeType::CovariantVectorType n1 = tubePointIterator
        ->GetNormal1();
      typename TubeType::CovariantVectorType n2 = tubePointIterator
        ->GetNormal2();

      // only try transformation of normals if both are non-zero
      if( !n1.GetVnlVector().is_zero() && !n2.GetVnlVector().is_zero() )
        {
        n1 = inputObjectToWorldTransform->TransformCovariantVector(
          n1, inputObjectPoint );
        n2 = inputObjectToWorldTransform->TransformCovariantVector(
          n2, inputObjectPoint );
        n1 = m_Transform->TransformCovariantVector( n1, worldPoint );
        n2 = m_Transform->TransformCovariantVector( n2, worldPoint );
        n1 = outputInverseObjectToWorldTransform
          ->TransformCovariantVector( n1, transformedWorldPoint );
        n2 = outputInverseObjectToWorldTransform
          ->TransformCovariantVector( n2, transformedWorldPoint );
        n1.Normalize();
        n2.Normalize();
        pnt.SetNormal1( n1 );
        pnt.SetNormal2( n2 );
        }

      typename TubeType::VectorType tang = tubePointIterator->
        GetTangent();
      if( !tang.GetVnlVector().is_zero() )
        {
        tang = inputObjectToWorldTransform->TransformVector(
          tang, inputObjectPoint );
        tang = m_Transform->TransformVector( tang, worldPoint );
        tang = outputInverseObjectToWorldTransform->
          TransformVector( tang, transformedWorldPoint );
        tang.Normalize();
        pnt.SetTangent( tang );
        }

      typename TubeType::VectorType radi;
      for( unsigned int i=0; i<TDimension; ++i )
        {
        radi[i] = tubePointIterator->GetRadius();
        }
      radi = inputIndexToWorldTransform->TransformVector( radi,
        inputPoint );
      radi = m_Transform->TransformVector( radi, worldPoint );
      radi = outputInverseIndexToWorldTransform->
        TransformVector( radi, worldPoint );
      pnt.SetRadius( radi[0] );

      pnt.SetMedialness( (*tubePointIterator).GetMedialness() );
      pnt.SetRidgeness( (*tubePointIterator).GetRidgeness() );
      pnt.SetBranchness( (*tubePointIterator).GetBranchness() );

      outputSOAsTube->GetPoints().push_back( pnt );

      ++tubePointIterator;
      }
    }
  else
    {
    outputSO->CopyInformation( inputSO );
    }
  std::cout << "Adding object " << outputSO->GetId() << std::endl;
  parentSO->AddSpatialObject( outputSO );

  typedef typename SpatialObject< TDimension >::ChildrenListType
    ChildrenListType;
  ChildrenListType * children = inputSO->GetChildren(0);
  typename ChildrenListType::const_iterator it = children->begin();
  while( it != children->end() )
    {
    this->UpdateLevel( *it, outputSO );
    ++it;
    }
  delete children;
}

template< class TTransformType, unsigned int TDimension >
void
TubeToTubeTransformFilter< TTransformType,TDimension >
::PrintSelf( std::ostream& os, Indent indent ) const
{
  Superclass::PrintSelf(os,indent);

  os << indent << "Transformation: " << m_Transform << std::endl;
  os << indent << "OutputIndexToObject Transform: " <<
    m_OutputIndexToObjectTransform << std::endl;
}

} // End namespace tube

} // End namespace itk

#endif // End !defined(__itktubeTubeToTubeTransformFilter_hxx)
