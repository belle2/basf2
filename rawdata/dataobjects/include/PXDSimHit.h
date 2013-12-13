/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Peter Kvasnicka, Martin Ritter             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDSIMHIT_H
#define PXDSIMHIT_H

#include <generators/dataobjects/SimHitBase.h>

// ROOT
#include <TVector3.h>
#include <vxd/dataobjects/VxdID.h>

namespace Belle2 {
  /** \addtogroup dataobjects
    * @{
    */

  /**
    * ClassPXDSimHit - Geant4 simulated hit for the PXD.
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response, they contain _local_
    * information.
    */
  class PXDSimHit : public SimHitBase {
  public:
    /** default constructor for ROOT */
    PXDSimHit(): SimHitBase(), m_sensorID(0), m_pdg(0), m_theta(0), m_energyDep(0), m_globalTime(0),
      m_posIn(0, 0, 0), m_posOut(0, 0, 0), m_momIn(0, 0, 0) {}

    /** Standard constructor
     * @param sensorID ID of the sensor
     * @param pdg PDG Code of the particle producing the Hit
     * @param theta Theta angle of particle
     * @param energyDep Deposited energy in electrons
     * @param globalTime Time of energy deposition
     * @param posIn Start point of energy deposition in local coordinates
     * @param posOut End point of energy deposition in local coordinates
     * @param momIn Momentum of particle at start of energy deposition
     */
    PXDSimHit(VxdID sensorID, int pdg, float theta, float energyDep, float globalTime,
              const TVector3& posIn, const TVector3& posOut, const TVector3& momIn):
      SimHitBase(), m_sensorID(sensorID), m_pdg(pdg), m_theta(theta), m_energyDep(energyDep),
      m_globalTime(globalTime), m_posIn(posIn), m_posOut(posOut), m_momIn(momIn) {}

    /** Return the sensorID of the sensor the energy was deposited in */
    VxdID getSensorID()         const { return m_sensorID; }
    /** Return the PDG code of the particle causing the energy deposition */
    int getPDGcode()            const { return m_pdg; }
    /** Return the start point of the energy deposition in local coordinates */
    const TVector3& getPosIn()  const { return m_posIn; }
    /** Return the end point of the energy deposition in local coordinates */
    const TVector3& getPosOut() const { return m_posOut; }
    /** Return the momentum at the start point in local coordinates */
    const TVector3& getMomIn()  const { return m_momIn; }
    /** Return the theta angle */
    float getTheta()            const { return m_theta; }
    /** Return the energy deposition in electrons */
    float getEnergyDep()        const { return m_energyDep; }
    /** Return the time of the energy deposition */
    float getGlobalTime()       const { return m_globalTime; }
    /** Shift the SimHit in time
     * @param delta The value of the time shift.
     */
    virtual void shiftInTime(float delta) { m_globalTime += delta; }

  private:
    /** ID of the sensor the energy was deposited in */
    unsigned short m_sensorID;
    /** PDG Code of the particle producing the Hit */
    int m_pdg;
    /** Theta angle of particle */
    float m_theta;
    /** Deposited energy in electrons */
    float m_energyDep;
    /** Time of energy deposition */
    float m_globalTime;
    /** Start point of energy deposition in local coordinates */
    TVector3 m_posIn;
    /** End point of energy deposition in local coordinates */
    TVector3 m_posOut;
    /** Momentum of particle at start of energy deposition */
    TVector3 m_momIn;

    ClassDef(PXDSimHit, 2)
  };

  /** @}*/

} // end namespace Belle2

#endif
