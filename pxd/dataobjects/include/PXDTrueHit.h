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

#include <vxd/dataobjects/VxdID.h>
#include <vxd/dataobjects/VXDTrueHit.h>

namespace Belle2 {
  /** \addtogroup dataobjects
    * @{
    */

  /**
    * Class PXDTrueHit - Records of tracks that either enter or leave
    * the sensitive volume.
    *
    * This class is meant as helper for tracking optimization. It stores
    * information about particle tracks in a sensor in condensed form: The local
    * coordinates of the start point, midpoint and endpoint of the track,
    * as well as track momenta at these points.
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
     * @param u u coordinate of the hit
     * @param v v coordinate of the hit
     * @param momentum momentum of the particle in local coordinates
     * @param globalTime timestamp of the hit
     */
    PXDTrueHit(VxdID sensorID, float u, float v, float energyDep, float globalTime,
               const TVector3& momentum, const TVector3& entryMomentum, const TVector3& exitMomentum):
      VXDTrueHit(sensorID, u, v, energyDep, globalTime, momentum, entryMomentum, exitMomentum)
    {}
    /** (Full) constructor
     * @param sensorID SensorID of the sensor
     * @param u u coordinate of the hit
     * @param v v coordinate of the hit
     * @param w w coordinate of the hit
     * @param entryU u coordinate of the track start point
     * @param entryV v coordinate of the track start point
     * @param entryW w coordinate of the track start point
     * @param exitU u coordinate of the track endpoint
     * @param exitV v coordinate of the track endpoint
     * @param exitW w coordinate of the track endpoint
     * @param momentum momentum of the particle in local coordinates
     * @param entryMomentum momentum at track start point
     * @param exitMomentum momentum at track endpoint
     * @param globalTime timestamp of the hit
     */
    PXDTrueHit(
      VxdID sensorID, float u, float v, float w, float entryU, float entryV, float entryW,
      float exitU, float exitV, float exitW, float energyDep, float globalTime,
      const TVector3& momentum, const TVector3& entryMomentum, const TVector3& exitMomentum):
      VXDTrueHit(sensorID, u, v, w, entryU, entryV, entryW, exitU, exitV, exitW,
                 energyDep, globalTime, momentum, entryMomentum, exitMomentum)
    {}
    ClassDef(PXDTrueHit, 4)
  };

  /** @}*/

} // end namespace Belle2

#endif
