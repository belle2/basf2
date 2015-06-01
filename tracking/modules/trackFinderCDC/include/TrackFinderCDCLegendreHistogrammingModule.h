#pragma once
#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>
#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCTrack;
  }

  class CDCLegendreHistogrammingModule: public TrackFinderCDCBaseModule {

  public:
    CDCLegendreHistogrammingModule() : TrackFinderCDCBaseModule()
    {
      addParam("DebugOutput", m_param_debugOutput, "Flag to turn on debug output.", false);
    }

  private:
    /**
     * Do a QuadTreeHistogramming with all the StereoHits
     */
    void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override;

    bool m_param_debugOutput; /// Flag to turn on debug output
  };

}
