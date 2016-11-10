/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <vector>
#include <map>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Searches for segments that have not been used at all and creates tracks from them.
     *  Accepts number of hits a segment must exceed to be promoted to a track.
     *  This number can be set differently for each super layer
     *  Usually only the segments of the inner most super layer might be interesting to be treated as tracks
     */
    class TrackCreatorSingleSegments:
      public Findlet<const CDCRecoSegment2D, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCRecoSegment2D, CDCTrack>;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Creates a track for each segments that is yet unused by any of the given tracks.";
      }

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override final
      {
        moduleParamList->addParameter(prefixed(prefix, "MinimalHitsForSingleSegmentTrackBySuperLayerId"),
                                      m_param_minimalHitsForSingleSegmentTrackBySuperLayerId,
                                      "Map of super layer ids to minimum hit number, "
                                      "for which left over segments shall be forwarded as tracks, "
                                      "if the exceed the minimal hit requirement. Default empty.",
                                      m_param_minimalHitsForSingleSegmentTrackBySuperLayerId);
      }

    public:
      /// Main algorithm
      virtual void apply(const std::vector<CDCRecoSegment2D>& segments,
                         std::vector<CDCTrack>& tracks) override final
      {
        // Create tracks from left over segments
        // First figure out which segments do not share any hits with any of the given tracks
        // (if the tracks vector is empty this is the case for all segments)
        for (const CDCRecoSegment2D& segment : segments) {
          segment.unsetAndForwardMaskedFlag();
        }

        for (const CDCTrack& track : tracks) {
          track.unsetAndForwardMaskedFlag();
        }

        for (const CDCRecoSegment2D& segment : segments) {
          segment.receiveMaskedFlag();
        }

        if (not m_param_minimalHitsForSingleSegmentTrackBySuperLayerId.empty()) {
          for (const CDCRecoSegment2D& segment : segments) {
            if (segment.getAutomatonCell().hasMaskedFlag()) continue;

            ISuperLayer iSuperLayer = segment.getISuperLayer();
            if (m_param_minimalHitsForSingleSegmentTrackBySuperLayerId.count(iSuperLayer) and
                segment.size() >= m_param_minimalHitsForSingleSegmentTrackBySuperLayerId[iSuperLayer]) {

              if (segment.getTrajectory2D().isFitted()) {
                tracks.push_back(CDCTrack(segment));
                segment.setAndForwardMaskedFlag();
                for (const CDCRecoSegment2D& otherSegment : segments) {
                  otherSegment.receiveMaskedFlag();
                }
              }
            }
          }
        }
      }

    private:
      /**
       *  Parameter: Map of super layer ids to minimum hit number
       *  for which left over segments shall be forwarded as tracks, if they exceed the minimal hit requirement.
       *
       *  Default empty.
       */
      std::map<ISuperLayer, size_t> m_param_minimalHitsForSingleSegmentTrackBySuperLayerId{};

    };
  }
}
