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
    AnalyzingAlgorithmValuePX() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(std::string("AnalyzingAlgorithmValuePX")) {}

    /** returns the momentum in X */
    virtual DataType calcData(const TCInfoType& aTC)
    { return AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectTC(aTC).momSeed.X(); }
  };



  /** Class for storing an algorithm determining the momentum in Y */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmValuePY : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmValuePY() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(std::string("AnalyzingAlgorithmValuePY")) {}

    /** returns the momentum in Y */
    virtual DataType calcData(const TCInfoType& aTC)
    { return AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectTC(aTC).momSeed.Y(); }
  };



  /** Class for storing an algorithm determining the momentum in Z */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmValuePZ : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmValuePZ() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(std::string("AnalyzingAlgorithmValuePZ")) {}

    /** returns the momentum in Z */
    virtual DataType calcData(const TCInfoType& aTC)
    { return AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectTC(aTC).momSeed.Z(); }
  };



  /** Class for storing an algorithm determining the momentum in pT */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmValuePT : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmValuePT() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(std::string("AnalyzingAlgorithmValuePT")) {}

    /** returns the momentum in pT */
    virtual DataType calcData(const TCInfoType& aTC)
    { return AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectTC(aTC).momSeed.Perp(); }
  };



  /** Class for storing an algorithm determining the momentum in |p| */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmValueP : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmValueP() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(std::string("AnalyzingAlgorithmValueP")) {}

    /** returns the momentum in |p| */
    virtual DataType calcData(const TCInfoType& aTC)
    { return AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectTC(aTC).momSeed.Mag(); }
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

  /** Class for storing an algorithm determining the momentum in theta (in degrees) */
//   template <class DataType, class TCInfoType, class VectorType>
//   class AnalyzingAlgorithmResidualTheta : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
//   public:
//  /** constructor */
//  AnalyzingAlgorithmResidualTheta() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(std::string("AnalyzingAlgorithmResidualTheta")) {}
//
  /** returns the momentum in theta (in degrees) */
//  virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC)
//  { return refTC.momSeed.Theta() * 180.*TMath::InvPi() - testTC.momSeed.Theta() * 180.*TMath::InvPi(); }
//   };
//
//
//
  /** Class for storing an algorithm determining the momentum in phi (in degrees) */
//   template <class DataType, class TCInfoType, class VectorType>
//   class AnalyzingAlgorithmResidualPhi : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
//   public:
//  /** constructor */
//  AnalyzingAlgorithmResidualPhi() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(std::string("AnalyzingAlgorithmResidualPhi")) {}
//
  /** returns the momentum in phi (in degrees) */
//  virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC)
//  { return refTC.momSeed.Phi() * 180.*TMath::InvPi() - testTC.momSeed.Phi() * 180.*TMath::InvPi(); }
//   };
//
//
//
  /** Class for storing an algorithm determining the momentum in its angle (direction residual in degrees) */
//   template <class DataType, class TCInfoType, class VectorType>
//   class AnalyzingAlgorithmResidualPAngle : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
//   public:
//  /** constructor */
//  AnalyzingAlgorithmResidualPAngle() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(std::string("AnalyzingAlgorithmResidualPAngle")) {}
//
  /** returns the momentum in its angle (direction residual in degrees) */
//  virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC)
//  { return testTC.momSeed.Angle(refTC.momSeed) * 180.*TMath::InvPi(); }
//   };
//
//
//
  /** Class for storing an algorithm determining the pT in angle (transverse direction residual in degrees) */
//   template <class DataType, class TCInfoType, class VectorType>
//   class AnalyzingAlgorithmResidualPTAngle : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
//   public:
//  /** constructor */
//  AnalyzingAlgorithmResidualPTAngle() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(std::string("AnalyzingAlgorithmResidualPTAngle")) {}
//
  /** returns the pT in angle (transverse direction residual in degrees) */
//  virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC) {
//    VectorType refMom = refTC.momSeed;
//    refMom.SetZ(0.);
//    VectorType testMom = testTC.momSeed;
//    testMom.SetZ(0.);
//    return testMom.Angle(refMom) * 180.*TMath::InvPi();
//  }
//   };
//
//
//
  /** Class for storing an algorithm determining the the seed position in 3D */
//   template <class DataType, class TCInfoType, class VectorType>
//   class AnalyzingAlgorithmResidualPosition : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
//   public:
//  /** constructor */
//  AnalyzingAlgorithmResidualPosition() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(std::string("AnalyzingAlgorithmResidualPosition")) {}
//
  /** returns the the seed position in 3D */
//  virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC) { return refTC.posSeed.Mag() - testTC.posSeed.Mag(); }
//   };

  /** Class for storing an algorithm determining the seed position in XY (=r) */
//   template <class DataType, class TCInfoType, class VectorType>
//   class AnalyzingAlgorithmResidualPosition : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
//   public:
//  /** constructor */
//  AnalyzingAlgorithmResidualPositionXY() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
//  (std::string("AnalyzingAlgorithmResidualPosition")) {}
//
  /** returns the seed position in XY (=r) */
//  virtual DataType calcData(const TCInfoType& aTC)
//  {
//    TcPair tcs = chooseCorrectPairOfTCs(aTC);
//    return tcs.refTC.posSeed.Perp() - tcs.testTC.posSeed.Perp();
//  }
//   };
}
