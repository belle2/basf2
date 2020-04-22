#pragma once

#include <tracking/dqmUtils/DQMEventProcessorBase.h>
#include <tracking/modules/TrackDQM/TrackDQMModule.h>

#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <vxd/geometry/SensorInfoBase.h>

using namespace std;

namespace Belle2 {

  class TrackDQMEventProcessor : public DQMEventProcessorBase {

  public:
    TrackDQMEventProcessor(TrackDQMModule* histoModule, string recoTracksStoreArrayName, string tracksStoreArrayName) :
      DQMEventProcessorBase(histoModule, recoTracksStoreArrayName, tracksStoreArrayName) { }

  private:
    virtual TString ConstructMessage() override;
  };
}