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
  /**
   * Class doing the momentum estimation from dEdX for SVDClusters and PXDClusters.
   * It uses a precompiled estimator function (given by the parameters FitParameters and CorrectionFitParameters
   * and calculates the momentum estimation based on dEdX that get compiled from the ADC count and the path length of each given hit.
   * For calculating the path length, some helix parameter must be used, than can for example come from the track fit.
   *
   * Template class for SVD or PXD clusters.
   *  */
  template <class ClusterType>
  class VXDMomentumEstimation {

  private:
    /** Do not copy or create. */
    VXDMomentumEstimation() { }
    /** Do not copy or create. */
    VXDMomentumEstimation(const VXDMomentumEstimation&);
    /** Do not copy or create. */
    VXDMomentumEstimation& operator = (const VXDMomentumEstimation&);

  public:
    /** Struct holding the parameters of the estimation function which maps dEdX to p. */
    struct FitParameters {
      /// a Parameter
      double aE = 1.56173140e+07;
      /// b Parameter
      double bE = -9.89192780e+03;
      /// c Parameter
      double cE = 2.42177970e-02;
      /// d Parameter
      double dE = 2.65702553e-08;
    };

    /** Struct holding the parameters of the correction function to map them median of the estimation
     * function to 0.  */
    struct CorrectionFitParameters {
      /// a Parameter
      double aM = 0;
      /// b Parameter
      double bM = 0;
      /// c Parameter
      double cM = 0;
      /// d Parameter
      double dM = 0;
    };

    /** Use this class as a singleton. */
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
