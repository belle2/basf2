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

// root:
#include <TMath.h>

namespace Belle2 {
  /** INFO
   * This file contains all the algorithms calculating residuals of something.
   * */

  /** Class for storing an algorithm determining the residual (ref-test) of momentum in X */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualPX : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualPX() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (AlgoritmType::AnalyzingAlgorithmResidualPX) {}

    /** returns the residual (ref-test) of momentum in X */
    DataType calcData(const TCInfoType& aTC) override
    {
      const auto tcs = AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectPairOfTCs(aTC);
      return tcs.refTC->momSeed.X() - tcs.testTC->momSeed.X();
    }
  };



  /** Class for storing an algorithm determining the residual (ref-test) of momentum in Y */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualPY : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualPY() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (AlgoritmType::AnalyzingAlgorithmResidualPY) {}

    /** returns the residual (ref-test) of momentum in Y */
    DataType calcData(const TCInfoType& aTC) override
    {
      const auto tcs = AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectPairOfTCs(aTC);
      return tcs.refTC->momSeed.Y() - tcs.testTC->momSeed.Y();
    }
  };



  /** Class for storing an algorithm determining the residual (ref-test) of momentum in Z */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualPZ : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualPZ() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (AlgoritmType::AnalyzingAlgorithmResidualPZ) {}

    /** returns the residual (ref-test) of momentum in Z */
    DataType calcData(const TCInfoType& aTC) override
    {
      const auto tcs = AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectPairOfTCs(aTC);
      return tcs.refTC->momSeed.Z() - tcs.testTC->momSeed.Z();
    }
  };



  /** Class for storing an algorithm determining the residual (ref-test) of momentum in pT */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualPT : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualPT() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (AlgoritmType::AnalyzingAlgorithmResidualPT) {}

    /** returns the residual (ref-test) of momentum in pT */
    DataType calcData(const TCInfoType& aTC) override
    {
      const auto tcs = AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectPairOfTCs(aTC);
      return tcs.refTC->momSeed.Perp() - tcs.testTC->momSeed.Perp();
    }
  };



  /** Class for storing an algorithm determining the residual (ref-test) of momentum in |p| */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualP : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualP() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (AlgoritmType::AnalyzingAlgorithmResidualP) {}

    /** returns the residual (ref-test) of momentum in |p| */
    DataType calcData(const TCInfoType& aTC) override
    {
      const auto tcs = AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectPairOfTCs(aTC);
      return tcs.refTC->momSeed.Mag() - tcs.testTC->momSeed.Mag();
    }
  };



  /** Class for storing an algorithm determining the residual (ref-test) of momentum in theta (in degrees) */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualPTheta : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualPTheta() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (AlgoritmType::AnalyzingAlgorithmResidualPTheta) {}

    /** returns the residual (ref-test) of momentum in theta (in degrees) */
    DataType calcData(const TCInfoType& aTC) override
    {
      const auto tcs = AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectPairOfTCs(aTC);
      return tcs.refTC->momSeed.Theta() * 180.*TMath::InvPi() - tcs.testTC->momSeed.Theta() * 180.*TMath::InvPi();
    }
  };



  /** Class for storing an algorithm determining the residual (ref-test) of momentum in phi (in degrees) */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualPPhi : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualPPhi() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (AlgoritmType::AnalyzingAlgorithmResidualPPhi) {}

    /** returns the residual (ref-test) of momentum in phi (in degrees) */
    DataType calcData(const TCInfoType& aTC) override
    {
      const auto tcs = AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectPairOfTCs(aTC);
      return tcs.refTC->momSeed.Phi() * 180.*TMath::InvPi() - tcs.testTC->momSeed.Phi() * 180.*TMath::InvPi();
    }
  };



  /** Class for storing an algorithm determining the residual (ref-test) of momentum in its angle (direction residual in degrees) */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualPAngle : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualPAngle() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (AlgoritmType::AnalyzingAlgorithmResidualPAngle) {}

    /** returns the residual (ref-test) of momentum in its angle (direction residual in degrees) */
    DataType calcData(const TCInfoType& aTC) override
    {
      const auto tcs = AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectPairOfTCs(aTC);
      return tcs.testTC->momSeed.Angle(tcs.refTC->momSeed) * 180.*TMath::InvPi();
    }
  };



  /** Class for storing an algorithm determining the residual (ref-test) of pT in angle (transverse direction residual in degrees) */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualPTAngle : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualPTAngle() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (AlgoritmType::AnalyzingAlgorithmResidualPTAngle) {}

    /** returns the residual (ref-test) of pT in angle (transverse direction residual in degrees) */
    DataType calcData(const TCInfoType& aTC) override
    {
      const auto tcs = AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectPairOfTCs(aTC);
      VectorType refMom = tcs.refTC->momSeed;
      refMom.SetZ(0.);
      VectorType testMom = tcs.testTC->momSeed;
      testMom.SetZ(0.);
      return testMom.Angle(refMom) * 180.*TMath::InvPi();
    }
  };



  /** Class for storing an algorithm determining the residual (ref-test) of the seed position in 3D */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualPosition : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualPosition() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (AlgoritmType::AnalyzingAlgorithmResidualPosition) {}

    /** returns the residual (ref-test) of the seed position in 3D */
    DataType calcData(const TCInfoType& aTC) override
    {
      const auto tcs = AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectPairOfTCs(aTC);
      return (tcs.refTC->posSeed - tcs.testTC->posSeed).Mag();
    }
  };



  /** Class for storing an algorithm determining the residual (ref-test) of the seed position in XY (=r) */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmResidualPositionXY : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmResidualPositionXY() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (AlgoritmType::AnalyzingAlgorithmResidualPositionXY) {}

    /** returns the residual (ref-test) of the seed position in XY (=r) */
    DataType calcData(const TCInfoType& aTC) override
    {
      const auto tcs = AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectPairOfTCs(aTC);
      return (tcs.refTC->posSeed - tcs.testTC->posSeed).Perp();
    }
  };

}
