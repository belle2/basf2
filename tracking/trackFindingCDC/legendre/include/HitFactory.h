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

    /**
     * @brief Class which holds and creates hit objects used during track finding procedure
     *
     * Class takes CDCWireHit objects from DataStore and convert them into QuadTreeHitWrapper objects.
     * Created QuadTreeHitWrapper objects stored in internal vector.
     * Also contains methods to:
     *   - create CDCRecoHit3D objects using QuadTreeHitWrapper or CDCWireHit
     *   - convert vector of QuadTreeHitWrapper to CDCWireHit and vice versa
     *   - prepare vector of QuadTreeHitWrapper for quadtree
     */
    class HitFactory {
    public:

      /**
       * Constructor
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

      /**
       * Convert vector of QuadTreeHitWrapper hits into vector of CDCWireHit
       */
      std::vector<const CDCWireHit*> convertQTHitsToWireHits(std::vector<QuadTreeHitWrapper>& qtHitsToConvert);

      /**
       * Convert vector of CDCWireHit hits into vector of QuadTreeHitWrapper
       */
      std::vector<QuadTreeHitWrapper> convertWireHitsToQTHits(std::vector<const CDCWireHit*>& wireHitsToConvert);

      /** Create CDCRecoHit3D */
      static const CDCRecoHit3D createRecoHit3D(CDCTrajectory2D& trackTrajectory2D, QuadTreeHitWrapper* hit);

      /** Create CDCRecoHit3D */
      static const CDCRecoHit3D createRecoHit3D(CDCTrajectory2D& trackTrajectory2D, const CDCWireHit* hit);

      /**
       * Get the list with currently stored tracks.
       */
      std::vector<QuadTreeHitWrapper>& getQuadTreeHitWrappers()
      {
        return m_QuadTreeHitWrappers;
      }

      /**
       * For the use in the QuadTree use this hit set.
       *
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
