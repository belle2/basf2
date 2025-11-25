/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMHit2d.h>
#include <klm/modules/KLMTracking/KLMTrackFitter.h>

/* C++ headers. */
#include <list>

namespace Belle2 {

  //! track finding procedure
  class KLMTrackFinder {

  public:

    //! Default constructor
    KLMTrackFinder();

    //! The track finder requires a track fitter.
    explicit KLMTrackFinder(KLMTrackFitter* fitter);

    //! Destructor
    ~KLMTrackFinder();

    //! Register a fitter if not constructed with one.
    void registerFitter(KLMTrackFitter* fitter);

    //! find associated hits and do fit.
    bool filter(const std::list<KLMHit2d* >& seed,
                std::list<KLMHit2d* >& hits,
                std::list<KLMHit2d* >& track,
                int iSubdetector);



  protected:

    //! pointer to the fitter
    KLMTrackFitter* m_Fitter;


  private:

  };

} // end of namespace Belle2
