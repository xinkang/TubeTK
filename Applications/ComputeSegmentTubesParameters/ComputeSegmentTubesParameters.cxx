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

#include "tubeCLIFilterWatcher.h"
#include "tubeCLIProgressReporter.h"
#include "tubeMessage.h"

#include <itkTimeProbesCollectorBase.h>
#include <itkImageFileWriter.h>
#include <itkImageFileReader.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkRescaleIntensityImageFilter.h>

#include <itktubeRidgeExtractor.h>

#include <itktubeMetaTubeExtractor.h>

#include <algorithm>

// Must include CLP before including tubeCLIHelperFunctions
#include "ComputeSegmentTubesParametersCLP.h"

// Must do a forward declaration of DoIt before including
// tubeCLIHelperFunctions
template< class TPixel, unsigned int VDimension >
int DoIt( int argc, char * argv[] );

// Must follow include of "...CLP.h" and forward declaration of int DoIt( ... ).
#include "tubeCLIHelperFunctions.h"

template< class ImageT >
int WriteOutputImage( std::string & fileName, typename ImageT::Pointer
  & image )
{
  typedef itk::ImageFileWriter< ImageT  >  WriterType;

  typename WriterType::Pointer writer = WriterType::New();

  writer->SetInput( image );
  writer->SetFileName( fileName.c_str() );
  writer->SetUseCompression( true );
  try
    {
    writer->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    tube::ErrorMessage( "Writing " + fileName + " : Exception caught: "
      + std::string(err.GetDescription()) );
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

template< int VDimension >
int WriteOutputData( std::ofstream & fileStream,
  itk::ContinuousIndex< double, VDimension > & cIndx, double intensity,
  double ridgeness, double roundness, double curvature, double levelness )
{
  for( unsigned int i = 0; i < VDimension; ++i )
    {
    fileStream << cIndx[i] << " ";
    }
  fileStream << intensity << " ";
  fileStream << ridgeness << " ";
  fileStream << roundness << " ";
  fileStream << curvature << " ";
  fileStream << levelness << std::endl;

  return EXIT_SUCCESS;
}

typedef vnl_vector< double >            MetricVectorType;
typedef std::vector< MetricVectorType > SampleListType;

int sort_column = 0;

// comparison, not case sensitive.
bool sort_column_compare( const MetricVectorType & first,
  const MetricVectorType & second)
{
  if( first[sort_column] < second[sort_column] )
    {
    return true;
    }
  return false;
}

// Your code should be within the DoIt function...
template< class TPixel, unsigned int VDimension >
int DoIt( int argc, char * argv[] )
{
  PARSE_ARGS;

  // The timeCollector is used to perform basic profiling of the components
  //   of your algorithm.
  itk::TimeProbesCollectorBase timeCollector;

  // CLIProgressReporter is used to communicate progress with the Slicer GUI
  tube::CLIProgressReporter    progressReporter(
    "ComputeSegmentTubesParameters", CLPProcessInformation );
  progressReporter.Start();

  typedef float                                     InputPixelType;
  typedef itk::Image< InputPixelType, VDimension >  InputImageType;
  typedef itk::ImageFileReader< InputImageType >    ReaderType;

  typedef int                                       MaskPixelType;
  typedef itk::Image< MaskPixelType, VDimension >   MaskImageType;
  typedef itk::ImageFileReader< MaskImageType >     MaskReaderType;

  typedef float                                     ScalePixelType;
  typedef itk::Image< ScalePixelType, VDimension >  ScaleImageType;
  typedef itk::ImageFileReader< ScaleImageType >    ScaleReaderType;

  typedef float                                     OutputPixelType;
  typedef itk::Image< OutputPixelType, VDimension > OutputImageType;

  typedef itk::tube::RidgeExtractor< InputImageType > RidgeExtractorType;

  timeCollector.Start("Load data");

  typename ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( inputImageFileName.c_str() );
  try
    {
    reader->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    tube::ErrorMessage( "Reading volume: Exception caught: "
                        + std::string(err.GetDescription()) );
    timeCollector.Report();
    return EXIT_FAILURE;
    }
  typename InputImageType::Pointer inputImage = reader->GetOutput();

  typename MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName( maskImageFileName.c_str() );
  try
    {
    maskReader->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    tube::ErrorMessage( "Reading mask: Exception caught: "
                        + std::string(err.GetDescription()) );
    timeCollector.Report();
    return EXIT_FAILURE;
    }
  typename MaskImageType::Pointer maskImage = maskReader->GetOutput();

  typename ScaleReaderType::Pointer scaleReader = ScaleReaderType::New();
  scaleReader->SetFileName( scaleImageFileName.c_str() );
  try
    {
    scaleReader->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    tube::ErrorMessage( "Reading scale: Exception caught: "
                        + std::string(err.GetDescription()) );
    timeCollector.Report();
    return EXIT_FAILURE;
    }
  typename ScaleImageType::Pointer scaleImage = scaleReader->GetOutput();

  timeCollector.Stop("Load data");
  double progress = 0.1;
  progressReporter.Report( progress );

  timeCollector.Start("Compute ridgeness images");

  typename OutputImageType::RegionType region =
    inputImage->GetLargestPossibleRegion();

  std::string fileName = outputParametersFile + ".init.txt";
  std::ofstream outputDataStreamInit;
  outputDataStreamInit.open( fileName.c_str(), std::ios::binary |
    std::ios::out );
  outputDataStreamInit.precision( 6 );

  fileName = outputParametersFile + ".tube.txt";
  std::ofstream outputDataStreamTube;
  outputDataStreamTube.open( fileName.c_str(), std::ios::binary |
    std::ios::out );
  outputDataStreamTube.precision( 6 );

  fileName = outputParametersFile + ".bkg.txt";
  std::ofstream outputDataStreamBkg;
  outputDataStreamBkg.open( fileName.c_str(), std::ios::binary |
    std::ios::out );
  outputDataStreamBkg.precision( 6 );

  SampleListType seed;
  SampleListType tube;
  SampleListType bkg;

  itk::ImageRegionIteratorWithIndex< InputImageType > itI(
    inputImage, region );
  itk::ImageRegionIteratorWithIndex< MaskImageType > itM(
    maskImage, region );
  itk::ImageRegionIteratorWithIndex< ScaleImageType > itS(
    scaleImage, region );

  typename RidgeExtractorType::Pointer ridgeExtractor = RidgeExtractorType::New();
  ridgeExtractor->SetInputImage( inputImage );

  const double BIGD = 9999999999;

  double scale = 0;
  double scaleMin = BIGD;
  double scaleMax = 0;

  double dataMin = BIGD;
  double dataMax = 0;

  MetricVectorType instance(5, 0);
  MetricVectorType instanceMin(5, BIGD);
  MetricVectorType instanceMax(5, -BIGD);
  typename RidgeExtractorType::IndexType minIndx;
  typename RidgeExtractorType::IndexType maxIndx;
  for( unsigned int i=0; i<VDimension; ++i )
    {
    minIndx[i] = region.GetIndex()[i] + 10;
    maxIndx[i] = region.GetIndex()[i] + region.GetSize()[i] - 10;
    }

  ridgeExtractor->SetMinRidgeness( 0.6 );
  ridgeExtractor->SetMinRidgenessStart( 0.6 );
  ridgeExtractor->SetMinRoundness( -1 );
  ridgeExtractor->SetMinRoundnessStart( -1 );
  ridgeExtractor->SetMinCurvature( -1 );
  ridgeExtractor->SetMinCurvatureStart( -1 );
  ridgeExtractor->SetMinLevelness( -1 );
  ridgeExtractor->SetMinLevelnessStart( -1 );
  while( !itM.IsAtEnd() )
    {
    if( itM.Get() == maskBackgroundId
      || itM.Get() == maskTubeId )
      {
      typename RidgeExtractorType::IndexType indx;
      typename RidgeExtractorType::ContinuousIndexType cIndx;
      indx = itM.GetIndex();
      bool outOfBounds = false;
      for( unsigned int i = 0; i < VDimension; ++i )
        {
        cIndx[i] = indx[i];
        if( indx[i] < minIndx[i] || indx[i] > maxIndx[i] )
          {
          outOfBounds = true;
          }
        }

      if( !outOfBounds )
        {
        scale = itS.Get();
        ridgeExtractor->SetScale( scale );

        if( itM.Get() == maskTubeId )
          {
          double intensity = 0;
          double ridgeness = 0;
          double roundness = 0;
          double curvature = 0;
          double levelness = 0;
          ridgeExtractor->Ridgeness( cIndx, intensity, roundness, curvature,
            levelness );

          instance[0] = intensity;
          instance[1] = ridgeness;
          instance[2] = roundness;
          instance[3] = curvature;
          instance[4] = levelness;
          seed.push_back( instance );

          WriteOutputData< VDimension >( outputDataStreamInit, cIndx,
            instance[0], instance[1], instance[2], instance[3], instance[4] );

          if( ridgeExtractor->LocalRidge( cIndx ) == RidgeExtractorType::SUCCESS )
            {
            if( scale < scaleMin )
              {
              scaleMin = scale;
              }
            else if( scale > scaleMax )
              {
              scaleMax = scale;
              }

            ridgeness = ridgeExtractor->Ridgeness( cIndx, intensity, roundness,
              curvature, levelness );

            instance[0] = intensity;
            instance[1] = ridgeness;
            instance[2] = roundness;
            instance[3] = curvature;
            instance[4] = levelness;
            tube.push_back( instance );

            WriteOutputData< VDimension >( outputDataStreamTube, cIndx,
              intensity, ridgeness, roundness, curvature, levelness );
            }
          }
        else if( itM.Get() == maskBackgroundId )
          {
          for( unsigned int i = 0; i < VDimension; ++i )
            {
            cIndx[i] = indx[i];
            }
          double intensity = 0;
          double ridgeness = 0;
          double roundness = 0;
          double curvature = 0;
          double levelness = 0;
          ridgeExtractor->Ridgeness( cIndx, intensity, roundness, curvature,
            levelness );

          instance[0] = intensity;
          instance[1] = ridgeness;
          instance[2] = roundness;
          instance[3] = curvature;
          instance[4] = levelness;
          bkg.push_back( instance );

          WriteOutputData< VDimension >( outputDataStreamBkg, cIndx,
            instance[0], instance[1], instance[2], instance[3], instance[4] );
          }
        }
      }

    double intensity = itI.Get();

    if( intensity < dataMin )
      {
      dataMin = intensity;
      }
    if( intensity > dataMax )
      {
      dataMax = intensity;
      }

    ++itI;
    ++itS;
    ++itM;
    }

  outputDataStreamBkg.close();
  outputDataStreamTube.close();
  outputDataStreamInit.close();
  timeCollector.Stop("Compute ridgeness images");


  int result = EXIT_SUCCESS;

  //
  //
  itk::tube::MetaTubeExtractor params;

  // Heuristics to identify common intensity ranges
  if( dataMin > -0.5 && dataMin < 0.5 &&
    dataMax > 0.5 && dataMax <= 1.5 )
    {
    // Synthetic: 0 to 1
    dataMin = 0;
    dataMax = 1;
    }
  else if( dataMin > -10 && dataMin < 50 &&
    dataMax > 200 && dataMax <= 300 )
    {
    // MRI or synthetic: 0 to 255
    dataMin = 0;
    dataMax = 255;
    }
  else if( dataMin > -20 && dataMin < 100 &&
    dataMax > 420 && dataMax <= 600 )
    {
    // MRI or synthetic: 0 to 512
    dataMin = 0;
    dataMax = 512;
    }
  else if( dataMin > -1100 && dataMin < -900 &&
    dataMax > 900 && dataMax <= 3100 )
    { // 900 = cancellous bone
    dataMin = -1000;  // Air in HU
    dataMax = 3000;    // Dense bone in HU
    }

  itk::tube::MetaTubeExtractor::VectorType tubeColor(4, 0.0);
  tubeColor[0] = 1.0;
  tubeColor[3] = 1.0;

  params.SetGeneralProperties( dataMin, dataMax, tubeColor );

  if( scaleMax == scaleMin )
    {
    scaleMax = 10 * scaleMin;
    }
  double scaleRange = scaleMax - scaleMin;
  double ridgeScale = scaleMin + 0.25 * scaleRange;
  double ridgeScaleKernelExtent = 2.5;

  bool   ridgeDynamicScale = true;

  bool   ridgeDynamicStepSize = false;

  double ridgeStepX = 0.1;

  double ridgeMaxTangentChange = 0.75;

  double ridgeMaxXChange = 3.0;

  double portion = 1.0 / 1000.0;
  int clippedMax = (int)(tube.size() * portion);
  portion = 1.0 / 500.0;
  int clippedMaxStart = (int)(tube.size() * portion);

  sort_column = 1;
  std::sort( tube.begin(), tube.end(), sort_column_compare );
  double ridgeMinRidgeness = tube[clippedMax][1];
  double ridgeMinRidgenessStart = tube[clippedMaxStart][1];

  sort_column = 2;
  std::sort( tube.begin(), tube.end(), sort_column_compare );
  double ridgeMinRoundness = tube[clippedMax][2];
  double ridgeMinRoundnessStart = tube[clippedMaxStart][2];

  sort_column = 3;
  std::sort( tube.begin(), tube.end(), sort_column_compare );
  double ridgeMinCurvature = tube[clippedMax][3];
  double ridgeMinCurvatureStart = tube[clippedMaxStart][3];

  sort_column = 4;
  std::sort( tube.begin(), tube.end(), sort_column_compare );
  double ridgeMinLevelness = tube[clippedMax][4];
  double ridgeMinLevelnessStart = tube[clippedMaxStart][4];

  int    ridgeMaxRecoveryAttempts = 3;

  params.SetRidgeProperties( ridgeScale, ridgeScaleKernelExtent,
    ridgeDynamicScale,
    ridgeDynamicStepSize,
    ridgeStepX,
    ridgeMaxTangentChange,
    ridgeMaxXChange,
    ridgeMinRidgeness, ridgeMinRidgenessStart,
    ridgeMinRoundness, ridgeMinRoundnessStart,
    ridgeMinCurvature, ridgeMinCurvatureStart,
    ridgeMinLevelness, ridgeMinLevelnessStart,
    ridgeMaxRecoveryAttempts );

  double radiusStart = ridgeScale / inputImage->GetSpacing()[0];
  double radiusMin = ( 0.2 * scaleMin ) / inputImage->GetSpacing()[0];
  if( radiusMin < 0.3 )
    {
    radiusMin = 0.3;
    }
  double radiusMax = ( scaleMax + 0.5 * scaleRange ) /
    inputImage->GetSpacing()[0];

  // Should be a function of curvature
  double radiusMinMedialness = ridgeMinCurvature / 100;
  double radiusMinMedialnessStart = ridgeMinCurvatureStart / 100;

  params.SetRadiusProperties( radiusStart,
    radiusMin, radiusMax,
    radiusMinMedialness, radiusMinMedialnessStart );

  params.Write( outputParametersFile.c_str() );

  progress = 1.0;
  progressReporter.Report( progress );
  progressReporter.End();

  timeCollector.Report();
  return result;
}

// Main
int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  // You may need to update this line if, in the project's .xml CLI file,
  //   you change the variable name for the inputImageFileName.
  return tube::ParseArgsAndCallDoIt( inputImageFileName, argc, argv );
}
