/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMElementNumbers.h>

/* Basf2 headers. */
#include <framework/datastore/RelationsObject.h>

/* ROOT headers. */
#include <TVectorD.h>
#include <Math/Vector3D.h>
#include <TMatrixDSym.h>

/* CLHEP headers. */
#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Matrix/SymMatrix.h>

namespace CLHEP {
  class HepVector;
  class HepSymMatrix;
}

namespace Belle2 {

  //! Store one KLM Track as a ROOT object
  class KLMTrack : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    KLMTrack();

    //! Copy constructor
    explicit KLMTrack(const KLMTrack&);

    //! Assignment operator
    KLMTrack& operator=(const KLMTrack&);

    //! Destructor
    ~KLMTrack()
    {
    }

    //! Get track parameters in the global system. y = p0 + p1 * x; z = p2 + p3 * x
    TVectorD  getTrackParam();

    //! Get invariance matrix of track parameters in the global system.
    TMatrixDSym  getTrackParamErr();

    //! Get the fitted chi2 of the track
    float getTrackChi2()
    {
      return m_Chi2;
    }

    //! Get the number of 2d hits on the track
    int getNumHitOnTrack()
    {
      return m_NumHit;
    }

    //! Is fit valid
    bool isValid()
    {
      return m_Valid;
    }

    //! Is fit good
    bool  isGood()
    {
      return m_Good;
    }

    //! Set track parameters in the global system. y = p0 + p1 * x; z = p2 + p3 * x
    void setTrackParam(const CLHEP::HepVector& trkPar);

    //! Set invariance matrix of track parameters in the global system.
    void setTrackParamErr(const CLHEP::HepSymMatrix& trkParErr);

    //! Set the fitted chi2 of the track
    void setTrackChi2(const float chi2)
    {
      m_Chi2 = chi2;
    }

    //! Set the number of 2d hits on the track
    void setNumHitOnTrack(const int NumHit)
    {
      m_NumHit = NumHit;
    }

    //! set the fit valid status
    void setIsValid(const bool valid)
    {
      m_Valid = valid;
    }

    //! setting subdetector flag (barrel or endcap)
    int setSubdetector(const int subdetector)
    {
      m_Subdetector = subdetector;
    }

    //! set the fit good status
    void setIsGood(const bool good)
    {
      m_Good = good;
    }

    bool inBKLM() const
    {
      return (m_Subdetector == KLMElementNumbers::c_BKLM);
    }

    bool inEKLM() const
    {
      return (m_Subdetector == KLMElementNumbers::c_EKLM);
    }

    int getSubdetector() const
    {
      return m_Subdetector;
    }



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

    //! which subdetector does the track hit
    int m_Subdetector;

    //! Needed to make the ROOT object storable
    ClassDef(KLMTrack, 1)

  };

} // end of namespace Belle2
