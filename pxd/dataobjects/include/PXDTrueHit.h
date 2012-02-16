/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Peter Kvasnicka, Martin Ritter             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDTRUEHIT_H
#define PXDTRUEHIT_H

#include <vxd/VxdID.h>
#include <vxd/dataobjects/VXDTrueHit.h>

namespace Belle2 {

  /**
    * Class PXDTrueHit - Position where are particle traversed the detector plane.
    *
    * This class is meant as helper for tracking optimization. It stores
    * information about particle traversal in condensed form: The local
    * coordinates where the particle traversed the detector plane as well as
    * the momenta when the particle entered the silicon, traversed the detector
    * plane and exited the silicon.
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response, they contain _local_
    * information.
    */
  class PXDTrueHit : public VXDTrueHit {
  public:
    /** Default constructor for ROOT IO */
    PXDTrueHit(): VXDTrueHit() {}

    /** Constructor
     * @param sensorID SensorID of the Sensor
     * @param u u coordinate of the hit in local coordinates
     * @param v v coordinate of the hit in local coordinates
     * @param momentum momentum of the particle in local coordinates
     * @param globalTime timestamp of the hit
     */
    PXDTrueHit(VxdID sensorID, float u, float v, float energyDep, float globalTime,
               const TVector3& momentum, const TVector3& entryMomentum, const TVector3& exitMomentum):
      VXDTrueHit(sensorID, u, v, energyDep, globalTime, momentum, entryMomentum, exitMomentum)
    {}
    /** Constructor
     * @param sensorID SensorID of the Sensor
     * @param u u coordinate of the hit in local coordinates
     * @param v v coordinate of the hit in local coordinates
     * @param entryU u coordinate of the hit in local coordinates when entering silicon
     * @param entryV v coordinate of the hit in local coordinates when entering silicon
     * @param exitU u coordinate of the hit in local coordinates when exiting silicon
     * @param exitV v coordinate of the hit in local coordinates when exiting silicon
     * @param momentum momentum of the particle in local coordinates
     * @param entryMomentum momentum of the particle in local coordinates when entering silicon
     * @param exitMomentum momentum of the particle in local coordinates when exiting silicon
     * @param globalTime timestamp of the hit
     */
    PXDTrueHit(
      VxdID sensorID, float u, float v, float entryU, float entryV, float exitU, float exitV, float energyDep, float globalTime,
      const TVector3& momentum, const TVector3& entryMomentum, const TVector3& exitMomentum):
      VXDTrueHit(sensorID, u, v, entryU, entryV, exitU, exitV, energyDep, globalTime, momentum, entryMomentum, exitMomentum)
    {}
    ClassDef(PXDTrueHit, 3)
  };

} // end namespace Belle2

#endif
