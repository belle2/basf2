/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMTRACKFINDER_H
#define BKLMTRACKFINDER_H

#include <list>

#include <bklm/dataobjects/BKLMStatus.h>
#include "bklm/dataobjects/BKLMHit2d.h"
#include "bklm/modules/bklmTracking/BKLMTrackFitter.h"

namespace Belle2 {

  class BKLMTrackFinder {

  public:

    //! Default constructor
    BKLMTrackFinder();

    //! The track finder requires a track fitter.
    BKLMTrackFinder(BKLMTrackFitter* fitter);

    //! Destructor
    ~BKLMTrackFinder();

    //! Register a fitter if not constructed with one.
    void registerFitter(BKLMTrackFitter* fitter);

    //! find associated hits and do fit.
    bool filter(std::list<BKLMHit2d* >& seed,
                std::list<BKLMHit2d* >& hits,
                std::list<BKLMHit2d* >& track);

  protected:

    //! pointer to the fitter
    BKLMTrackFitter* m_Fitter;

  private:

  };

} // end of namespace Belle2

#endif
