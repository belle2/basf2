/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#ifndef VTX_DATAOBJECTS_VTXTRUEHIT_H
#define VTX_DATAOBJECTS_VTXTRUEHIT_H

#include <vxd/dataobjects/VXDTrueHit.h>

namespace Belle2 {
  /**
    * Class VTXTrueHit - Records of tracks that either enter or leave
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
  class VTXTrueHit : public VXDTrueHit {
  public:
    /** Default constructor for ROOT IO */
    VTXTrueHit(): VXDTrueHit() {}

    /** Constructor
     * @param sensorID SensorID of the sensor
     * @param posEntry position of the start of the track in local coordinates
     * @param posMidPoint position of the mid point of the track in local coordinates
     * @param posExit position of the end of the track in local coordinates
     * @param momEntry momentum of the start of the track in local coordinates
     * @param momMidPoint momentum of the mid point of the track in local coordinates
     * @param momExit momentum of the end of the track in local coordinates
     * @param momentum momentum of the particle in local coordinates
     * @param energyDep energy deposited by the track inside the sensor
     * @param globalTime timestamp of the hit
     */
    VTXTrueHit(VxdID sensorID, float* posEntry, float* posMidPoint, float* posExit,
               float* momEntry, float* momMidPoint, float* momExit, float energyDep, float globalTime):
      VXDTrueHit(sensorID, posEntry, posMidPoint, posExit,
                 momEntry, momMidPoint, momExit, energyDep, globalTime)
    {}

    ClassDef(VTXTrueHit, 6)
  };

} // end namespace Belle2

#endif
