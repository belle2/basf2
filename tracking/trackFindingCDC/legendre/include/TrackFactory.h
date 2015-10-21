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

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>
#include <tracking/trackFindingCDC/legendre/TrackFitter.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>
#include <tracking/trackFindingCDC/legendre/QuadTreeHitWrapper.h>

#include "TH1F.h"
#include "TH2F.h"

#include <list>
#include <vector>
#include <string>
#include <set>

using namespace std;

namespace Belle2 {

  class CDCHit;

  namespace TrackFindingCDC {

    class TrackDrawer;
    class TrackHit;
    class CDCTrack;
    class CDCTrajectory2D;

    class TrackFactory {
    public:

      /**
       * Please note that the implemented methods do only use the axial hits!
       * We use the fitter and the drawer as a pointer to have the possibility to use different classes.
       */
      TrackFactory(TrackFitter& trackFitter) : m_cdcTracks(), m_trackFitter(trackFitter) { };


      /**
       * Do not copy this class
       */
      TrackFactory(const TrackFactory& copy) = delete;

      /**
       * Do not copy this class
       */
      TrackFactory& operator=(const TrackFactory&) = delete;

      CDCTrack& createCDCTrackCandidate(std::vector<QuadTreeHitWrapper*>& qtHits);

      CDCTrack& createCDCTrackCandidate(std::vector<const CDCWireHit*>& wireHits);

      /** Created CDCTracks from the stored CDCLegendreTrackCandidates */
      void createCDCTracks(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) ;

      void deleteTrack(CDCTrack& track);

      CDCTrajectory2D fit(CDCTrack& track);

      /**
       * Reset all masked hits
       */
      void resetMaskedHits()
      {
        doForAllTracks([](CDCTrack & track) {
          for (CDCRecoHit3D& hit : track) {
            hit.getWireHit().getAutomatonCell().setTakenFlag(true);
            hit.getWireHit().getAutomatonCell().setMaskedFlag(false);
          }
        });
      }

      void doForAllTracks(std::function<void(CDCTrack& track)> function)
      {
        for (std::list<CDCTrack>::iterator it = m_cdcTracks.begin(); it !=  m_cdcTracks.end(); ++it) {
          function(*it);
        }
      }

    private:
      std::list<CDCTrack> m_cdcTracks;
      TrackFitter m_trackFitter;

    };
  }
}

