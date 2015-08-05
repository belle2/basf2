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
    static const VXDMomentumEstimation& getInstance()
    {
      static VXDMomentumEstimation instance;
      return instance;
    }

    double estimateQOverP(const ClusterType& cluster, const TVector3& momentum, const TVector3& position, short charge) const
    {
      double momentumEstimation = estimateMomentum(cluster, momentum, position, charge);
      double QOverP = charge / momentumEstimation;

      return QOverP;
    }

  private:
    double estimateMomentum(const ClusterType& cluster, const TVector3& momentum, const TVector3& position, short charge) const
    {
      const VXDMomentumEstimationTools<ClusterType>& tools = VXDMomentumEstimationTools<ClusterType>::getInstance();

      double dEdX = tools.getDEDX(cluster, momentum, position, charge);

      // These numbers are fit results from the Analyse_Hitwise.ipynb ipython notebook and can not be calculated.
      double aE = 1.049925575e+11;
      double bE = -738645.4502;
      double cE = 0.03691275027;
      double dE = -3.155086281e-09;
      double estimation = aE / (dEdX - bE) / (dEdX - bE) + cE + dE * dEdX;


      double aM = 103.3845985;
      double bM = -4.812561153;
      double cM = 0.08154437323;
      double mediumCorrection = aM * estimation * estimation + bM * estimation + cM;
      double estimationWithMediumCalibration = estimation - mediumCorrection;


      return estimation;
    }

  };
}
