/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSIMHIT_H
#define ECLSIMHIT_H

#include <TVector3.h>
#include <generators/dataobjects/SimHitBase.h>

namespace Belle2 {

  /** \addtogroup dataobjects
   * @{
   */

  /** ClassECLSimHit - Geant4 simulated hit for the ECL.
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response information.
    * relation to MCParticle
    * filled in ecl/simulation/src/SensitiveDetector.cc
    */

  class ECLSimHit : public SimHitBase {
  public:
    /** default constructor for ROOT */
    ECLSimHit() {;}

    /*! Set Cell ID
     */
    void setCellId(int cellId) { m_cellId = cellId; }

    /*! Set Track ID
     */
    void setTrackId(int trackId) { m_trackId = trackId; }

    /*! Set Particle PDG (can be one of secondaries)
     */
    void setPDGCode(int pdg) { m_pdg = pdg; }

    /*! Set Flight time from IP
     */
    void setFlightTime(double flightTime) { m_flightTime = flightTime; }

    /*! Set Deposit energy
     */
    void setEnergyDep(double edep) { m_edep = edep; }

    /*! Set Momentum
     */
    void setMomentum(TVector3 momentum) { m_momentum = momentum; }

    /*! Set Position
     */
    void setPosIn(TVector3 posIn) { m_posIn = posIn; }


    /*! Get Cell ID
     * @return Cell ID
     */
    int getCellId() const { return m_cellId; }

    /*! Get Track ID
     * @return Track ID
     */
    int getTrackId() const { return m_trackId; }

    /*! Get Particle PDG (can be one of secondaries)
     * @return Particle PDG (can be one of secondaries)
     */
    int getPDGCode() const { return m_pdg; }

    /*! Get Flight time from IP
     * @return Flight time from IP
     */
    double getFlightTime() const { return m_flightTime; }

    /*! Get Deposit energy
     * @return Deposit energy
     */
    double getEnergyDep() const { return m_edep; }

    /*! Get Momentum
     * @return Momentum
     */
    TVector3 getMomentum() const { return m_momentum; }

    /*! Get Position
     * @return Position
     */
    TVector3 getPosIn() const { return m_posIn; }

    /** Shift the SimHit in time (needed for beam background mixing)
     * @param delta The value of the time shift.
     */
    void shiftInTime(float delta) {  m_flightTime += delta; }


  private:
    int m_cellId;            /**< Cell ID */
    int m_trackId;           /**< Track ID */
    int m_pdg;               /**< Particle PDG (can be one of secondaries) */
    double m_flightTime;     /**< Flight time from IP */
    double m_edep;           /**< Deposit energy */
    TVector3 m_momentum;     /**< momentum */
    TVector3 m_posIn;        /**< Position */


    ClassDef(ECLSimHit, 3);/**< the class title */

  };
  /** @}*/
} // end namespace Belle2

#endif
