/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun, Dmitrii Neverov               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/processing/AxialTrackUtil.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHit;
    class Vector2D;

    /**
     *  Base class that receives candidates found by quadtree.
     */
    class BaseCandidateReceiver {

    public:
      /// Constructor
      BaseCandidateReceiver(std::vector<const CDCWireHit*> allAxialWireHits)
        : m_allAxialWireHits(std::move(allAxialWireHits))
      {
      }
      /// Destructor
      virtual ~BaseCandidateReceiver()
      {
      }
      /// Main entry point for the post processing call from the QuadTreeProcessor
      virtual void operator()(const std::vector<const CDCWireHit*>& inputWireHits, void* qt __attribute__((unused)))
      {
        AxialTrackUtil::addCandidateFromHits(inputWireHits,
                                             m_allAxialWireHits,
                                             m_tracks,
                                             false);
      }

      /// Get the collected tracks
      const std::vector<CDCTrack>& getTracks() const
      {
        return m_tracks;
      }

    protected:
      /// Pool of all axial hits from which the road search may select additional hits
      std::vector<const CDCWireHit*> m_allAxialWireHits;

      /// Collected tracks
      std::vector<CDCTrack> m_tracks;
    };
  }
}
