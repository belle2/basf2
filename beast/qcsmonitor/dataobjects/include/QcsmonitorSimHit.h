/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef QCSMONITORSIMHIT_H
#define QCSMONITORSIMHIT_H

#include <simulation/dataobjects/SimHitBase.h>

// ROOT
#include <TVector3.h>

namespace Belle2 {

  /** ClassQcsmonitorSimHit - Geant4 simulated hit for the Qcsmonitor crystal in beast..
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response information.
    * relation to MCParticle
    * filled in beast/qcsmonitor/simulation/src/SensitiveDetector.cc
    */

  class QcsmonitorSimHit : public SimHitBase {
  public:
    /** default constructor for ROOT */
    QcsmonitorSimHit(): SimHitBase(), m_CellId(0), m_TrackId(0), m_Pdg(0), m_FlightTime(0), m_Edep(0), m_Momentum(0, 0, 0),
      m_Position(0, 0, 0) {;}
    //! Useful Constructor
    QcsmonitorSimHit(
      int CellId,            /**< Cell ID */
      int TrackId,           /**< Track ID */
      int Pdg,               /**< Particle PDG (can be one of secondaries) */
      float FlightTime,      /**< Flight time from IP */
      float Edep,            /**< Deposit energy */
      TVector3 Momentum,     /**< Momentum */
      TVector3 Position        /**< Position */
    ): SimHitBase(), m_CellId(CellId), m_TrackId(TrackId), m_Pdg(Pdg), m_FlightTime(FlightTime), m_Edep(Edep), m_Momentum(Momentum),
      m_Position(Position)
    {
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
    void setFlightTime(double FlightTime) { m_FlightTime = (float) FlightTime; }

    /*! Set Deposit energy
     */
    void setEnergyDep(double Edep) { m_Edep = (float) Edep; }

    /*! Set Momentum
     */
    void setMomentum(TVector3 Momentum) { m_Momentum = Momentum; }

    /*! Set Position
     */
    void setPosIn(TVector3 Position) { m_Position = Position; }

    /*! Set Position
     */
    void setPosition(TVector3 Position) { m_Position = Position; }


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
    double getFlightTime() const { return (double)m_FlightTime; }

    /*! Get Deposit energy
     * @return Deposit energy
     */
    double getEnergyDep() const { return (double) m_Edep; }

    /*! Get Momentum
     * @return Momentum
     */
    TVector3 getMomentum() const { return m_Momentum; }

    /*! Get Position
     * @return Position
     */
    TVector3 getPosIn() const { return m_Position; }

    /*! Get Position
     * @return Position
     */
    TVector3 getPosition() const { return m_Position; }


    /** Shift the SimHit in time (needed for beam background mixing)
     * @param delta The value of the time shift.
     */
    void shiftInTime(float delta) override {  m_FlightTime += delta; }


  private:
    int m_CellId;            /**< Cell ID */
    int m_TrackId;           /**< Track ID */
    int m_Pdg;               /**< Particle PDG (can be one of secondaries) */
    float m_FlightTime;      /**< Flight time from IP */
    float m_Edep;            /**< Deposit energy */
    TVector3 m_Momentum;     /**< Momentum */
    TVector3 m_Position;        /**< Position */

    ClassDefOverride(QcsmonitorSimHit, 1)
  };

} // end namespace Belle2

#endif
