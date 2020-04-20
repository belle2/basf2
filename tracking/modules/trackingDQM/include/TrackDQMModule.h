/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for track quality check                                       *
 * Prepared for Phase 2 and Belle II geometry                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/dqmUtils/BaseDQMHistogramModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <TH1F.h>
#include <TH2F.h>

#include <mdst/dataobjects/EventLevelTrackingInfo.h>

#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>

using namespace std;

namespace Belle2 {

  /** DQM of tracks
    * their momentum,
    * Number of hits in tracks,
    * Number of tracks.
    *
    */
  class TrackDQMModule : public BaseDQMHistogramModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    TrackDQMModule();
    /* Destructor */
    ~TrackDQMModule();

    /** Module functions */
    void initialize() override final;
    //void beginRun() override final;
    void event() override final;

    /**
    * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
    * to be placed in this function.
    */
    void defineHisto() override final;

  private:

    virtual void DefineFlags();

    /** Monitors the Error flags set by the tracking code. As of the time of implementation there only were two flags:
      VXDTF2AbortionFlag, i.e. how often the VXDTF2 did abort the event and did not produce tracks,
      and UnspecifiedTrackFindingFailure
      The histogram records if any flag was set.
    */
    TH1F* m_trackingErrorFlags = nullptr;
  };  //end class declaration
}  // end namespace Belle2