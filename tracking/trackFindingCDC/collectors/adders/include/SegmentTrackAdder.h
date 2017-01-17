/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/collectors/adders/AdderInterface.h>
#include <tracking/trackFindingCDC/utilities/MayBePtr.h>
#include <map>

namespace Belle2 {
  class CDCHit;

  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCSegment2D;

    /** Class to add the matched segments to the track and set the taken flag correctly. */
    class SegmentTrackAdder : public AdderInterface<CDCTrack, CDCSegment2D> {
    public:
      /// Before the event starts, clear the map from hits to matched tracks.
      void beginEvent() override
      {
        m_hitMatchedTrackMap.clear();
      }

      /// Getter for the map from wire hits to matched tracks
      std::map<const CDCHit*, MayBePtr<const CDCTrack>>& getHitTrackMap()
      {
        return m_hitMatchedTrackMap;
      };
    private:
      /** Add the matched segment to the track and set the taken flag correctly. We ignore the weight completely here. */
      void add(CDCTrack& track, const CDCSegment2D& segment, Weight weight) override;

      std::map<const CDCHit*, MayBePtr<const CDCTrack>> m_hitMatchedTrackMap;
    };
  }
}
