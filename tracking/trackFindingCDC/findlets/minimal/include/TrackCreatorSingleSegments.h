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

  namespace TrackFindingCDC {
    class CDCSegment2D;
    class CDCTrack;

    /**
     *  Searches for segments that have not been used at all and creates tracks from them.
     *  Accepts number of hits a segment must exceed to be promoted to a track.
     *  This number can be set differently for each super layer
     *  Usually only the segments of the inner most super layer might be interesting to be treated as tracks.
     */
    class TrackCreatorSingleSegments : public Findlet<const CDCSegment2D, CDCTrack&> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCSegment2D, CDCTrack&>;

    public:
      /// Constructor setting up default parameters.
      TrackCreatorSingleSegments();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    public:
      /// Main algorithm
      void
      apply(const std::vector<CDCSegment2D>& segments, std::vector<CDCTrack>& tracks) final;

    private:
      /**
       *  Parameter: Map of super layer ids to minimum hit number
       *  for which left over segments shall be forwarded as tracks,
       *  if they exceed the minimal hit requirement.
       *
       *  Defaults to {{0, 15}}, meaning a segment with more then 15 hits
       *  in the first super layer will become a track.
       */
      std::map<ISuperLayer, size_t> m_param_minimalHitsBySuperLayerId;
    };
  }
}
