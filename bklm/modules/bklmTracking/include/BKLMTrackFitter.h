/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMTRACKFITTER_H
#define BKLMTRACKFITTER_H

#include <list>
#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Matrix/Matrix.h"
#include "CLHEP/Matrix/SymMatrix.h"
#include "CLHEP/Matrix/DiagMatrix.h"
#include "bklm/geometry/Module.h"
#include <bklm/geometry/GeometryPar.h>
#include "bklm/dataobjects/BKLMHit2d.h"


namespace Belle2 {

  namespace bklm {
    class Module;
  }

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

    //! do fit in the y-x plane or z-x plane
    double fit1dSectorTrack(std::list< BKLMHit2d* > hitList,
                            CLHEP::HepVector&  eta,
                            CLHEP::HepSymMatrix&  error,
                            int depDir,    int indDir);

    //! Get track parameters in the global system. y = p0 + p1 * x; z = p2 + p3 * x
    CLHEP::HepVector    getTrackParam() {return m_GlobalPar; }

    //! Get invariance matrix of track parameters in the global system.
    CLHEP::HepSymMatrix getTrackParamErr() {return m_GlobalErr; }

    //! Get track parameters in the sector locan system, where the first layer of the sector is used as reference.
    CLHEP::HepVector    getTrackParamSector() {return m_SectorPar; }

    //! Get invariance matrix of track parameters in the sector local system, where the first layer of the sector is used as reference.
    CLHEP::HepSymMatrix getTrackParamSectorErr() {return m_SectorErr; }

    //! Is fit valid
    bool isValid() { return m_Valid; }

    //! Is fit good
    bool  isGood()  { return m_Good; }

    //! Chi square of the fit
    float getChi2() { return m_Chi2; }

    //! number of the hits on this track
    int getNumHit() { return m_NumHit; }

    //! Invalidate track
    void  inValidate() { m_Valid = false; }

  private:

    //! Is fit valid
    bool m_Valid;

    //! Is fit good
    bool m_Good;

    //! Chi square of fit
    float m_Chi2;

    //! the number of hits on this track
    int m_NumHit;

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

    //! Hep3Vector indices
    //enum { VX = 0, VY = 1, VZ = 2 };

    //! fit vector and  Error matrix indices
    //enum { AY = 0, BY = 1, AZ = 2, BZ = 3 };

    //! a Matrix indices
    //enum { MY = 0, MZ = 1 };

  };
} // end of namespace Belle2
#endif

