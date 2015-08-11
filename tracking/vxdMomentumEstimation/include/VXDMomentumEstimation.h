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
  /** Class doing the momentum estimation from dEdX for SVDClusters and PXDClusters */
  template <class ClusterType>
  class VXDMomentumEstimation {

  private:
    /** Do not copy or create */
    VXDMomentumEstimation() { }
    /** Do not copy or create */
    VXDMomentumEstimation(const VXDMomentumEstimation&);
    /** Do not copy or create */
    VXDMomentumEstimation& operator = (const VXDMomentumEstimation&);

  public:
    /** Struct holding the parameters of the estimation function which maps dEdX to p */
    struct FitParameters {
      double aE = 1.080493149e+11;
      double bE = -752991.2151;
      double cE = 0.03636985211;
      double dE = -3.020359066e-09;
    };

    /** Struct holding the parameters of the correction function to map them median of the estimation
     * function to 0.  */
    struct CorrectionFitParameters {
      double aM = 41.21477882;
      double bM = -2.650950176;
      double cM = 0.05576940304;
      double dM = -206.4416227;
    };

    /** Use this class as a singleton */
    static const VXDMomentumEstimation& getInstance()
    {
      static VXDMomentumEstimation instance;
      return instance;
    }

    /** Main function: Estimate p over q for the given cluster and the path length calculated using the given tracking seeds
     * with the fit parameters. */
    double estimateQOverP(const ClusterType& cluster, const TVector3& momentum, const TVector3& position, short charge,
                          const FitParameters& fitParameters, const CorrectionFitParameters& correctionFitParameters) const
    {
      const VXDMomentumEstimationTools<ClusterType>& tools = VXDMomentumEstimationTools<ClusterType>::getInstance();

      const double dEdX = tools.getDEDX(cluster, momentum, position, charge);
      const double momentumEstimation = convertDEDXToMomentum(dEdX, fitParameters, correctionFitParameters);
      const double QOverP = charge / momentumEstimation;

      return QOverP;
    }

    /** Main function: Estimate p over q for the given cluster and the thickness of the cluster
     * with the fit parameters. */
    double estimateQOverPWithThickness(const ClusterType& cluster, short charge, const FitParameters& fitParameters,
                                       const CorrectionFitParameters& correctionFitParameters) const
    {
      const VXDMomentumEstimationTools<ClusterType>& tools = VXDMomentumEstimationTools<ClusterType>::getInstance();

      const double dEdX = tools.getDEDXWithThickness(cluster);
      const double momentumEstimation = convertDEDXToMomentum(dEdX, fitParameters, correctionFitParameters);
      const double QOverP = charge / momentumEstimation;

      return QOverP;
    }

  private:
    /** After calculating dEdX we need to map this to p using a predefined function and parameters that
     * were extracted with a fit to mc data. */
    double convertDEDXToMomentum(double dEdX, const FitParameters& fitParameters,
                                 const CorrectionFitParameters& correctionFitParameters) const
    {
      const double firstPart = fitParameters.aE / (dEdX - fitParameters.bE) / (dEdX - fitParameters.bE);
      const double lastPart = fitParameters.cE + fitParameters.dE * dEdX;
      const double estimation = firstPart + lastPart;

      const double quadPart = correctionFitParameters.aM * estimation * estimation;
      const double linearPart = correctionFitParameters.bM * estimation;
      const double constantPart = correctionFitParameters.cM;
      const double cubicPart = correctionFitParameters.dM * estimation * estimation * estimation;
      const double mediumCorrection = cubicPart + quadPart + linearPart + constantPart;

      const double estimationWithMediumCalibration = estimation - mediumCorrection;
      return estimationWithMediumCalibration;
    }

  };
}
