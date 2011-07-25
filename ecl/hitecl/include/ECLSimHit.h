/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Guofu Cao                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSIMHIT_H
#define ECLSIMHIT_H

#include <framework/datastore/DataStore.h>

#include <TObject.h>
#include <TVector3.h>

#define DEFAULT_ECLSIMHITS           "ECLSimHits"
#define DEFAULT_MCPART_TO_ECLSIMHITS "MCPartToECLSimHits"

namespace Belle2 {

  //! Example Detector
  class ECLSimHit : public TObject {
  public:

    //! The cell id of this hit.
    int m_cellId;

    //! The theta id of this hit.
    int m_thetaId;

    //! The phi id of this hit.
    int m_phiId;


    //! The track id of this hit.
    int m_trackId;

    //! Particle PDG (can be one of secondaries).
    int m_pdg;

    //! Drift length of this hit.
    double m_driftLength;

    //!  Flight time from IP.
    double m_flightTime;

    //! Deposited energy of this hit.
    double m_edep;

    //! Step length of this hit.
    double m_stepLength;

    //! The momentum of pre-step.
    TVector3 m_momentum;

    //! The position on cell which is closest to this hit.
    TVector3 m_posCell;

    //! Position of pre-step.
    TVector3 m_posIn;

    //! Position of post-step.
    TVector3 m_posOut;

    //! The flag to denote this hit is in the left or right side.
    int m_posFlag;

    //! The method to set theta id
    void setThetaId(int thetaId) { m_thetaId = thetaId; }

    //! The method to set theta id
    void setPhiId(int phiId) { m_phiId = phiId; }

    //! The method to set cell id
    void setCellId(int cellId) { m_cellId = cellId; }

    //! The method to set track id
    void setTrackId(int trackId) { m_trackId = trackId; }

    //! The method to set PDG code
    void setPDGCode(int pdg) { m_pdg = pdg; }


    //! The method to set flight time
    void setFlightTime(double flightTime) { m_flightTime = flightTime; }

    //! The method to set deposited energy
    void setEnergyDep(double edep) { m_edep = edep; }

    //! The method to set step length
    void setStepLength(double stepLength) { m_stepLength = stepLength; }

    //! The method to set momentum
    void setMomentum(TVector3 momentum) { m_momentum = momentum; }

    //! The method to set position on cell
    void setPosCell(TVector3 posCell) { m_posCell = posCell; }

    //! The method to set position of pre-step
    void setPosIn(TVector3 posIn) { m_posIn = posIn; }

    //! The method to set position of post-step
    void setPosOut(TVector3 posOut) { m_posOut = posOut; }

    //! The method to set position flag
    void setPosFlag(int posFlag) { m_posFlag = posFlag; }

    //! The method to get theta id
    int getThetaId() const { return m_thetaId; }

    //! The method to get phi id
    int getPhiId() const { return m_phiId; }


    //! The method to get cell id
    int getCellId() const { return m_cellId; }

    //! The method to get track id
    int getTrackId() const { return m_trackId; }

    //! The method to get PDG code
    int getPDGCode() const { return m_pdg; }


    //! The method to get flight time
    double getFlightTime() const { return m_flightTime; }

    //! The method to get deposited energy
    double getEnergyDep() const { return m_edep; }

    //! The method to get step length
    double getStepLength() const { return m_stepLength; }

    //! The method to get momentum
    TVector3 getMomentum() const { return m_momentum; }

    //! The method to get position on Cell
    TVector3 getPosCell() const { return m_posCell; }

    //! The method to get position of pre-step
    TVector3 getPosIn() const { return m_posIn; }

    //! The method to get position of post-step
    TVector3 getPosOut() const { return m_posOut; }

    //! The method to get position flag
    int getPosFlag() const { return m_posFlag; }

    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    ECLSimHit() {;}

    //! Useful Constructor
    ECLSimHit(
      int cellId,
      int thetaId,
      int phiId,
      int trackId,
      int pdg,
      double flightTime,
      double edep,
      double stepLength,
      TVector3 momentum,
      TVector3 posCell,
      TVector3 posIn,
      TVector3 posOut,
      int posFlag) {
      m_thetaId = thetaId;
      m_phiId = phiId;
      m_cellId = cellId;
      m_trackId = trackId;
      m_pdg = pdg;
      m_flightTime = flightTime;
      m_edep = edep;
      m_stepLength = stepLength;
      m_momentum = momentum;
      m_posCell = posCell;
      m_posIn = posIn;
      m_posOut = posOut;
      m_posFlag = posFlag;
    }

    ClassDef(ECLSimHit, 1);

  };

} // end namespace Belle2

#endif
