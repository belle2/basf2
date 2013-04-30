/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Peter Kvasnicka, Martin Ritter             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXDHIT_H
#define VXDHIT_H

#include <vxd/dataobjects/VxdID.h>
#include <vxd/dataobjects/VXDTrueHit.h>
#include <TVector3.h>

namespace Belle2 {
  /** \addtogroup dataobjects
    * @{
    */

  /**
    * Class VXDHit - Modified TrueHit type for testbeam studies.
    *
    * Along with information of VXDTrueHit, this class stores data about
    * whether the hit was created by a primary particle, the PDG code of the
    * primary (if applicable), number of related primaries, number of related
    * particles, and momentum of the generating particle.
    */
  class VXDHit : public VXDTrueHit {

  public:
    VXDHit(): VXDTrueHit(), m_fromPrimary(false), m_pdg(0), m_numPrimary(0),
      m_numParticles(0), m_particleMomentum(0.0)
    {}

    /** Constructor
     * @param sensorID SensorID of the Sensor
     * @param u u coordinate of the hit in local coordinates
     * @param v v coordinate of the hit in local coordinates
     * @param momentum momentum of the particle in local coordinates
     * @param globalTime timestamp of the hit
     */
    VXDHit(VxdID sensorID, float u, float v, float energyDep, float globalTime,
           const TVector3& momentum, const TVector3& entryMomentum, const TVector3& exitMomentum):
      VXDTrueHit(sensorID, u, v, energyDep, globalTime, momentum, entryMomentum, exitMomentum),
      m_fromPrimary(false), m_pdg(0), m_numPrimary(0), m_numParticles(0),
      m_particleMomentum(0.0)
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
    VXDHit(
      VxdID sensorID, float u, float v, float entryU, float entryV, float exitU, float exitV, float energyDep, float globalTime,
      const TVector3& momentum, const TVector3& entryMomentum, const TVector3& exitMomentum):
      VXDTrueHit(sensorID, u, v, entryU, entryV, exitU, exitV, energyDep, globalTime, momentum, entryMomentum, exitMomentum),
      m_fromPrimary(false), m_pdg(0), m_numPrimary(0), m_numParticles(0),
      m_particleMomentum(0.0)
    {}

    /** Returns true for hits with relation to primary particle*/
    bool getFromPrimary() const { return m_fromPrimary; }

    /** Set whether this hit has relation to a primary particle */
    void setFromPrimary(int isFromPrimary) { m_fromPrimary = isFromPrimary; }

    /** Get number of relations to primary particles */
    int getNumPrimary() const { return m_numPrimary; }

    /** Set whether this hit has relation to primary particle*/
    void setNumPrimary(int numPrimary) { m_numPrimary = numPrimary; }

    /** Returns true for hits with relation to a primary particle */
    int getNumParticles() const { return m_numParticles; }

    /** Set whether this hit has relation to a primary particle */
    void setNumParticles(int numParticles) { m_numParticles = numParticles; }

    /** Returns true from hits with relation to a primary particle */
    int getPdg() const { return m_pdg; }

    /** Set whether this hit has relation to primary particle */
    void setPdg(int pdg) { m_pdg = pdg; }

    /** Returns true from hits with relation to primary particle */
    double getParticleMomentum() const { return m_particleMomentum; }

    /** Set whether this hit has relation to a primary particle */
    void setParticleMomentum(double momentum) { m_particleMomentum = momentum; }



  protected:

    bool m_fromPrimary;         /**< True for hits with relation to primary particle */
    int m_pdg;                  /**< pdg of first related particle */
    int m_numPrimary;           /**< # of related primaries */
    int m_numParticles;         /**< # of related particles */
    double m_particleMomentum;  /**< momentum of related particle */

    ClassDef(VXDHit, 2)
  };

  /** @}*/

} // end namespace Belle2

#endif
