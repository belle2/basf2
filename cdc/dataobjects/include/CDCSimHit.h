/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <cdc/dataobjects/WireID.h>
#include <simulation/dataobjects/SimHitBase.h>

#include <TVector3.h>

namespace Belle2 {
  //! Example Detector
  class CDCSimHit : public SimHitBase {

  private:

    //! The WireID of the hit.
    WireID m_wireID;

    //! The track id of this hit.
    unsigned short m_trackId;

    //! Particle PDG (can be one of secondaries).
    int m_pdg;

    //! Drift length of this hit.
    float m_driftLength;

    //!  Flight time from IP.
    float m_flightTime;

    //! Deposited energy of this hit.
    float m_edep;

    //! Step length of this hit.
    float m_stepLength;

    //! The momentum at closest point.
    //    TVector3 m_momentum;
    float m_momentum[3];

    //! The position on wire which is closest to this hit.
    //    TVector3 m_posWire;
    float m_posWire[3];

    //! Position of pre-step.
    //    TVector3 m_posIn;
    float m_posIn[3];

    //! Position of post-step.
    //    TVector3 m_posOut;
    float m_posOut[3];

    //! Position on the track.
    //    TVector3 m_posTrack;
    float m_posTrack[3];

    /**
     * The flag to denote this hit is in the left or right side.
     * bit0: old L/R flag;
     * bit1: new L/R flag for digitization;
     * bit2: new L/R flag for tracking.
     */
    unsigned char m_leftRight;

    /** Time of energy deposition */
    float m_globalTime;


  public:
    /** Setter for Wire ID.
     *
     *  @param iCLayerID   number of layer with continuous counting method.
     *  @param iWireID     number of wire within the layer.
     */
    void setWireID(int iCLayerID, int iWireID)
    {
      m_wireID.setWireID(iCLayerID, iWireID);
    }

    //! The method to set track id
    void setTrackId(int trackId) { m_trackId = (unsigned short)trackId; }

    //! The method to set PDG code
    void setPDGCode(int pdg) { m_pdg = pdg; }

    //! The method to set drift length
    void setDriftLength(double driftLength) { m_driftLength = driftLength; }

    //! The method to set flight time
    void setFlightTime(double flightTime) { m_flightTime = flightTime; }

    //! The method to set global time
    void setGlobalTime(double globalTime) { m_globalTime = globalTime; }

    //! The method to set deposited energy
    void setEnergyDep(double edep) { m_edep = edep; }

    //! The method to set step length
    void setStepLength(double stepLength) { m_stepLength = stepLength; }

    //! The method to set momentum
    void setMomentum(TVector3 momentum)
    {
      m_momentum[0] = momentum.X();
      m_momentum[1] = momentum.Y();
      m_momentum[2] = momentum.Z();
    }

    //! The method to set position on wire
    void setPosWire(TVector3 posWire)
    {
      m_posWire[0] = posWire.X();
      m_posWire[1] = posWire.Y();
      m_posWire[2] = posWire.Z();
    }

    //! The method to set position of pre-step
    void setPosIn(TVector3 posIn)
    {
      m_posIn[0] = posIn.X();
      m_posIn[1] = posIn.Y();
      m_posIn[2] = posIn.Z();
    }

    //! The method to set position of post-step.
    void setPosOut(TVector3 posOut)
    {
      m_posOut[0] = posOut.X();
      m_posOut[1] = posOut.Y();
      m_posOut[2] = posOut.Z();
    }

    //! The method to set position on the track.
    void setPosTrack(TVector3 posTrack)
    {
      m_posTrack[0] = posTrack.X();
      m_posTrack[1] = posTrack.Y();
      m_posTrack[2] = posTrack.Z();
    }

    /**
     * The method to set position flag.
     */

    void setPosFlag(int zeroOrOne)
    {
      m_leftRight &= 0x6;
      m_leftRight = (m_leftRight | (unsigned char)zeroOrOne);
    }

    //! The method to set new left/right info. for digitization
    void setLeftRightPassageRaw(int minusOneOrZeroOrOne)
    {
      int zeroOrOne = (minusOneOrZeroOrOne <= 0) ? 0 : 1;
      m_leftRight &= 0x5;
      m_leftRight = (m_leftRight | ((unsigned char)zeroOrOne << 1));
    }

    //! The method to set new left/right info. for tracking
    void setLeftRightPassage(int minusOneOrZeroOrOne)
    {
      int zeroOrOne = (minusOneOrZeroOrOne <= 0) ? 0 : 1;
      m_leftRight &= 0x3;
      m_leftRight = (m_leftRight | ((unsigned char)zeroOrOne << 2));
    }

    /** Getter for WireID object. */
    WireID getWireID() const {return m_wireID;}

    //! The method to get track id.
    int getTrackId() const { return m_trackId; }

    //! The method to get PDG code.
    int getPDGCode() const { return m_pdg; }

    //! The method to get drift length.
    double getDriftLength() const { return m_driftLength; }

    //! The method to get flight time.
    double getFlightTime() const { return m_flightTime; }

    //! The method to get deposited energy.
    double getEnergyDep() const { return m_edep; }

    //! The method to get step length.
    double getStepLength() const { return m_stepLength; }

    //! The method to get momentum.
    TVector3 getMomentum() const
    {
      return TVector3(m_momentum[0], m_momentum[1], m_momentum[2]);
    }

    //! The method to get position on wire.
    TVector3 getPosWire() const
    {
      return TVector3(m_posWire[0], m_posWire[1], m_posWire[2]);
    }

    //! The method to get position of pre-step.
    TVector3 getPosIn() const
    {
      return TVector3(m_posIn[0], m_posIn[1], m_posIn[2]);
    }

    //! The method to get position of post-step.
    TVector3 getPosOut() const
    {
      return TVector3(m_posOut[0], m_posOut[1], m_posOut[2]);
    }

    //! The method to get position on the track
    TVector3 getPosTrack() const
    {
      return TVector3(m_posTrack[0], m_posTrack[1], m_posTrack[2]);
    }

    //! The method to get old left/right info.
    int getPosFlag() const
    {
      return (int)(m_leftRight & 0x1);
    }

    //! The method to get new left/right info. for digitization
    int getLeftRightPassageRaw() const
    {
      //      int minusOneOrOne = (int((m_leftRight & 0x2) >> 1) == 0) ? -1 : 1;
      //      return minusOneOrOne;
      int zeroOrOne = (int((m_leftRight & 0x2) >> 1) == 0) ? 0 : 1;
      return zeroOrOne;
    }

    //! The method to get new left/right info. for tracking
    int getLeftRightPassage() const
    {
      //      int minusOneOrOne = (int((m_leftRight & 0x4) >> 2) == 0) ? -1 : 1;
      //      return minusOneOrOne;
      int zeroOrOne = (int((m_leftRight & 0x4) >> 2) == 0) ? 0 : 1;
      return zeroOrOne;
    }


    //! The method to get global time
    float getGlobalTime() const override { return m_globalTime; }

    //! Empty constructor
    /*! Recommended for ROOT IO
    */

    CDCSimHit(): SimHitBase(),
      m_trackId(0), m_pdg(0), m_driftLength(0.0),
      m_flightTime(0.0), m_edep(0.0), m_stepLength(1.0),
      m_momentum(), m_posWire(),
      m_posIn(), m_posOut(), m_posTrack(),
      m_leftRight(0), m_globalTime(0.0)
    {}

    //! Useful Constructor
    CDCSimHit(int layerId,
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
              TVector3 posTrack,
              int leftRight,
              double globalTime): SimHitBase()
    {

      m_wireID.setWireID(layerId, wireId);
      m_trackId = trackId;
      m_pdg = pdg;
      m_driftLength = driftLength;
      m_flightTime = flightTime;
      m_edep = edep;
      m_stepLength = stepLength;
      m_momentum[0] = momentum.X();
      m_momentum[1] = momentum.Y();
      m_momentum[2] = momentum.Z();
      m_posWire[0] = posWire.X();
      m_posWire[1] = posWire.Y();
      m_posWire[2] = posWire.Z();
      m_posIn[0] = posIn.X();
      m_posIn[1] = posIn.Y();
      m_posIn[2] = posIn.Z();
      m_posOut[0] = posOut.X();
      m_posOut[1] = posOut.Y();
      m_posOut[2] = posOut.Z();
      m_posTrack[0] = posTrack.X();
      m_posTrack[1] = posTrack.Y();
      m_posTrack[2] = posTrack.Z();
      m_leftRight = (unsigned char)leftRight & 0x7; // mask 3 bits (LSB).
      m_globalTime = globalTime;
    }

    /** Shift the SimHit in time
     * @param delta The value of the time shift.
     */
    virtual void shiftInTime(float delta) override
    {
      m_globalTime += delta;
    }


    /** ROOT Macro. */
    ClassDefOverride(CDCSimHit, 6);
  };
} // end namespace Belle2
