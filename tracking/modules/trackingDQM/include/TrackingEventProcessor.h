#pragma once

#include <tracking/dqmUtils/BaseDQMEventProcessor.h>
#include <tracking/modules/trackingDQM/TrackDQMModule.h>

#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <vxd/geometry/SensorInfoBase.h>

using namespace std;

namespace Belle2 {

  class TrackingEventProcessor : public BaseDQMEventProcessor {

  public:
    TrackingEventProcessor(TrackDQMModule* p_histoModule, std::string p_m_RecoTracksStoreArrayName,
                           std::string p_m_TracksStoreArrayName) :
      BaseDQMEventProcessor(p_histoModule, p_m_RecoTracksStoreArrayName, p_m_TracksStoreArrayName) { }

  private:
    TString ConstructMessage() override;
  };
}