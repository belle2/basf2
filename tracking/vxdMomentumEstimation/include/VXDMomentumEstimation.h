/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <tracking/vxdMomentumEstimation/VXDMomentumEstimationTools.h>


namespace Belle2 {
  template <class ClusterType>
  class VXDMomentumEstimation {

  private:
    VXDMomentumEstimation() { }
    VXDMomentumEstimation(const VXDMomentumEstimation&);
    VXDMomentumEstimation& operator = (const VXDMomentumEstimation&);

  public:
    struct FitParameters {
      double aE = 1.080493149e+11;
      double bE = -752991.2151;
      double cE = 0.03636985211;
      double dE = -3.020359066e-09;
    };

    struct CorrectionFitParameters {
      double aM = 41.21477882;
      double bM = -2.650950176;
      double cM = 0.05576940304;
      double dM = -206.4416227;
    };

    static const VXDMomentumEstimation& getInstance()
    {
      static VXDMomentumEstimation instance;
      return instance;
    }

    double estimateQOverP(const ClusterType& cluster, const TVector3& momentum, const TVector3& position, short charge,
                          const FitParameters& fitParameters, const CorrectionFitParameters& correctionFitParameters) const
    {
      double momentumEstimation = estimateMomentum(cluster, momentum, position, charge, fitParameters, correctionFitParameters);
      double QOverP = charge / momentumEstimation;

      return QOverP;
    }

  private:
    double estimateMomentum(const ClusterType& cluster, const TVector3& momentum, const TVector3& position, short charge,
                            const FitParameters& fitParameters, const CorrectionFitParameters& correctionFitParameters) const
    {
      const VXDMomentumEstimationTools<ClusterType>& tools = VXDMomentumEstimationTools<ClusterType>::getInstance();

      double dEdX = tools.getDEDX(cluster, momentum, position, charge);

      // These numbers are fit results from the Analyse_Hitwise.ipynb ipython notebook and can not be calculated from scratch.
      double estimation = fitParameters.aE / (dEdX - fitParameters.bE) / (dEdX - fitParameters.bE) + fitParameters.cE + fitParameters.dE *
                          dEdX;
      double mediumCorrection = correctionFitParameters.aM * estimation * estimation + correctionFitParameters.bM * estimation +
                                correctionFitParameters.cM + correctionFitParameters.dM * estimation * estimation * estimation;
      double estimationWithMediumCalibration = estimation - mediumCorrection;
      return estimationWithMediumCalibration;
    }

  };
}
