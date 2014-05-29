/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>
#include <TVector3.h>
#include <TMatrixDSym.h>

namespace Belle2 {

  //! Define state of extrapolation for each recorded hit
  enum ExtHitStatus { EXT_FIRST = -1, EXT_ENTER, EXT_EXIT, EXT_STOP, EXT_ESCAPE };

  //! Store one Ext hit as a ROOT object
  class ExtHit : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    ExtHit();

    //! Constructor with initial values
    //! @param pdgCode PDG hypothesis used for this extrapolation
    //! @param detector Detector containing this hit
    //! @param element Detector element containing this hit
    //! @param status State of extrapolation at this hit
    //! @param t Time of flight from start of event to this hit (ns)
    //! @param r Global position of this hit (cm)
    //! @param p Momentum of extrapolated track at this hit (GeV/c)
    //! @param e Covariance matrix of extrapolation at this hit (GeV/c and cm)
    ExtHit(int pdgCode, Const::EDetector detector, int element, ExtHitStatus status, double t, const TVector3& r, const TVector3& p, const TMatrixDSym& e);

    //! Copy constructor
    ExtHit(const ExtHit&);

    //! Destructor
    virtual ~ExtHit() {}

    //! Get PDG code of this extrapolation's hypothesis
    //! @return PDG code of this extrapolation's hypothesis
    int getPdgCode() const { return m_PdgCode; }

    //! Get detector ID of this extrapolation hit
    //! @return detector ID of this extrapolation hit
    Const::EDetector getDetectorID() const { return m_DetectorID; }

    //! Get detector-element ID of sensitive element within detector
    //! @return detector-element ID of sensitive element within detector
    int getCopyID() const { return m_CopyID; }

    //! Get state of extrapolation at this hit
    //! @return state of extrapolation at this hit
    ExtHitStatus getStatus() const { return m_Status; }

    //! Get time of flight from the point of closest approach near the origin to this hit
    //! @return time of flight from the point of closest approach near the origin to this hit (ns)
    double getTOF() const { return m_TOF; }

    //! Get position of this extrapolation hit
    //! @return position (cm) of this extrapolation hit
    TVector3 getPosition() const { return m_Position; }

    //! Get momentum at this extrapolation hit
    //! @return momentum (GeV/c) at this extrapolation hit
    TVector3 getMomentum() const { return m_Momentum; }

    //! Get phase-space covariance at this extrapolation hit
    //! @return phase-space covariance (6x6, position & momentum, cm & GeV/c) at this extrapolation hit
    TMatrixDSym getCovariance() const { return m_Covariance; }

  private:

    //! PDG code
    int m_PdgCode;

    //! detector ID
    Const::EDetector m_DetectorID;

    //! copy ID
    int m_CopyID;

    //! extrapolation state
    ExtHitStatus m_Status;

    //! time of flight (ns)
    double m_TOF;

    //! position (cm)
    TVector3 m_Position;

    //! momentum (GeV/c)
    TVector3 m_Momentum;

    //! phase-space covariance (6x6, position & momentum, cm & GeV/c)
    TMatrixDSym m_Covariance;

    //! Needed to make the ROOT object storable
    ClassDef(ExtHit, 3)

  };
}
