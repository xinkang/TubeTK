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
#ifndef __itkAnisotropicDiffusionTensorFunction_h
#define __itkAnisotropicDiffusionTensorFunction_h

#include "itkFiniteDifferenceFunction.h"
#include "vnl/vnl_matrix_fixed.h"
#include "itkDiffusionTensor3D.h"
#include "itkSymmetricSecondRankTensor.h"

namespace itk {

/** \class AnisotropicDiffusionTensorFunction
 * \brief This class is a function object that is used
 * to create a solver filter for edge enhancement diffusion equation
 *
 * \sa AnisotropicDiffusionTensorImageFilter
 * \ingroup FiniteDifferenceFunctions
 * \ingroup Functions
 */
template <class TImageType>
class ITK_EXPORT AnisotropicDiffusionTensorFunction
  : public FiniteDifferenceFunction<TImageType>
{
public:
  /** Standard class typedefs. */
  typedef AnisotropicDiffusionTensorFunction          Self;
  typedef FiniteDifferenceFunction<TImageType>        Superclass;
  typedef SmartPointer<Self>                          Pointer;
  typedef SmartPointer<const Self>                    ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods) */
  itkTypeMacro( AnisotropicDiffusionTensorFunction,
                                           FiniteDifferenceFunction );

  /** Extract some parameters from the superclass. */
  itkStaticConstMacro(ImageDimension, unsigned int, Superclass::ImageDimension);

  /** Convenient typedefs. */
  typedef double                                       TimeStepType;
  typedef typename Superclass::ImageType               ImageType;
  typedef typename Superclass::PixelType               PixelType;
  typedef double                                       ScalarValueType;
  typedef typename Superclass::RadiusType              RadiusType;
  typedef typename Superclass::NeighborhoodType        NeighborhoodType;
  typedef typename Superclass::FloatOffsetType         FloatOffsetType;

  typedef DiffusionTensor3D< double >                  DiffusionTensorType;
  typedef itk::Image< DiffusionTensorType, 3 >         DiffusionTensorImageType;

  /** The default boundary condition for finite difference
   * functions that is used unless overridden in the Evaluate() method. */
  typedef ZeroFluxNeumannBoundaryCondition<DiffusionTensorImageType>
    DefaultBoundaryConditionType;

  /** Define diffusion image iterator/neighborhood types */
  typedef ConstNeighborhoodIterator<DiffusionTensorImageType,
                                    DefaultBoundaryConditionType>
                                           DiffusionTensorNeighborhoodType;
  typedef ImageRegionIterator<DiffusionTensorImageType>
      DiffusionTensorImageRegionType;

  /** Tensor pixel type */
  typedef itk::SymmetricSecondRankTensor< double >  TensorPixelType;

  /** A global data type for this class of equations.  Used to store
   * values that are needed in calculating the time step and other intermediate
   * products such as derivatives that may be used by virtual functions called
   * from ComputeUpdate.  Caching these values here allows the ComputeUpdate
   * function to be const and thread safe.*/
  struct GlobalDataStruct
    {
    /** Hessian matrix */
    vnl_matrix_fixed<ScalarValueType,
                     itkGetStaticConstMacro(ImageDimension),
                     itkGetStaticConstMacro(ImageDimension)> m_dxy;

    /** diffusion tensor first derivative matrix */
    vnl_matrix_fixed<ScalarValueType,
                     itkGetStaticConstMacro(ImageDimension),
                     itkGetStaticConstMacro(ImageDimension)> m_DT_dxy;

    /** Array of first derivatives*/
    ScalarValueType m_dx[itkGetStaticConstMacro(ImageDimension)];

    ScalarValueType m_GradMagSqr;
    };

  /** Compute the equation value. */
  virtual PixelType ComputeUpdate(const NeighborhoodType &neighborhood,
                                  void *globalData,
                                  const FloatOffsetType& = FloatOffsetType(0.0));

  /** Compute the equation value. */
  virtual PixelType ComputeUpdate(
                     const NeighborhoodType &neighborhood,
                     const DiffusionTensorNeighborhoodType &neighborhoodTensor,
                     void *globalData,
                     const FloatOffsetType& = FloatOffsetType(0.0));

  /** Compute the equation value, using precomputed first derivatives for the
      diffusion tensor. */
  virtual PixelType ComputeUpdate(
                     const NeighborhoodType &neighborhood,
                     const DiffusionTensorNeighborhoodType &neighborhoodTensor,
                     const DiffusionTensorImageRegionType &tensorDerivative,
                     void *globalData,
                     const FloatOffsetType& = FloatOffsetType(0.0));

  /** Computes the time step for an update given a global data structure. */
  virtual TimeStepType ComputeGlobalTimeStep(void *GlobalData) const;

  /** Returns a pointer to a global data structure that is passed to this
   * object from the solver at each calculation.*/
  virtual void *GetGlobalDataPointer() const
    {
    GlobalDataStruct *ans = new GlobalDataStruct();
    return ans;
    }

  virtual void ReleaseGlobalDataPointer(void *GlobalData) const
    { delete (GlobalDataStruct *) GlobalData; }

  /** Set/Get the time step. For this class of anisotropic diffusion filters,
      the time-step is supplied by the user and remains fixed for all
      updates. */
  void SetTimeStep(const TimeStepType &t)
    {
    m_TimeStep = t;
    }

  const TimeStepType &GetTimeStep() const
    {
    return m_TimeStep;
    }

  /** Utility function to check whether the timestep is stable, optionally based
    * on the spacing of the given image */
  template< class TPixel, unsigned int VImageDimension >
  void CheckTimeStepStability(
      const itk::Image< TPixel, VImageDimension > * input,
      bool useImageSpacing );

  /** Computes the first derivative of a diffusion tensor image. */
  void ComputeDiffusionFirstDerivative(
      const DiffusionTensorNeighborhoodType &neighborhoodTensor,
      DiffusionTensorImageRegionType &derivativeTensor ) const;

protected:
  AnisotropicDiffusionTensorFunction();

  virtual ~AnisotropicDiffusionTensorFunction() {}
  void PrintSelf(std::ostream &s, Indent indent) const;

  /** Slices for the ND neighborhood. */
  std::slice x_slice[itkGetStaticConstMacro(ImageDimension)];

  /** The offset of the center pixel in the neighborhood. */
  unsigned int m_Center;

  /** Stride length along the y-dimension. */
  unsigned int m_xStride[itkGetStaticConstMacro(ImageDimension)];

  /** Defines various positions surrounding the center pixel in an image
    iterator. */
  unsigned int m_positionA[itkGetStaticConstMacro(ImageDimension)];
  unsigned int m_positionB[itkGetStaticConstMacro(ImageDimension)];
  unsigned int m_positionAa[itkGetStaticConstMacro(ImageDimension)]
      [itkGetStaticConstMacro(ImageDimension)];
  unsigned int m_positionBa[itkGetStaticConstMacro(ImageDimension)]
      [itkGetStaticConstMacro(ImageDimension)];
  unsigned int m_positionCa[itkGetStaticConstMacro(ImageDimension)]
      [itkGetStaticConstMacro(ImageDimension)];
  unsigned int m_positionDa[itkGetStaticConstMacro(ImageDimension)]
      [itkGetStaticConstMacro(ImageDimension)];

  /** Computes the first and second derivatives of an intensity image. */
  void ComputeIntensityFirstAndSecondDerivatives(
      const NeighborhoodType &neighborhoodIntensity,
      GlobalDataStruct *globalData ) const;

  /** Compute the first derivative of a diffusion image */
  TensorPixelType ComputeDiffusionFirstDerivative(
      const DiffusionTensorNeighborhoodType &neighborhoodTensor,
      GlobalDataStruct *globalData ) const;

  /** Computes the final update term based on the results of the first and
    * second derivative computations */
  PixelType ComputeFinalUpdateTerm(
      const DiffusionTensorNeighborhoodType &neighborhoodTensor,
      const GlobalDataStruct* globalData ) const;

  /** Copies a diffusion tensor derivative into a globalDataStruct's diffusion
    tensor first derivative field */
  void CopyTensorToGlobalData( const DiffusionTensorImageRegionType &tensor,
                               GlobalDataStruct* globalData ) const;

private:
  //purposely not implemented
  AnisotropicDiffusionTensorFunction(const Self&);
  void operator=(const Self&);   //purposely not implemented

  TimeStepType    m_TimeStep;
};

} // namespace itk

#if ITK_TEMPLATE_EXPLICIT
# include "Templates/itkAnisotropicDiffusionTensorFunction+-.h"
#endif

#if ITK_TEMPLATE_TXX
# include "itkAnisotropicDiffusionTensorFunction.txx"
#endif

#endif
