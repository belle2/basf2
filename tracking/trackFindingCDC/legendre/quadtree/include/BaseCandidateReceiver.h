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
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Base class that receives candidates found by quadtree.
     */
    class BaseCandidateReceiver {

    public:
      /// Constructor
      explicit BaseCandidateReceiver(std::vector<const CDCWireHit*> allAxialWireHits);
      /// Destructor
      virtual ~BaseCandidateReceiver();
      /// Main entry point for the post processing call from the QuadTreeProcessor
      virtual void operator()(const std::vector<const CDCWireHit*>& inputWireHits, void* qt);
      /// Get the collected tracks
      const std::vector<CDCTrack>& getTracks() const;

    protected:
      /// Pool of all axial hits from which the road search may select additional hits
      std::vector<const CDCWireHit*> m_allAxialWireHits;

      /// Collected tracks
      std::vector<CDCTrack> m_tracks;
    };
  }
}
