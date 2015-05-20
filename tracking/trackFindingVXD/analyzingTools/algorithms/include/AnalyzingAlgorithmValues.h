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

    /** virtual class to calculate data. takes two TCInfos */
    virtual DataType calcData(const TCInfoType& aTC, const TCInfoType&) { return aTC.momSeed.X(); }
  };



  /** Class for storing an algorithm determining the momentum in Y */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmValuePY : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmValuePY() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(std::string("AnalyzingAlgorithmValuePY")) {}

    /** virtual class to calculate data. takes two TCInfos */
    virtual DataType calcData(const TCInfoType& aTC, const TCInfoType&) { return aTC.momSeed.Y(); }
  };



  /** Class for storing an algorithm determining the momentum in Z */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmValuePZ : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmValuePZ() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(std::string("AnalyzingAlgorithmValuePZ")) {}

    /** virtual class to calculate data. takes two TCInfos */
    virtual DataType calcData(const TCInfoType& aTC, const TCInfoType&) { return aTC.momSeed.Z(); }
  };



  /** Class for storing an algorithm determining the residual of momentum in pT */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmValuePT : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmValuePT() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(std::string("AnalyzingAlgorithmValuePT")) {}

    /** virtual class to calculate data. takes two TCInfos */
    virtual DataType calcData(const TCInfoType& aTC, const TCInfoType&) { return aTC.momSeed.Perp(); }
  };



  /** Class for storing an algorithm determining the residual of momentum in |p| */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmValueP : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmValueP() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(std::string("AnalyzingAlgorithmValueP")) {}

    /** virtual class to calculate data. takes two TCInfos */
    virtual DataType calcData(const TCInfoType& aTC, const TCInfoType&) { return aTC.momSeed.Mag(); }
  };

  // TODO
  /**
   * wie gehts weiter:
   * die ganzen values-Algorithmen einbauen (und was sonst nocht in TrackFinderVXDAnalizerModule.h angesprochen wird).
   * AnalyzingAlgorithmFactory updaten.
   *
   * TrackFinderVXDAnalizerModule:
   *  Parameter basteln, vector of vectors (for each entry : firstVector == parameter2collect) { .first = dataType, .second = algorithmType })
   *
   * in TrackFinderVXDAnalizerModule::initialize()
   * alle übergebenen Parameter, die in Root gesammelt werden, sollen eingelesen werden (factory übersetzt) und in container gesammelt (nach tcType geclustert, sowie nach typen, wo 1 oder 2 tcs übergeben werden)
   * das so bauen, dass die im event einfach übergeben werden können
   * das so bauen, dass das Root-branch füllen auch eingepackt wird
   *
   * Auswertung testen
   */

  /** Class for storing an algorithm determining the residual of momentum in theta (in degrees) */
//   template <class DataType, class TCInfoType, class VectorType>
//   class AnalyzingAlgorithmResidualTheta : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
//   public:
//  /** constructor */
//  AnalyzingAlgorithmResidualTheta() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(std::string("AnalyzingAlgorithmResidualTheta")) {}
//
//  /** virtual class to calculate data. takes two TCInfos */
//  virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC)
//  { return refTC.momSeed.Theta() * 180.*TMath::InvPi() - testTC.momSeed.Theta() * 180.*TMath::InvPi(); }
//   };
//
//
//
//   /** Class for storing an algorithm determining the residual of momentum in phi (in degrees) */
//   template <class DataType, class TCInfoType, class VectorType>
//   class AnalyzingAlgorithmResidualPhi : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
//   public:
//  /** constructor */
//  AnalyzingAlgorithmResidualPhi() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(std::string("AnalyzingAlgorithmResidualPhi")) {}
//
//  /** virtual class to calculate data. takes two TCInfos */
//  virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC)
//  { return refTC.momSeed.Phi() * 180.*TMath::InvPi() - testTC.momSeed.Phi() * 180.*TMath::InvPi(); }
//   };
//
//
//
//   /** Class for storing an algorithm determining the residual of momentum in its angle (direction residual in degrees) */
//   template <class DataType, class TCInfoType, class VectorType>
//   class AnalyzingAlgorithmResidualPAngle : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
//   public:
//  /** constructor */
//  AnalyzingAlgorithmResidualPAngle() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(std::string("AnalyzingAlgorithmResidualPAngle")) {}
//
//  /** virtual class to calculate data. takes two TCInfos */
//  virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC)
//  { return testTC.momSeed.Angle(refTC.momSeed) * 180.*TMath::InvPi(); }
//   };
//
//
//
//   /** Class for storing an algorithm determining the residual of pT in angle (transverse direction residual in degrees) */
//   template <class DataType, class TCInfoType, class VectorType>
//   class AnalyzingAlgorithmResidualPTAngle : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
//   public:
//  /** constructor */
//  AnalyzingAlgorithmResidualPTAngle() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(std::string("AnalyzingAlgorithmResidualPTAngle")) {}
//
//  /** virtual class to calculate data. takes two TCInfos */
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
//   /** Class for storing an algorithm determining the residual of the seed position in 3D */
//   template <class DataType, class TCInfoType, class VectorType>
//   class AnalyzingAlgorithmResidualPosition : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
//   public:
//  /** constructor */
//  AnalyzingAlgorithmResidualPosition() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(std::string("AnalyzingAlgorithmResidualPosition")) {}
//
//  /** virtual class to calculate data. takes two TCInfos */
//  virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC) { return refTC.posSeed.Mag() - testTC.posSeed.Mag(); }
//   };
}
