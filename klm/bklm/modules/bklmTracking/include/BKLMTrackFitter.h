/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/dataobjects/bklm/BKLMHit2d.h>

/* CLHEP headers. */
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Matrix/Vector.h>

/* C++ headers. */
#include <list>

namespace Belle2 {

  //! track fitting procedure
  class BKLMTrackFitter {

  public:

    //! Default constructor
    BKLMTrackFitter();

    //! Destructor
    ~BKLMTrackFitter();

    //! do fit and returns chi square of the fit.
    double fit(std::list<BKLMHit2d*>& listTrackPoint);

    //! Distance from track to a hit in the plane of the module
    double distanceToHit(BKLMHit2d* hit,
                         double& error,
                         double& sigma);

    //! Distance from track to a hit in the global system
    double globalDistanceToHit(BKLMHit2d* hit,
                               double& error,
                               double& sigma);

    //! do fit in the y-x plane or z-x plane
    double fit1dSectorTrack(std::list< BKLMHit2d* > hitList,
                            CLHEP::HepVector&  eta,
                            CLHEP::HepSymMatrix&  error,
                            int depDir,    int indDir);

    //! do fit in the global system
    double fit1dTrack(std::list< BKLMHit2d* > hitList,
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

    //! Get track parameters in the sector locan system, y = p0 + p1 * x, z = p2 + p3 *x, where the first layer of the sector is used as reference.
    CLHEP::HepVector    getTrackParamSector()
    {
      return m_SectorPar;
    }

    //! Get invariance matrix of track parameters in the sector local system, where the first layer of the sector is used as reference.
    CLHEP::HepSymMatrix getTrackParamSectorErr()
    {
      return m_SectorErr;
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

    //!  set the fitting mode, local system or global system
    void  setGlobalFit(bool localOrGlobal)
    {
      m_globalFit = localOrGlobal;
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

    //! do fit in the local system or global system false: local sys; true: global sys.
    bool m_globalFit;

    //! track params in the sector local system
    CLHEP::HepVector     m_SectorPar;

    //! track params errors in the sector local system
    CLHEP::HepSymMatrix  m_SectorErr;

    //! track params in global system
    CLHEP::HepVector     m_GlobalPar;

    //! track params errors in global system
    CLHEP::HepSymMatrix  m_GlobalErr;

    //! pointer to GeometryPar singleton
    bklm::GeometryPar*   m_GeoPar;

  };
} // end of namespace Belle2
