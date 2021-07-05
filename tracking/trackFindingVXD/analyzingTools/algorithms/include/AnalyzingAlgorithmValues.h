/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

// tracking:
#include <tracking/trackFindingVXD/analyzingTools/algorithms/AnalyzingAlgorithmBase.h>

// stl:
#include <string>


namespace Belle2 {
  /** INFO
   * This file contains all the algorithms calculating a certain value of something.
   * */

  /** Class for storing an algorithm determining the momentum in X */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmValuePX : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmValuePX() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(AlgoritmType::AnalyzingAlgorithmValuePX) {}

    /** returns the momentum in X */
    DataType calcData(const TCInfoType& aTC) override
    { return AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectTC(aTC).momSeed.X(); }
  };



  /** Class for storing an algorithm determining the momentum in Y */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmValuePY : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmValuePY() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(AlgoritmType::AnalyzingAlgorithmValuePY) {}

    /** returns the momentum in Y */
    DataType calcData(const TCInfoType& aTC) override
    { return AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectTC(aTC).momSeed.Y(); }
  };



  /** Class for storing an algorithm determining the momentum in Z */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmValuePZ : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmValuePZ() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(AlgoritmType::AnalyzingAlgorithmValuePZ) {}

    /** returns the momentum in Z */
    DataType calcData(const TCInfoType& aTC) override
    { return AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectTC(aTC).momSeed.Z(); }
  };



  /** Class for storing an algorithm determining the momentum in pT */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmValuePT : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmValuePT() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(AlgoritmType::AnalyzingAlgorithmValuePT) {}

    /** returns the momentum in pT */
    DataType calcData(const TCInfoType& aTC) override
    { return AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectTC(aTC).momSeed.Perp(); }
  };



  /** Class for storing an algorithm determining the momentum in |p| */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmValueP : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmValueP() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(AlgoritmType::AnalyzingAlgorithmValueP) {}

    /** returns the momentum in |p| */
    DataType calcData(const TCInfoType& aTC) override
    { return AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectTC(aTC).momSeed.Mag(); }
  };



  /** Class for storing an algorithm determining the momentum in theta (in degrees) */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmValuePTheta : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmValuePTheta() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (AlgoritmType::AnalyzingAlgorithmValuePTheta) {}

    /** returns the momentum in theta (in degrees) */
    DataType calcData(const TCInfoType& aTC) override
    { return AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectTC(aTC).momSeed.Theta() * 180.*TMath::InvPi(); }
  };



  /** Class for storing an algorithm determining the momentum in phi (in degrees) */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmValuePPhi : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmValuePPhi() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (AlgoritmType::AnalyzingAlgorithmValuePPhi) {}

    /** returns the momentum in phi (in degrees) */
    DataType calcData(const TCInfoType& aTC) override
    { return AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectTC(aTC).momSeed.Phi() * 180.*TMath::InvPi(); }
  };




  /** Class for storing an algorithm determining the the distance seedHit to IP in 3D */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmValueDistSeed2IP : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmValueDistSeed2IP() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (AlgoritmType::AnalyzingAlgorithmValueDistSeed2IP) {}

    /** returns the the seed position in 3D */
    DataType calcData(const TCInfoType& aTC) override
    { return (AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectTC(aTC).posSeed - AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::s_origin).Mag(); }
  };



  /** Class for storing an algorithm determining the distance seedHit to IP in XY (=r) */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmValueDistSeed2IPXY : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmValueDistSeed2IPXY() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (AlgoritmType::AnalyzingAlgorithmValueDistSeed2IPXY) {}

    /** returns the seed position in XY (=r) */
    DataType calcData(const TCInfoType& aTC) override
    { return (AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectTC(aTC).posSeed - AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::s_origin).Perp(); }
  };


  /** Class for storing an algorithm determining the distance seedHit to IP in XY (=r) */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmValueDistSeed2IPZ : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmValueDistSeed2IPZ() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (AlgoritmType::AnalyzingAlgorithmValueDistSeed2IPZ) {}

    /** returns the seed position in XY (=r) */
    DataType calcData(const TCInfoType& aTC) override
    { return (AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectTC(aTC).posSeed.Z() - AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::s_origin.Z()); }
  };


  /** Class for storing an algorithm determining the quality indicator of the TC */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmValueQI : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmValueQI() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (AlgoritmType::AnalyzingAlgorithmValueQI) {}

    /** returns the the quality indicator of the TC) */
    DataType calcData(const TCInfoType& aTC) override
    { return (aTC.tC->getQualityIndicator()); }
  };

  // TODO
  /**
   * next steps:
   * implement all values-algorithms here (simply everything which can be found in TrackFinderVXDAnalizerModule.h.
   * update AnalyzingAlgorithmFactory
   *
   * TrackFinderVXDAnalizerModule:
   *  implement parameter, vector of vectors (for each entry : firstVector == parameter2collect) { .first = dataType, .second = algorithmType })
   *
   * in TrackFinderVXDAnalizerModule::initialize()
   * all parameters passed shall be streamed into RootParameterTracker, which takes care of root-storing.
   *  is clustered via tcType and types where 1 or 2 tcs are passed.
   *
   * implement it in a way that is easy to use within events
   * implement the feature for the root-Tracker to take care of branches too.
   *
   */

}
