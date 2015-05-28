#pragma once
#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>
#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCTrack;
  }

  class CDCLegendreHistogrammingModule: public TrackFinderCDCBaseModule {

  private:
    /**
     * Do a QuadTreeHistogramming with all the StereoHits
     */
    void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override;
  };

}
