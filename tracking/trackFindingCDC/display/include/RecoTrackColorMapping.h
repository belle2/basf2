/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/display/ColorMapping.h>
#include <tracking/trackFindingCDC/display/Mapping.h>

namespace Belle2 {
  class RecoTrack;
  namespace TrackFindingCDC {

    /// Interface class for reco track to color map functional objects.
    using RecoTrackColorMapping = Mapping<const RecoTrack>;

    /// Color mapping using the default color cycle list
    using RecoTrackListColorsColorMap = DefaultColorCycleMapping<const RecoTrack>;

    /// Color mapping showing the match status of the pattern recognition track
    class RecoTrackMatchingStatusColorMap : public RecoTrackColorMapping {

    public:
      /// Function call to map a reco tracks object to a color
      std::string map(int index, const RecoTrack& recoTrack) override;

      /// Short description of the coloring scheme
      std::string info() override;
    };

    /// Color mapping showing the match status of the monte carlo track
    class MCRecoTrackMatchingStatusColorMap : public RecoTrackColorMapping {

    public:
      /// Function call to map a reco tracks object to a color
      std::string map(int index, const RecoTrack& recoTrack) override;

      /// Short description of the coloring scheme
      std::string info() override;
    };
  }
}
