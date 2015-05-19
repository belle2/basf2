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
   * This file contains all the algorithms calculating residuals of something.
   * */

  /** Class for storing an algorithm determining the residual of momentum in X */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualPX : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualPX() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (std::string("AnalyzingAlgorithmResidualPX")) {}

    /** virtual class to calculate data. takes two TCInfos */
    virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC) { return refTC.momSeed.X() - testTC.momSeed.X(); }
  };



  /** Class for storing an algorithm determining the residual of momentum in Y */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualPY : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualPY() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (std::string("AnalyzingAlgorithmResidualPY")) {}

    /** virtual class to calculate data. takes two TCInfos */
    virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC) { return refTC.momSeed.Y() - testTC.momSeed.Y(); }
  };



  /** Class for storing an algorithm determining the residual of momentum in Z */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualPZ : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualPZ() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (std::string("AnalyzingAlgorithmResidualPZ")) {}

    /** virtual class to calculate data. takes two TCInfos */
    virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC) { return refTC.momSeed.Z() - testTC.momSeed.Z(); }
  };



  /** Class for storing an algorithm determining the residual of momentum in pT */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualPT : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualPT() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (std::string("AnalyzingAlgorithmResidualPT")) {}

    /** virtual class to calculate data. takes two TCInfos */
    virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC) { return refTC.momSeed.Perp() - testTC.momSeed.Perp(); }
  };



  /** Class for storing an algorithm determining the residual of momentum in |p| */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualP : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualP() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (std::string("AnalyzingAlgorithmResidualP")) {}

    /** virtual class to calculate data. takes two TCInfos */
    virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC) { return refTC.momSeed.Mag() - testTC.momSeed.Mag(); }
  };



  /** Class for storing an algorithm determining the residual of momentum in theta (in degrees) */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualTheta : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualTheta() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (std::string("AnalyzingAlgorithmResidualTheta")) {}

    /** virtual class to calculate data. takes two TCInfos */
    virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC)
    { return refTC.momSeed.Theta() * 180.*TMath::InvPi() - testTC.momSeed.Theta() * 180.*TMath::InvPi(); }
  };



  /** Class for storing an algorithm determining the residual of momentum in phi (in degrees) */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualPhi : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualPhi() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (std::string("AnalyzingAlgorithmResidualPhi")) {}

    /** virtual class to calculate data. takes two TCInfos */
    virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC)
    { return refTC.momSeed.Phi() * 180.*TMath::InvPi() - testTC.momSeed.Phi() * 180.*TMath::InvPi(); }
  };



  /** Class for storing an algorithm determining the residual of momentum in its angle (direction residual in degrees) */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualPAngle : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualPAngle() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (std::string("AnalyzingAlgorithmResidualPAngle")) {}

    /** virtual class to calculate data. takes two TCInfos */
    virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC)
    { return testTC.momSeed.Angle(refTC.momSeed) * 180.*TMath::InvPi(); }
  };



  /** Class for storing an algorithm determining the residual of pT in angle (transverse direction residual in degrees) */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualPTAngle : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualPTAngle() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (std::string("AnalyzingAlgorithmResidualPTAngle")) {}

    /** virtual class to calculate data. takes two TCInfos */
    virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC)
    {
      VectorType refMom = refTC.momSeed;
      refMom.SetZ(0.);
      VectorType testMom = testTC.momSeed;
      testMom.SetZ(0.);
      return testMom.Angle(refMom) * 180.*TMath::InvPi();
    }
  };



  /** Class for storing an algorithm determining the residual of the seed position in 3D */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualPosition : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualPosition() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (std::string("AnalyzingAlgorithmResidualPosition")) {}

    /** virtual class to calculate data. takes two TCInfos */
    virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC) { return refTC.posSeed.Mag() - testTC.posSeed.Mag(); }
  };
}
