/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef  BKLMTRACKING_H
#define  BKLMTRACKING_H

#include <framework/core/Module.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <bklm/dataobjects/BKLMTrack.h>
#include <bklm/modules/bklmTracking/BKLMTrackFinder.h>
#include <bklm/geometry/Module.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {

  //! This module perform straight line track finding and fitting for BKLM
  class BKLMTrackingModule: public Module {

  public:

    //! Constructor
    BKLMTrackingModule();

    //! Destructor
    virtual ~BKLMTrackingModule();

    //! Initialize at start of job
    virtual void initialize();

    //! begin run stuff
    virtual void beginRun();

    //! Unpack one event and create digits
    virtual void event();

    //! end run stuff
    virtual void endRun();

    //! Terminate at the end of job
    virtual void terminate();

    //! Judge if two hits come from the same sector
    bool sameSector(BKLMHit2d* hit1, BKLMHit2d* hit2);

    //! find the closest RecoTrack, match BKLMTrack to RecoTrack, if the matched RecoTrack is found, return true
    bool findClosestRecoTrack(BKLMTrack* bklmTrk, RecoTrack*& closestTrack);


  protected:

    //! Mean hit - trigger time (ns)
    double m_MeanDt;

    //! Coincidence window half-width for in-time KLM hits (ns)
    double m_MaxDt;

    //! whether match BKLMTrack to RecoTrack
    bool m_MatchToRecoTrack;

    //! angle required between RecoTrack and BKLMTrack, if openangle is larger than m_maxAngleRequired, they don't match
    double m_maxAngleRequired = 10;

  private:

    //! my defined sort function using layer number
    static bool sortByLayer(BKLMHit2d* hit1, BKLMHit2d* hit2);


  };
} // end namespace Belle2
#endif
