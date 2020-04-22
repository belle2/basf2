#pragma once

#include <tracking/dqmUtils/DQMEventProcessorBase.h>
#include <alignment/modules/AlignDQM/AlignDQMModule.h>

#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <vxd/geometry/SensorInfoBase.h>

using namespace std;

namespace Belle2 {

  class AlignDQMEventProcessor : public DQMEventProcessorBase {

  public:
    AlignDQMEventProcessor(AlignDQMModule* histoModule, string tracksStoreArrayName, string recoTracksStoreArrayName) :
      DQMEventProcessorBase(histoModule, recoTracksStoreArrayName, tracksStoreArrayName) { }

  private:
    virtual TString ConstructMessage() override;

    virtual void FillCommonHistograms() override;
  };
}