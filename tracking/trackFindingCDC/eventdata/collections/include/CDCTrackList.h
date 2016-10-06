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


namespace Belle2 {

  namespace TrackFindingCDC {

    /**
     * Object which holds the list of created CDCTracks.
     * It is more or less only a wrapper to a std::list with some additional functions for convenience.
     */
    class CDCTrackList {
    public:
      /// Empty constructor does nothing.
      CDCTrackList() { };

      /// Do not copy this class.
      CDCTrackList(const CDCTrackList& copy) = delete;

      /// Do not copy this class.
      CDCTrackList& operator=(const CDCTrackList&) = delete;

      /// Create empty CDCTrack object at the end of the list.
      CDCTrack& createEmptyTrack()
      {
        m_cdcTracks.emplace_back();
        return m_cdcTracks.back();
      };

      /// Perform the provided function to all tracks.
      void doForAllTracks(std::function<void(CDCTrack& track)> function)
      {
        for (CDCTrack& cdcTrack : m_cdcTracks) {
          function(cdcTrack);
        }
      }

      /// Clear the list of tracks at the end of the event.
      void clear()
      {
        m_cdcTracks.clear();
      }

      /// Get the list of CDCTracks.
      std::list<CDCTrack>& getCDCTracks()
      {
        return m_cdcTracks;
      };

    private:
      std::list<CDCTrack> m_cdcTracks; /**< List containing CDCTrack objects. */
    };
  }
}

