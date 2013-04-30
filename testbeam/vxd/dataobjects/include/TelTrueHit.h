/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Peter Kvasnicka, Martin Ritter             *
 *               Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.              *
 **************************************************************************/

#ifndef TelTrueHit_H
#define TelTrueHit_H

#include <vxd/dataobjects/VxdID.h>

// ROOT
#include <framework/datastore/RelationsObject.h>
#include <TVector3.h>

namespace Belle2 {

  /**
    * Class TelTrueHit - Position where are particle traversed the detector plane.
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
  class TelTrueHit : public RelationsObject {
  public:
    /** Default constructor for ROOT IO */
    TelTrueHit(): RelationsObject(),
      m_sensorID(0), m_u(0), m_v(0), m_entryU(0), m_entryV(0),
      m_exitU(0), m_exitV(0), m_energyDep(0), m_globalTime(0)
    {}

    /** Constructor
     * @param sensorID SensorID of the Sensor
     * @param u u coordinate of the hit in local coordinates
     * @param v v coordinate of the hit in local coordinates
     * @param momentum momentum of the particle in local coordinates
     * @param entryMomentum momentum of the particle in local coordinates when entering silicon
     * @param exitMomentum momentum of the particle in local coordinates when exiting silicon
     * @param globalTime timestamp of the hit
     */
    TelTrueHit(
      VxdID sensorID, float u, float v, float energyDep, float globalTime,
      const TVector3& momentum, const TVector3& entryMomentum,
      const TVector3& exitMomentum):
      RelationsObject(),
      m_sensorID(sensorID), m_u(u), m_v(v), m_entryU(0), m_entryV(0),
      m_exitU(0), m_exitV(0),  m_energyDep(energyDep), m_globalTime(globalTime),
      m_momentum(momentum), m_entryMomentum(entryMomentum),
      m_exitMomentum(exitMomentum)
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
    TelTrueHit(
      VxdID sensorID, float u, float v, float entryU, float entryV,
      float exitU, float exitV, float energyDep, float globalTime,
      const TVector3& momentum, const TVector3& entryMomentum,
      const TVector3& exitMomentum):
      RelationsObject(),
      m_sensorID(sensorID), m_u(u), m_v(v), m_entryU(entryU), m_entryV(entryV),
      m_exitU(exitU), m_exitV(exitV), m_energyDep(energyDep), m_globalTime(globalTime),
      m_momentum(momentum), m_entryMomentum(entryMomentum),
      m_exitMomentum(exitMomentum)
    {}

    /** Return the Sensor ID */
    unsigned short getSensorID() const { return m_sensorID; }
    /** Return local u coordinate of hit */
    float getU() const { return m_u; }
    /** Return local v coordinate of hit */
    float getV() const { return m_v; }
    /** Return local u coordinate of hit when entering silicon*/
    float getEntryU() const { return m_entryU; }
    /** Return local v coordinate of hit when entering silicon*/
    float getEntryV() const { return m_entryV; }
    /** Return local u coordinate of hit  when exiting silicon*/
    float getExitU() const { return m_exitU; }
    /** Return local v coordinate of hit  when exiting silicon*/
    float getExitV() const { return m_exitV; }
    /** Return energy deposited during traversal of sensor */
    float getEnergyDep() const { return m_energyDep; }
    /** Return Time of hit.*/
    float getGlobalTime() const { return m_globalTime; }
    /** Return momentum when crossing detector plane.*/
    const TVector3& getMomentum() const { return m_momentum; }
    /** Return momentum when entering silicon.*/
    const TVector3& getEntryMomentum() const { return m_entryMomentum; }
    /** Return momentum when exiting silicon.*/
    const TVector3& getExitMomentum() const { return m_exitMomentum; }

  protected:

    int m_sensorID;           /**< ID of the sensor */
    float m_u;                /**< Local u coordinate when crossing detector plane*/
    float m_v;                /**< Local v coordinate when crossing detector plane*/
    float m_entryU;           /**< Local u coordinate when entering silicon */
    float m_entryV;           /**< Local v coordinate when entering silicon */
    float m_exitU;            /**< Local u coordinate when exiting silicon*/
    float m_exitV;            /**< Local v coordinate when exiting silicon*/
    float m_energyDep;        /**< Deposited energy while traversing sensor */
    float m_globalTime;       /**< Global time. */
    TVector3 m_momentum;      /**< momentum in local coordinates when crossing detector plane */
    TVector3 m_entryMomentum; /**< momentum in local coordinates when entering silicon */
    TVector3 m_exitMomentum;  /**< momentum in local coordinates when exiting silicon */

    ClassDef(TelTrueHit, 3)
  };

} // end namespace Belle2

#endif
