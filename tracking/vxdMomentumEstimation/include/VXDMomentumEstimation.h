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
      double a = 4.70402242E11;
      double b = -1.87831022E06;
      double c = 1.39962192E-02;
      double momentumEstimation = a / (dEdX - b) / (dEdX - b) + c;

      return momentumEstimation;
    }

  };
}
