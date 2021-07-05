/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFitting/measurementCreator/measurements/PlanarVXDMomentumMeasurement.h>


using namespace Belle2;

template<>
int PlanarVXDMomentumMeasurement<PXDCluster>::getDetId() const
{
  return Belle2::Const::PXD;
}

template<>
int PlanarVXDMomentumMeasurement<SVDCluster>::getDetId() const
{
  return Belle2::Const::SVD;
}
