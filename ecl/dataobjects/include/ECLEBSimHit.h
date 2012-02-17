/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Guofu Cao                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLEBSIMHIT_H
#define ECLEBSIMHIT_H

#include <TObject.h>
#include <TVector3.h>

#define DEFAULT_ECLEBSIMHITS           "ECLEBSimHits"
#define DEFAULT_MCPART_TO_ECLEBSIMHITS "MCPartToECLEBSimHits"

namespace Belle2 {

  //! Example Detector
  class ECLEBSimHit : public TObject {
  public:

    //! The cell id of this hit.
    int m_cellId;

    //! The track id of this hit.
    int m_trackId;

    //! Particle PDG (can be one of secondaries).
    int m_pdg;

    //!  Flight time from IP.
    double m_flightLength;

    //! Deposited energy of this hit.
    double m_edep;

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
    void setFlightLength(double flightLength) { m_flightLength = flightLength; }

    //! The method to set deposited energy
    void setEnergyDep(double edep) { m_edep = edep; }

    //! The method to set momentum
    void setMomentum(TVector3 momentum) { m_momentum = momentum; }

    //! The method to set position of pre-step
    void setPosIn(TVector3 posIn) { m_posIn = posIn; }

    //! The method to get theta id

    //! The method to get cell id
    int getCellId() const { return m_cellId; }

    //! The method to get track id
    int getTrackId() const { return m_trackId; }

    //! The method to get PDG code
    int getPDGCode() const { return m_pdg; }


    //! The method to get flight time
    double getFlightLength() const { return m_flightLength; }

    //! The method to get deposited energy
    double getEnergyDep() const { return m_edep; }

    //! The method to get momentum
    TVector3 getMomentum() const { return m_momentum; }

    //! The method to get position of pre-step
    TVector3 getPosIn() const { return m_posIn; }


    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    ECLEBSimHit() {;}

    //! Useful Constructor
    ECLEBSimHit(
      int cellId,
      int thetaId,
      int phiId,
      int trackId,
      int pdg,
      double flightLength,
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
      m_flightLength = flightLength;
      m_edep = edep;
      m_momentum = momentum;
      m_posIn = posIn;
    }

    ClassDef(ECLEBSimHit, 1);/**< the class title */

  };

} // end namespace Belle2

#endif
