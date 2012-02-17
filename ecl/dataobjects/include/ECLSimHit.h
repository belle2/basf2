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

    //! The track id of this hit.
    int m_trackId;

    //! Particle PDG (can be one of secondaries).
    int m_pdg;

    //!  Flight time from IP.
    double m_flightTime;

    //! Deposited energy of this hit.
    double m_edep;

    //! Step length of this hit.
    double m_FirstStep;

    //! The momentum of pre-step.
    TVector3 m_momentum;

    //! Position of pre-step.
    TVector3 m_posIn;

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

    //! The method to set momentum
    void setMomentum(TVector3 momentum) { m_momentum = momentum; }


    //! The method to set position of pre-step
    void setPosIn(TVector3 posIn) { m_posIn = posIn; }


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

    //! The method to get momentum
    TVector3 getMomentum() const { return m_momentum; }


    TVector3 getPosIn() const { return m_posIn; }

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
      double FirstStep,
      TVector3 momentum,
      TVector3 posCell,
      TVector3 posIn,
      TVector3 posOut,
      int posFlag) {
      m_cellId = cellId;
      m_trackId = trackId;
      m_pdg = pdg;
      m_flightTime = flightTime;
      m_edep = edep;
      m_momentum = momentum;
      m_posIn = posIn;
    }

    ClassDef(ECLSimHit, 2);/**< the class title */

  };

} // end namespace Belle2

#endif
