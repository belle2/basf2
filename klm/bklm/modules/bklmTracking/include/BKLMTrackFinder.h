/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMHit2d.h>
#include <klm/bklm/modules/bklmTracking/BKLMTrackFitter.h>

/* C++ headers. */
#include <list>

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
    bool filter(const std::list<BKLMHit2d* >& seed,
                std::list<BKLMHit2d* >& hits,
                std::list<BKLMHit2d* >& track);

    //!  set the fitting mode, local system or global system
    void  setGlobalFit(bool localOrGlobal)
    {
      m_globalFit = localOrGlobal;
    }

  protected:

    //! pointer to the fitter
    BKLMTrackFitter* m_Fitter;

    //! do fit in the local system or global system false: local sys; true: global sys.
    bool m_globalFit;

  private:

  };

} // end of namespace Belle2
