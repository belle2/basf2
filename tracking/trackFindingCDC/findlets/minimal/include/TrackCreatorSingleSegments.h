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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <map>
#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
    class CDCTrack;

    /**
     *  Searches for segments that have not been used at all and creates tracks from them.
     *  Accepts number of hits a segment must exceed to be promoted to a track.
     *  This number can be set differently for each super layer
     *  Usually only the segments of the inner most super layer might be interesting to be treated as tracks.
     */
    class TrackCreatorSingleSegments : public Findlet<const CDCRecoSegment2D, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCRecoSegment2D, CDCTrack>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    public:
      /// Main algorithm
      void
      apply(const std::vector<CDCRecoSegment2D>& segments, std::vector<CDCTrack>& tracks) final;

    private:
      /**
       *  Parameter: Map of super layer ids to minimum hit number
       *  for which left over segments shall be forwarded as tracks,
       *  if they exceed the minimal hit requirement.
       *
       *  Defaults to empty map, meaning no segments are promoted.
       */
      std::map<ISuperLayer, size_t> m_param_minimalHitsForSingleSegmentTrackBySuperLayerId{};
    };
  }
}
