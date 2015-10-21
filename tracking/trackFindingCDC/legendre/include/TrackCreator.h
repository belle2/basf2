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
#include <tracking/trackFindingCDC/legendre/TrackFitter.h>

#include <vector>

using namespace std;

namespace Belle2 {

  class CDCHit;

  namespace TrackFindingCDC {

    class CDCTrack;
    class CDCWireHit;
    class QuadTreeHitWrapper;
    class TrackFitter;
    class CDCTrajectory2D;

    /**
     * Class which creates CDCTrack objects
     */
    class TrackCreator {
    public:

      ///Constructor
      TrackCreator() : m_trackFitter() { };

      /**
       * Create CDCTrack. Before calling this function empty CDCTrack object should be created and passes as argument
       * @param qtHits vector of QuadTreeHitWrapper objects used for CDCTrack creation
       * @param track reference to the empty CDCTrack object which should be filled with hits
       */
      void createCDCTrackCandidate(std::vector<QuadTreeHitWrapper*>& qtHits, CDCTrack& track);

      /**
       * Create CDCTrack. Before calling this function empty CDCTrack object should be created and passes as argument
       * @param qtHits vector of CDCWireHit objects used for CDCTrack creation
       * @param track reference to the empty CDCTrack object which should be filled with hits
       */
      void createCDCTrackCandidate(std::vector<const CDCWireHit*>& wireHits, CDCTrack& track);

      /**
       * Fill common vector of CDCTrack with fount tracks. Used for compability with common TrackFindingCDC interface.
       * @param tracksToStore common vector of CDCTracks which will be stored in DataStore
       * @param tracksFromFinder tracks found with LegendreFinder
       */
      void createCDCTracks(std::vector<CDCTrack>& tracksToStore, std::list<CDCTrack>& tracksFromFinder) ;

      /**
       * Fit CDCTrack
       * @return trajectory of the track
       */
      CDCTrajectory2D fit(CDCTrack& track);

    private:
      TrackFitter m_trackFitter; /**< Track fitter object. */

    };
  }
}

