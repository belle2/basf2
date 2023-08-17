/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/geometry/KLMGeometryPar.h>
#include <klm/dataobjects/KLMHit2d.h>
#include <klm/dataobjects/KLMElementNumbers.h>

/* CLHEP headers. */
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Matrix/Vector.h>

/* C++ headers. */
#include <list>

namespace Belle2 {

  //! track fitting procedure
  class KLMTrackFitter {

  public:

    //! Default constructor
    KLMTrackFitter();

    //! Destructor
    ~KLMTrackFitter();

    //! do fit and returns chi square of the fit.
    double fit(std::list<KLMHit2d*>& listTrackPoint);


    //! Distance from track to a hit in the global system
    double globalDistanceToHit(KLMHit2d* hit,
                               double& error,
                               double& sigma);


    //! do fit in the global system
    double fit1dTrack(std::list< KLMHit2d* > hitList,
                      CLHEP::HepVector&  eta,
                      CLHEP::HepSymMatrix&  error,
                      int depDir,    int indDir);

    //! Get track parameters in the global system. y = p0 + p1 * x; y = p2 + p3 * z, if in local sector fit mode: y = p0 + p1 * x; z = p2 + p3 * x
    CLHEP::HepVector    getTrackParam()
    {
      return m_GlobalPar;
    }

    //! Get invariance matrix of track parameters in the global system.
    CLHEP::HepSymMatrix getTrackParamErr()
    {
      return m_GlobalErr;
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

    //! Chi square of the fit
    float getChi2()
    {
      return m_Chi2;
    }

    //! number of the hits on this track
    int getNumHit()
    {
      return m_NumHit;
    }

    //! Invalidate track
    void  inValidate()
    {
      m_Valid = false;
    }


  private:

    //! Is fit valid
    bool m_Valid;

    //! Is fit good
    bool m_Good;

    //! Chi square of fit
    float m_Chi2;

    //! the number of hits on this track
    int m_NumHit;

    //! track params in global system
    CLHEP::HepVector     m_GlobalPar;

    //! track params errors in global system
    CLHEP::HepSymMatrix  m_GlobalErr;

    //! pointer to GeometryPar singleton
    KLM::KLMGeometryPar*   m_GeoPar;

  };
} // end of namespace Belle2
