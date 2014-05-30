/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2014  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Peter Kvasnicka, Martin Ritter             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef SVD_DATAOBJECTS_SVDSIMHIT_H
#define SVD_DATAOBJECTS_SVDSIMHIT_H

#include <vxd/dataobjects/VXDSimHit.h>

namespace Belle2 {
  /** Class SVDSimHit - Geant4 simulated hit for the SVD.
   *
   * This class holds particle hit data from geant4 simulation. As the
   * simulated hit classes are used to generate detector response, they contain
   * _local_ information.
   */
  class SVDSimHit : public VXDSimHit {
  public:
    /** default constructor for ROOT */
    SVDSimHit(): VXDSimHit() {}

    /** Standard constructor
     * @param sensorID ID of the sensor
     * @param pdg PDG Code of the particle producing the Hit
     * @param globalTime Time of electron deposition
     * @param posIn Start point of electron deposition in local coordinates
     * @param posOut End point of electron deposition in local coordinates
     */
    SVDSimHit(VxdID sensorID, int pdg, float globalTime, const float* posIn,
              const float* posOut): VXDSimHit(sensorID, pdg, globalTime, posIn, posOut)
    {}

  private:
    ClassDef(SVDSimHit, 3)
  };
} // end namespace Belle2

#endif
