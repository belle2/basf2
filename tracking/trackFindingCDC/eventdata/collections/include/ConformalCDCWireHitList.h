/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/hits/ConformalCDCWireHit.h>
#include <vector>

using namespace std;

namespace Belle2 {

  namespace TrackFindingCDC {

    /**
     * Object which holds the list of ConformalCDCWireHit.
     * It is more or less only a wrapper to a std::vector with some additional functions for convenience.
     */
    class ConformalCDCWireHitList {
    public:
      /// Empty constructor does nothing.
      ConformalCDCWireHitList() { };

      /// Do not copy this class.
      ConformalCDCWireHitList(const ConformalCDCWireHitList& copy) = delete;

      /// Do not copy this class.
      ConformalCDCWireHitList& operator=(const ConformalCDCWireHitList&) = delete;

      /// Perform the provided function to all tracks.
      void doForAllHits(std::function<void(ConformalCDCWireHit& hit)> function)
      {
        for (ConformalCDCWireHit& hit : m_conformalCDCWireHitList) {
          function(hit);
        }
      }

      /// Clear the list of tracks at the end of the event.
      void clear()
      {
        m_conformalCDCWireHitList.clear();
      }

      /// Get the list of CDCTracks.
      std::vector<ConformalCDCWireHit>& getConformalCDCWireHitList()
      {
        return m_conformalCDCWireHitList;
      };

    private:
      std::vector<ConformalCDCWireHit> m_conformalCDCWireHitList; /**< List containing ConformalCDCWireHit objects. */
    };
  }
}

