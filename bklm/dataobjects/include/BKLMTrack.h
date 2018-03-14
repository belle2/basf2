/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMTRACK_H
#define BKLMTRACK_H

#include <framework/datastore/RelationsObject.h>
#include <TVectorD.h>
#include <TVector3.h>
#include <TMatrixDSym.h>

#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Matrix/SymMatrix.h"

namespace CLHEP {
  class HepVector;
  class HepSymMatrix;
}

namespace Belle2 {

  //! Store one BKLM Track as a ROOT object
  class BKLMTrack : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMTrack();

    //! Copy constructor
    explicit BKLMTrack(const BKLMTrack&);

    //! Assignment operator
    BKLMTrack& operator=(const BKLMTrack&);

    //! Destructor
    virtual ~BKLMTrack() {}

    //! Get track parameters in the global system. y = p0 + p1 * x; z = p2 + p3 * x
    TVectorD  getTrackParam();

    //! Get invariance matrix of track parameters in the global system.
    TMatrixDSym  getTrackParamErr();

    //! Get track parameters in the sector locan system, where the first layer of the sector is used as reference.
    TVectorD   getLocalTrackParam();

    //! Get invariance matrix of track parameters in the sector local system, where the first layer of the sector is used as reference.
    TMatrixDSym  getLocalTrackParamErr();

    //! Get the positon in local coordinate system of track intercept in plane of constant x
    TVector3 getLocalIntercept(double x);

    //! Get the variance matrix of (y,z) coordinates of the track intercept in plane of constant x in sector local system
    TMatrixD getLocalInterceptVariance(double x);

    //! Get the fitted chi2 of the track
    float getTrackChi2() { return m_Chi2; }

    //! Get the number of 2d hits on the track
    int getNumHitOnTrack() { return m_NumHit; }

    //! Is fit valid
    bool isValid() { return m_Valid; }

    //! Is fit good
    bool  isGood()  { return m_Good; }

    //! Set track parameters in the global system. y = p0 + p1 * x; z = p2 + p3 * x
    void setTrackParam(const CLHEP::HepVector& trkPar);

    //! Set invariance matrix of track parameters in the global system.
    void setTrackParamErr(const CLHEP::HepSymMatrix& trkParErr);

    //! Set track parameters in the sector local system, where the first layer of the sector is used as reference.
    void setLocalTrackParam(const CLHEP::HepVector& trkPar);

    //! Set invariance matrix of track parameters in the sector local system, where the first layer of the sector is used as reference.
    void setLocalTrackParamErr(const CLHEP::HepSymMatrix& trkParErr);

    //! Set the fitted chi2 of the track
    void setTrackChi2(const float chi2) {m_Chi2 = chi2; }

    //! Set the number of 2d hits on the track
    void setNumHitOnTrack(const int NumHit) { m_NumHit = NumHit; }

    //! set the fit valid status
    void setIsValid(const bool valid) {  m_Valid = valid; }

    //! set the fit good status
    void setIsGood(const bool good) {  m_Good = good; }

  private:

    //! Is fit valid
    bool m_Valid;

    //! Is fit good
    bool m_Good;

    //! fitted chi2 of the track
    float m_Chi2;

    //! the number of 2d hits on the track
    int m_NumHit;

    //! track parameters in the global system. y = p0 + p1 * x; z = p2 + p3 * x
    float m_TrackParam[4];

    //! track parameters variance in the global system.
    float m_TrackParamErr[4][4];

    //! track parameters in the sector local system.
    float m_LocalTrackParam[4];

    //! track parameters variance in the sector local system.
    float m_LocalTrackParamErr[4][4];

    //! Needed to make the ROOT object storable
    ClassDef(BKLMTrack, 1)

  };

} // end of namespace Belle2

#endif //BKLMTRACK_H
