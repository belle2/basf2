#pragma once
#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>
#include <framework/datastore/StoreArray.h>
#include <vector>

namespace genfit {
  class Track;
}

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCTrack;
  }


  class StereoHitFinderCDCVXDMergerModule: public TrackFinderCDCBaseModule {

  public:
    StereoHitFinderCDCVXDMergerModule() : TrackFinderCDCBaseModule(),
      m_param_VXDGenfitTracksStoreArrayName("VXDTracks"),
      m_param_minimalDistanceToMerge(0.5),
      m_param_maximumAllowedDeviationAngle(0.5)
    {
    }

    void initialize() override
    {
      TrackFinderCDCBaseModule::initialize();
      StoreArray<genfit::Track> vxdGFTracks(m_param_VXDGenfitTracksStoreArrayName);
      vxdGFTracks.isRequired();
    }

  private:
    /**
     * Do a CDCVXDMerging with all the StereoHits
     */
    void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override;

    /// Parameter
    std::string m_param_VXDGenfitTracksStoreArrayName; /**< Name of the VXD tracks from genfit */
    double m_param_minimalDistanceToMerge;
    double m_param_maximumAllowedDeviationAngle;
  };


}
