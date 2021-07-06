/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/collectors/selectors/SingleMatchSelector.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackNormalizer.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/trackFindingCDC/utilities/HitComperator.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCSegment2D;
    class CDCRecoHit3D;

    /**
     * Add the matched segments to the tracks and normalize the tracks afterwards.
     * Also deletes all hits from the tracks, that are part of segments, that were not matched to these tracks.
     */
    class SegmentTrackAdderWithNormalization
      : public Findlet<WeightedRelation<CDCTrack, const CDCSegment2D>&, CDCTrack&, const CDCSegment2D> {

    private:
      /// Type of the base class
      using Super = Findlet<WeightedRelation<CDCTrack, const CDCSegment2D>&, CDCTrack&, const CDCSegment2D>;

    public:
      /// Constructor for registering the sub-findlets
      SegmentTrackAdderWithNormalization();

      /// Expose the parameters of the sub-findlets.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// Short description of the findlet
      std::string getDescription() override;

      /// Apply the findlet
      void apply(std::vector<WeightedRelation<CDCTrack, const CDCSegment2D>>& relations,
                 std::vector<CDCTrack>& tracks, const std::vector<CDCSegment2D>& segment) override;

    private:
      // Parameters
      /// Parameter : Swtich to remove hits in segments that have no matching track from all tracks
      bool m_param_removeUnmatchedSegments = true;

      // Findlets
      /// The selector for finding the track each hit should belong to.
      SingleMatchSelector<CDCTrack, CDCRecoHit3D, HitComperator> m_singleHitSelector;

      /// Findlet for performing the normalization of the tracks afterwards
      TrackNormalizer m_trackNormalizer;
    };
  }
}
