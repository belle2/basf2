/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Guofu Cao                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SIMHITCDC_H
#define SIMHITCDC_H

#include <framework/datastore/DataStore.h>

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class SimHitCDC : public TObject {
  public:

    //! The layer id of this hit.
    int m_layerId;

    //! The wire id of this hit.
    int m_wireId;

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

    //! The position on wire which is closest to this hit.
    TVector3 m_posWire;

    //! Position of pre-step.
    TVector3 m_posIn;

    //! Position of post-step.
    TVector3 m_posOut;

    //! The flag to denote this hit is in the left or right side.
    int m_posFlag;

    //! The method to set layer id
    void setLayerId(int layerId) { m_layerId = layerId; }

    //! The method to set wire id
    void setWireId(int wireId) { m_wireId = wireId; }

    //! The method to set track id
    void setTrackId(int trackId) { m_trackId = trackId; }

    //! The method to set PDG code
    void setPDGCode(int pdg) { m_pdg = pdg; }

    //! The method to set drift length
    void setDriftLength(double driftLength) { m_driftLength = driftLength; }

    //! The method to set flight time
    void setFlightTime(double flightTime) { m_flightTime = flightTime; }

    //! The method to set deposited energy
    void setEnergyDep(double edep) { m_edep = edep; }

    //! The method to set step length
    void setStepLength(double stepLength) { m_stepLength = stepLength; }

    //! The method to set momentum
    void setMomentum(TVector3 momentum) { m_momentum = momentum; }

    //! The method to set position on wire
    void setPosWire(TVector3 posWire) { m_posWire = posWire; }

    //! The method to set position of pre-step
    void setPosIn(TVector3 posIn) { m_posIn = posIn; }

    //! The method to set position of post-step
    void setPosOut(TVector3 posOut) { m_posOut = posOut; }

    //! The method to set position flag
    void setPosFlag(int posFlag) { m_posFlag = posFlag; }

    //! The method to get layer id
    int getLayerId() const { return m_layerId; }

    //! The method to get wire id
    int getWireId() const { return m_wireId; }

    //! The method to get track id
    int getTrackId() const { return m_trackId; }

    //! The method to get PDG code
    int getPDGCode() const { return m_pdg; }

    //! The method to get drift length
    double getDriftLength() const { return m_driftLength; }

    //! The method to get flight time
    double getFlightTime() const { return m_flightTime; }

    //! The method to get deposited energy
    double getEnergyDep() const { return m_edep; }

    //! The method to get step length
    double getStepLength() const { return m_stepLength; }

    //! The method to get momentum
    TVector3 getMomentum() const { return m_momentum; }

    //! The method to get position on wire
    TVector3 getPosWire() const { return m_posWire; }

    //! The method to get position of pre-step
    TVector3 getPosIn() const { return m_posIn; }

    //! The method to get position of post-step
    TVector3 getPosOut() const { return m_posOut; }

    //! The method to get position flag
    int getPosFlag() const { return m_posFlag; }

    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    SimHitCDC() {;}

    //! Useful Constructor
    SimHitCDC(int layerId,
              int wireId,
              int trackId,
              int pdg,
              double driftLength,
              double flightTime,
              double edep,
              double stepLength,
              TVector3 momentum,
              TVector3 posWire,
              TVector3 posIn,
              TVector3 posOut,
              int posFlag) {
      m_layerId = layerId;
      m_wireId = wireId;
      m_trackId = trackId;
      m_pdg = pdg;
      m_driftLength = driftLength;
      m_flightTime = flightTime;
      m_edep = edep;
      m_stepLength = stepLength;
      m_momentum = momentum;
      m_posWire = posWire;
      m_posIn = posIn;
      m_posOut = posOut;
      m_posFlag = posFlag;
    }

    ClassDef(SimHitCDC, 1);

  };

} // end namespace Belle2

#endif
