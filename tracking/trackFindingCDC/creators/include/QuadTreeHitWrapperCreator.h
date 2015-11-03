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
    class ConformalCDCWireHit;

    /**
     * @brief Class which holds and creates hit objects used during track finding procedure.
     *
     * Class takes CDCWireHit objects from DataStore and convert them into QuadTreeHitWrapper objects.
     * Created QuadTreeHitWrapper objects stored in internal vector and this class is a wrapper around this vector.
     * Also contains methods to:
     *   - create CDCRecoHit3D objects using QuadTreeHitWrapper or CDCWireHit
     *   - convert vector of QuadTreeHitWrapper to CDCWireHit and vice versa
     *   - prepare vector of QuadTreeHitWrapper for quadtree
     */
    class QuadTreeHitWrapperCreator {
    public:

      /// Empty constructor does nothing.
      QuadTreeHitWrapperCreator() { } ;

      /// Do not copy this class.
      QuadTreeHitWrapperCreator(const QuadTreeHitWrapperCreator& copy) = delete;

      /// Do not copy this class.
      QuadTreeHitWrapperCreator& operator=(const QuadTreeHitWrapperCreator&) = delete;

      /// Compile the hitList from the wire hit topology.
      void initializeQuadTreeHitWrappers();

      /// Create CDCRecoHit3D.
      static const CDCRecoHit3D createRecoHit3D(const CDCTrajectory2D& trackTrajectory2D, ConformalCDCWireHit* hit);

      /// Create CDCRecoHit3D.
      static const CDCRecoHit3D createRecoHit3D(const CDCTrajectory2D& trackTrajectory2D, const CDCWireHit* hit);

      /// Get the list with currently stored tracks.
      std::vector<ConformalCDCWireHit>& getQuadTreeHitWrappers()
      {
        return m_QuadTreeHitWrappers;
      }

      /**
       * For the use in the QuadTree use this hit set.
       *
       * @return the hit set with axial hits to use in the QuadTree-Finding.
       */
      std::vector<ConformalCDCWireHit*> createQuadTreeHitWrappersForQT(bool useSegmentsOnly = false);

      /// Reset all masked hits.
      void resetMaskedHits(std::list<CDCTrack>& cdcTracks)
      {
        doForAllHits([](ConformalCDCWireHit & hit) {
          hit.setMaskedFlag(false);
          hit.setUsedFlag(false);
        });

        for (CDCTrack& cdcTrack : cdcTracks) {
          for (const CDCRecoHit3D& hit : cdcTrack) {
            hit.getWireHit().getAutomatonCell().setTakenFlag(true);
          }
        }
      }

      /// After each event the created hits and trackCandidates should be deleted.
      void clear()
      {
        m_QuadTreeHitWrappers.clear();
      }

      /// Do a certain function for each track in the track list.
      void doForAllHits(std::function<void(ConformalCDCWireHit& hit)> function)
      {
        for (ConformalCDCWireHit& hit : m_QuadTreeHitWrappers) {
          function(hit);
        }
      }

    private:
      std::vector<ConformalCDCWireHit> m_QuadTreeHitWrappers; /**< Vector which hold axial hits. */
    };
  }
}
