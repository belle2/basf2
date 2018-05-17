/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *               Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSIMHIT_H
#define ECLSIMHIT_H

#include <G4ThreeVector.hh>
#include <simulation/dataobjects/SimHitBase.h>

namespace Belle2 {


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
    ECLSimHit(): SimHitBase(), m_CellId(0), m_TrackId(0), m_Pdg(0), m_FlightTime(0), m_Edep(0), m_Momentum{0}, m_Position{0},
      m_HadronEdep{0} {;}

    //! Useful Constructor
    ECLSimHit(
      int CellId,            /**< Cell ID */
      int TrackId,           /**< Track ID */
      int Pdg,               /**< Particle PDG (can be one of secondaries) */
      double FlightTime,      /**< Flight time from IP */
      double Edep,            /**< Deposit energy */
      G4ThreeVector Momentum,     /**< Momentum */
      G4ThreeVector Position,       /**< Position */
      double Hadronedep              /**< Hadron edep */
    ): SimHitBase(), m_CellId(CellId), m_TrackId(TrackId), m_Pdg(Pdg), m_FlightTime(FlightTime), m_Edep(Edep), m_HadronEdep(Hadronedep)
    {
      m_Position[0] = Position.x(); m_Position[1] = Position.y(), m_Position[2] = Position.z();
      m_Momentum[0] = Momentum.x(); m_Momentum[1] = Momentum.y(), m_Momentum[2] = Momentum.z();
    }

    /*! Set Cell ID
     */
    void setCellId(int CellId) { m_CellId = CellId; }

    /*! Set Track ID
     */
    void setTrackId(int TrackId) { m_TrackId = TrackId; }

    /*! Set Particle PDG (can be one of secondaries)
     */
    void setPDGCode(int Pdg) { m_Pdg = Pdg; }

    /*! Set Flight time from IP
     */
    void setFlightTime(double FlightTime) { m_FlightTime = static_cast<double>(FlightTime); }

    /*! Set Deposit energy
     */
    void setEnergyDep(double Edep) { m_Edep = static_cast<double>(Edep); }

    /*! Set Momentum
     */
    void setMomentum(const G4ThreeVector& Momentum) { m_Momentum[0] = Momentum.x(); m_Momentum[1] = Momentum.y(), m_Momentum[2] = Momentum.z(); }

    /*! Set Position
     */
    //KM void setPosIn(G4ThreeVector Position) { m_Position = Position; }

    /*! Set Position
     */
    void setPosition(const G4ThreeVector& Position) { m_Position[0] = Position.x(); m_Position[1] = Position.y(), m_Position[2] = Position.z(); }

    /*! Get Cell ID
     * @return Cell ID
     */
    int getCellId() const { return m_CellId; }

    /*! Get Track ID
     * @return Track ID
     */
    int getTrackId() const { return m_TrackId; }

    /*! Get Particle PDG (can be one of secondaries)
     * @return Particle PDG (can be one of secondaries)
     */
    int getPDGCode() const { return m_Pdg; }

    /*! Get Flight time from IP
     * @return Flight time from IP
     */
    double getFlightTime() const { return m_FlightTime; }

    /*! Get Deposit energy
     * @return Deposit energy
     */
    double getEnergyDep() const { return m_Edep; }

    /*! Get Hadron Deposit energy
     * @return Hadron Deposit energy
     */
    double getHadronEnergyDep() const { return m_HadronEdep; }

    /*! Get Momentum
     * @return Momentum
     */
    G4ThreeVector getMomentum() const { return G4ThreeVector(m_Momentum[0], m_Momentum[1], m_Momentum[2]); }

    /*! Get Position
     * @return Position
     */
    G4ThreeVector getPosIn() const { return getPosition(); }

    /*! Get Position
     * @return Position
     */
    G4ThreeVector getPosition() const { return G4ThreeVector(m_Position[0], m_Position[1], m_Position[2]); }

    /** Shift the SimHit in time (needed for beam background mixing)
     * @param delta The value of the time shift.
     */
    void shiftInTime(float delta) { m_FlightTime += delta; }


  private:
    int m_CellId;            /**< Cell ID */
    int m_TrackId;           /**< Track ID */
    int m_Pdg;               /**< Particle PDG (can be one of secondaries) */
    double m_FlightTime;      /**< Flight time from IP */
    double m_Edep;            /**< Deposit energy */
    double m_Momentum[3];     /**< Momentum */
    double m_Position[3];        /**< Position */
    double m_HadronEdep;      /**< Hadron Energy Deposit */


    ClassDef(ECLSimHit, 6);/**< the class title */

  };
} // end namespace Belle2

#endif
