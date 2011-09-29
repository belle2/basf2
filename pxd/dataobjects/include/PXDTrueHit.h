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

#include <pxd/vxd/VxdID.h>

// ROOT
#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  /**
    * ClassPXDTrueHit - Geant4 simulated hit for the PXD.
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response, they contain _local_
    * information.
    */
  class PXDTrueHit : public TObject {

  public:
    /** Constructor
     * @param sensorID SensorID of the Sensor
     * @param u u coordinate of the hit in local coordinates
     * @param v v coordinate of the hit in local coordinates
     * @param momentum momentum of the particle in local coordinates
     * @param globalTime timestamp of the hit
     */
    PXDTrueHit(
      VxdID sensorID = 0,
      float u = 0, float v = 0, const TVector3 momentum = TVector3(0, 0, 0), float globalTime = 0):
        m_sensorID(sensorID),
        m_u(u), m_v(v), m_momentum(momentum), m_globalTime(globalTime) {}

    /** Return the Sensor ID */
    VxdID getSensorID() const { return m_sensorID; }
    /** Retun local u coordinate of hit */
    float getU() const { return m_u; }
    /** Retun local v coordinate of hit */
    float getV() const { return m_v; }
    /** The method to get momentum.*/
    const TVector3& getMomentum() const { return m_momentum; }
    /** The method to get GlobalTime.*/
    float getGlobalTime() const { return m_globalTime; }

  private:

    int m_sensorID;          /**< ID of the sensor */
    float m_u;               /**< Local u coordinate */
    float m_v;               /**< Local v coordinate */
    TVector3 m_momentum;     /**< momentum in local coordinates */
    float m_globalTime;      /**< Global time. */

    ClassDef(PXDTrueHit, 1)
  };

} // end namespace Belle2

#endif
