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
#include <G4ThreeVector.hh>
#include <G4ErrorSymMatrix.hh>

namespace Belle2 {

  //! Define state of extrapolation for each recorded hit
  enum ExtHitStatus { EXT_FIRST = -1, EXT_ENTER, EXT_EXIT, EXT_STOP, EXT_ESCAPE,
                      EXT_ECLCROSS, EXT_ECLDL, EXT_ECLNEAR
                    };

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
    //! @param backwardPropagation Direction of the track propagation.
    //! @param t Time of flight from start of event to this hit (ns)
    //! @param r Global position of this hit (cm)
    //! @param p Momentum of extrapolated track at this hit (GeV/c)
    //! @param e Covariance matrix of extrapolation at this hit (GeV/c and cm)
    ExtHit(int pdgCode, Const::EDetector detector, int element, ExtHitStatus status, bool backwardPropagation, double t,
           const TVector3& r, const TVector3& p, const TMatrixDSym& e);

    //! Constructor with initial values
    //! @param pdgCode PDG hypothesis used for this extrapolation
    //! @param detector Detector containing this hit
    //! @param element Detector element containing this hit
    //! @param status State of extrapolation at this hit
    //! @param backwardPropagation Direction of the track propagation.
    //! @param t Time of flight from IP to this hit (ns)
    //! @param r Global position of this hit (cm)
    //! @param p Momentum of extrapolated track at this hit (GeV/c)
    //! @param e Covariance matrix of extrapolation at this hit (GeV/c and cm)
    ExtHit(int pdgCode, Const::EDetector detector, int element, ExtHitStatus status, bool backwardPropagation, double t,
           const G4ThreeVector& r, const G4ThreeVector& p, const G4ErrorSymMatrix& e);

    //! Copy constructor
    ExtHit(const ExtHit&);

    //! Assignment operator
    ExtHit& operator=(const ExtHit&);

    //! Destructor
    virtual ~ExtHit() {}

    //! Set PDG code of this extrapolation's hypothesis
    void setPDGCode(int pdgCode) { m_PdgCode = pdgCode; }

    //! Set detector ID of this extrapolation hit
    void setDetectorID(Const::EDetector detectorID) { m_DetectorID = detectorID; }

    //! Set detector-element ID of sensitive element within detector
    void setCopyID(int copyID) { m_CopyID = copyID; }

    //! Set the direction of track propagation.
    void setBackwardPropagation(bool backwardPropagation) { m_BackwardPropagation = backwardPropagation; }

    //! Set state of extrapolation at this hit
    void setStatus(ExtHitStatus extHitStatus) { m_Status = extHitStatus; }

    //! Set time of flight from the point of closest approach near the origin to this hit
    void setTOF(double tof) { m_TOF = tof; }

    //! Set position of this extrapolation hit
    void setPosition(const TVector3& position)
    {
      m_Position[0] = position.x();
      m_Position[1] = position.y();
      m_Position[2] = position.z();
    }

    //! Set momentum at this extrapolation hit
    void setMomentum(const TVector3& momentum)
    {
      m_Momentum[0] = momentum.x();
      m_Momentum[1] = momentum.y();
      m_Momentum[2] = momentum.z();
    }

    //! Set phase-space covariance at this extrapolation hit
    void setCovariance(double covArray[21])
    {
      for (int k = 0; k < 21; ++k) {
        m_Covariance[k] = covArray[k];
      }
    }

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

    //! Get the direction of track propagation.
    bool isBackwardPropagated() const { return m_BackwardPropagation; }

    //! Get time of flight from the point of closest approach near the origin to this hit
    //! @return time of flight from the point of closest approach near the origin to this hit (ns)
    double getTOF() const { return m_TOF; }

    //! Get path length from start of extrapolation to closest approach to ECL cluster (for EXT_ECLDL only)
    //! @return path length (in radiation lengths)
    double getLength() const { return m_TOF; }

    //! Get position of this extrapolation hit
    //! @return position (cm) of this extrapolation hit
    TVector3 getPosition() const { return TVector3(m_Position[0], m_Position[1], m_Position[2]); }

    //! Get momentum at this extrapolation hit
    //! @return momentum (GeV/c) at this extrapolation hit
    TVector3 getMomentum() const { return TVector3(m_Momentum[0], m_Momentum[1], m_Momentum[2]); }

    //! Get phase-space covariance at this extrapolation hit
    //! @return phase-space covariance (symmetric 6x6: position & momentum in cm & GeV/c) at this extrapolation hit
    TMatrixDSym getCovariance() const;

    //! Update the parameters of this extrapolation hit
    //! @param status State of extrapolation at this hit
    //! @param t Time of flight from IP to this hit (ns)
    //! @param r Global position of this hit (cm)
    //! @param p Momentum of extrapolated track at this hit (GeV/c)
    //! @param e Covariance matrix of extrapolation at this hit (GeV/c and cm)
    void update(ExtHitStatus status, double t,
                const G4ThreeVector& r, const G4ThreeVector& p, const G4ErrorSymMatrix& e);

    //! Get the uncertainty in the azimuthal angle phi (radians)
    double getErrorTheta() const;

    //! Get the uncertainty in the azimuthal angle phi (radians)
    double getErrorPhi() const;

  private:

    //! Get diagonal elemment of the 3x3 position-covariance matrix in polar coordinates (r, theta, phi)
    double getPolarCovariance(int i) const;

    //! PDG code
    int m_PdgCode;

    //! detector ID
    Const::EDetector m_DetectorID;

    //! copy ID
    int m_CopyID;

    //! extrapolation state
    ExtHitStatus m_Status;

    //! Direction of track propagation.
    bool m_BackwardPropagation;

    //! time of flight (ns)
    float m_TOF;

    //! position (cm)
    float m_Position[3];

    //! momentum (GeV/c)
    float m_Momentum[3];

    //! phase-space covariance (symmetric 6x6 linearized to 6*(6+1)/2: position & momentum in cm & GeV/c)
    float m_Covariance[21];

    //! Needed to make the ROOT object storable
    ClassDef(ExtHit, 8);

  };
}
