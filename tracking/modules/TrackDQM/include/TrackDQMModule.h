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

#include <tracking/dqmUtils/DQMHistoModuleBase.h>

#include <mdst/dataobjects/EventLevelTrackingInfo.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>

#include <TH1F.h>
#include <TH2F.h>


using namespace std;

namespace Belle2 {

  /** DQM of tracks
    * their momentum,
    * Number of hits in tracks,
    * Number of tracks.
    *
    */
  class TrackDQMModule : public DQMHistoModuleBase {  // <- derived from HistoModule class

  public:

    /** Constructor */
    TrackDQMModule();
    /* Destructor */
    ~TrackDQMModule() { }

    /** Module functions */
    virtual void initialize() override;
    //void beginRun() override final;
    virtual void event() override;

    /**
    * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
    * to be placed in this function.
    */
    virtual void defineHisto() override;

  private:

    virtual void DefineFlags();

    /// Acccess to the EventLevelTrackingInfo object in the datastore.
    StoreObjPtr<EventLevelTrackingInfo> m_eventLevelTrackingInfo;

    /** Monitors the Error flags set by the tracking code. As of the time of implementation there only were two flags:
      VXDTF2AbortionFlag, i.e. how often the VXDTF2 did abort the event and did not produce tracks,
      and UnspecifiedTrackFindingFailure
      The histogram records if any flag was set.
    */
    TH1F* m_trackingErrorFlags = nullptr;
  };
}