/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VXDSimHit.h>

namespace Belle2 {
  /** Class PXDSimHit - Geant4 simulated hit for the PXD.
   *
   * This class holds particle hit data from geant4 simulation. As the
   * simulated hit classes are used to generate detector response, they contain
   * _local_ information.
   */
  class PXDSimHit : public VXDSimHit {
  public:
    /** default constructor for ROOT */
    PXDSimHit(): VXDSimHit() {}

    /** Standard constructor
     * @param sensorID ID of the sensor
     * @param pdg PDG Code of the particle producing the Hit
     * @param globalTime Time of electron deposition
     * @param posIn Start point of electron deposition in local coordinates
     * @param posOut End point of electron deposition in local coordinates
     */
    PXDSimHit(VxdID sensorID, int pdg, float globalTime, const float* posIn,
              const float* posOut): VXDSimHit(sensorID, pdg, globalTime, posIn, posOut)
    {}

  private:
    ClassDef(PXDSimHit, 3)
  };
} // end namespace Belle2
