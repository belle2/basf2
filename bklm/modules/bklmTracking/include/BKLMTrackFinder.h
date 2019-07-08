/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <list>

#include <bklm/dataobjects/BKLMStatus.h>
#include "bklm/dataobjects/BKLMHit2d.h"
#include "bklm/modules/bklmTracking/BKLMTrackFitter.h"

namespace Belle2 {

  //! track finding procedure
  class BKLMTrackFinder {

  public:

    //! Default constructor
    BKLMTrackFinder();

    //! The track finder requires a track fitter.
    explicit BKLMTrackFinder(BKLMTrackFitter* fitter);

    //! Destructor
    ~BKLMTrackFinder();

    //! Register a fitter if not constructed with one.
    void registerFitter(BKLMTrackFitter* fitter);

    //! find associated hits and do fit.
    bool filter(std::list<BKLMHit2d* >& seed,
                std::list<BKLMHit2d* >& hits,
                std::list<BKLMHit2d* >& track);

    //!  set the fitting mode, local system or global system
    void  setGlobalFit(bool localOrGlobal) { m_globalFit = localOrGlobal; }

  protected:

    //! pointer to the fitter
    BKLMTrackFitter* m_Fitter;

    //! do fit in the local system or global system false: local sys; true: global sys.
    bool m_globalFit;

  private:

  };

} // end of namespace Belle2
