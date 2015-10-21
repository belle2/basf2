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


#include <tracking/trackFindingCDC/legendre/QuadTreeHitWrapper.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <vector>
#include <list>
#include <algorithm>

#include <cstdlib>
#include <iomanip>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cmath>
#include <functional>

using namespace std;

namespace Belle2 {

  class CDCHit;

  namespace TrackFindingCDC {

    class CDCTrack;
    class CDCWireHit;
    class QuadTreeHitWrapper;

    class HitFactory {
    public:

      /**
       * Please note that the implemented methods do only use the axial hits!
       * We use the fitter and the drawer as a pointer to have the possibility to use different classes.
       */
      HitFactory() : m_QuadTreeHitWrappers()  { } ;

      /**
       * Do not copy this class
       */
      HitFactory(const HitFactory& copy) = delete;

      /**
       * Do not copy this class
       */
      HitFactory& operator=(const HitFactory&) = delete;

      /**
       * Compile the hitList from the wire hit topology.
       */
      void initializeQuadTreeHitWrappers();

      void unmaskHitsInTrack(CDCTrack& track);

      std::vector<const CDCWireHit*> convertQTHitsToWireHits(std::vector<QuadTreeHitWrapper>& qtHitsToConvert);

      std::vector<QuadTreeHitWrapper> convertWireHitsToQTHits(std::vector<const CDCWireHit*>& wireHitsToConvert);

      /**
       * Get the list with currently stored tracks.
       */
      std::vector<QuadTreeHitWrapper>& getQuadTreeHitWrappers()
      {
        return m_QuadTreeHitWrappers;
      }

      /**
       * For the use in the QuadTree use this hit set.
       * @return the hit set with axial hits to use in the QuadTree-Finding.
       */
      std::vector<QuadTreeHitWrapper*> createQuadTreeHitWrappersForQT(bool useSegmentsOnly = false);


      /**
       * Reset all masked hits
       */
      void resetMaskedHits(std::list<CDCTrack>& cdcTracks)
      {
        doForAllHits([](QuadTreeHitWrapper & hit) {
          hit.setMaskedFlag(false);
          hit.setUsedFlag(false);
        });

        for (std::list<CDCTrack>::iterator it = cdcTracks.begin(); it !=  cdcTracks.end(); ++it) {
          for (const CDCRecoHit3D& hit : *it) {
            hit.getWireHit().getAutomatonCell().setTakenFlag(true);
          }
        }
      }

      /**
       * After each event the created hits and trackCandidates should be deleted.
       */
      void clearVectors()
      {
        m_QuadTreeHitWrappers.clear();
      }

      /**
       * Do a certain function for each track in the track list
       */
      void doForAllHits(std::function<void(QuadTreeHitWrapper& hit)> function)
      {
        for (QuadTreeHitWrapper& hit : m_QuadTreeHitWrappers) {
          function(hit);
        }
      }

    private:
      std::vector<QuadTreeHitWrapper> m_QuadTreeHitWrappers; /**< Vector which hold axial hits */


    };
  }
}
