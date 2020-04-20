#pragma once

#include <tracking/dqmUtils/BaseDQMEventProcessor.h>
#include <alignment/modules/AlignmentDQM/AlignDQMModule.h>

#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <vxd/geometry/SensorInfoBase.h>

using namespace std;

namespace Belle2 {
  class AlignmentEventProcessor : public BaseDQMEventProcessor {
  public:
    AlignmentEventProcessor(AlignDQMModule* p_histoModule, string p_m_TracksStoreArrayName, string p_m_RecoTracksStoreArrayName) :
      BaseDQMEventProcessor(p_histoModule, p_m_RecoTracksStoreArrayName, p_m_TracksStoreArrayName) { }

  private:
    TString ConstructMessage() override;

    void FillCommonHistograms() override;
  };
}