/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackFitting/measurementCreator/measurements/PlanarVXDMomentumMeasurement.h>


using namespace Belle2;

template<>
int PlanarVXDMomentumMeasurement<PXDCluster>::getDetId() const
{
  return Belle2::Const::PXD;
}

template<>
int PlanarVXDMomentumMeasurement<VTXCluster>::getDetId() const
{
  return Belle2::Const::VTX;
}

template<>
int PlanarVXDMomentumMeasurement<SVDCluster>::getDetId() const
{
  return Belle2::Const::SVD;
}
