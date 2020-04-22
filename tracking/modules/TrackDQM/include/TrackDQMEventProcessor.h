#pragma once

#include <tracking/dqmUtils/DQMEventProcessorBase.h>
#include <tracking/modules/TrackDQM/TrackDQMModule.h>

using namespace std;

namespace Belle2 {

  class TrackDQMEventProcessor : public DQMEventProcessorBase {

  public:
    TrackDQMEventProcessor(TrackDQMModule* histoModule, string recoTracksStoreArrayName, string tracksStoreArrayName) :
      DQMEventProcessorBase(histoModule, recoTracksStoreArrayName, tracksStoreArrayName) { }

  private:

  };
}