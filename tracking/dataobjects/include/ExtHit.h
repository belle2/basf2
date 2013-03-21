/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EXTHIT_H
#define EXTHIT_H

#include <framework/datastore/RelationsObject.h>
#include <TVector3.h>
#include <TMatrixD.h>

namespace Belle2 {

  enum ExtHitStatus { EXT_FIRST = -1, EXT_ENTER, EXT_EXIT, EXT_STOP, EXT_ESCAPE };
  enum ExtDetectorID { EXT_UNKNOWN = -1, EXT_PXD, EXT_SVD, EXT_CDC, EXT_TOP, EXT_ARICH, EXT_ECL, EXT_BKLM, EXT_EKLM };

  //! Store one Ext hit as a ROOT object
  class ExtHit : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    ExtHit();

    //! Constructor with initial values
    ExtHit(int, ExtDetectorID, int, ExtHitStatus, double, const TVector3&, const TVector3&, const TMatrixD&);

    //! Copy constructor
    ExtHit(const ExtHit&);

    //! Destructor
    virtual ~ExtHit() {}

    //! returns PDG code of this extrapolation's hypothesis
    int getPdgCode() const { return m_PdgCode; }

    //! returns detector ID of this extrapolation hit
    ExtDetectorID getDetectorID() const { return m_DetectorID; }

    //! returns copy ID of sensitive element within detector
    int getCopyID() const { return m_CopyID; }

    //! returns status of this extrapolation hit
    ExtHitStatus getStatus() const { return m_Status; }

    //! returns time of flight from the point of closest approach to the origin (ns)
    double getTOF() const { return m_TOF; }

    //! returns position (cm) of this extrapolation hit
    TVector3 getPosition() const { return m_Position; }

    //! returns momentum (GeV/c) at this extrapolation hit
    TVector3 getMomentum() const { return m_Momentum; }

    //! returns phase-space covariance (6x6, position--momentum) at this extrapolation hit
    TMatrixD getCovariance() const { return m_Covariance; }

  private:

    //! PDG code
    int m_PdgCode;

    //! detector ID
    ExtDetectorID m_DetectorID;

    //! copy ID
    int m_CopyID;

    //! status
    ExtHitStatus m_Status;

    //! time of flight (ns)
    double m_TOF;

    //! position (cm)
    TVector3 m_Position;

    //! momentum (GeV/c)
    TVector3 m_Momentum;

    //! phase-space covariance (6x6, position--momentum)
    TMatrixD m_Covariance;

    //! Needed to make the ROOT object storable
    ClassDef(ExtHit, 1)

  };

} // end of namespace Belle2

#endif //EXTHIT_H
