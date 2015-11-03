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

#include <tracking/trackFindingCDC/eventdata/hits/QuadTreeHitWrapper.h>
#include <list>

using namespace std;

namespace Belle2 {

  namespace TrackFindingCDC {

    /**
     * Object which holds the list of QuadTreeHitWrappers.
     * It is more or less only a wrapper to a std::vector with some additional functions for convenience.
     */
    class QuadTreeHitWrapperList {
    public:
      /// Empty constructor does nothing.
      QuadTreeHitWrapperList() { };

      /// Do not copy this class.
      QuadTreeHitWrapperList(const QuadTreeHitWrapperList& copy) = delete;

      /// Do not copy this class.
      QuadTreeHitWrapperList& operator=(const QuadTreeHitWrapperList&) = delete;

      /// Perform the provided function to all tracks.
      void doForAllHits(std::function<void(QuadTreeHitWrapper& hit)> function)
      {
        for (QuadTreeHitWrapper& hit : m_quadTreeHitWrapperList) {
          function(hit);
        }
      }

      /// Clear the list of tracks at the end of the event.
      void clear()
      {
        m_quadTreeHitWrapperList.clear();
      }

      /// Get the list of CDCTracks.
      std::vector<QuadTreeHitWrapper>& getQuadTreeHitWrapperList()
      {
        return m_quadTreeHitWrapperList;
      };

    private:
      std::vector<QuadTreeHitWrapper> m_quadTreeHitWrapperList; /**< List containing QuadTreeHitWrapper objects. */
    };
  }
}

