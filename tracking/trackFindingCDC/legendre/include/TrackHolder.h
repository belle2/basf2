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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <list>
#include <vector>

using namespace std;

namespace Belle2 {

  namespace TrackFindingCDC {

    /**
     * Object which holds list of created CDCTracks
     */
    class TrackHolder {
    public:

      /// Constructor
      TrackHolder() : m_cdcTracks() { };

      /**
       * Do not copy this class
       */
      TrackHolder(const TrackHolder& copy) = delete;

      /**
       * Do not copy this class
       */
      TrackHolder& operator=(const TrackHolder&) = delete;

      /// Create empty CDCTrack object at the end of the list
      CDCTrack& createEmptyTrack()
      {
        m_cdcTracks.emplace_back();
        return m_cdcTracks.back();
      };

      /// Delete track from the list.
      void deleteTrack(CDCTrack& track)
      {
        m_cdcTracks.remove(track);
      }

      /// Perform provided function to all tracks
      void doForAllTracks(std::function<void(CDCTrack& track)> function)
      {
        for (std::list<CDCTrack>::iterator it = m_cdcTracks.begin(); it !=  m_cdcTracks.end(); ++it) {
          function(*it);
        }
      }

      /// Reset all masked hits
      void resetMaskedHits()
      {
        doForAllTracks([](CDCTrack & track) {
          for (CDCRecoHit3D& hit : track) {
            hit.getWireHit().getAutomatonCell().setTakenFlag(true);
            hit.getWireHit().getAutomatonCell().setMaskedFlag(false);
          }
        });
      }

      /// Clear list of track at the end of event
      void clearVectors()
      {
        m_cdcTracks.clear();
      }

      /// Get list of CDCTracks
      std::list<CDCTrack>& getCDCTracks()
      {
        return m_cdcTracks;
      };

    private:
      std::list<CDCTrack> m_cdcTracks; /**< List containing CDCTrack objects. */

    };
  }
}

