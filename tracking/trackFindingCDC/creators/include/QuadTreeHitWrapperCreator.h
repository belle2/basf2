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
#include <vector>
#include <list>

using namespace std;

namespace Belle2 {

  class CDCHit;

  namespace TrackFindingCDC {

    class CDCTrack;
    class CDCTrajectory2D;
    class CDCWireHit;
    class CDCRecoHit3D;
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
    class ConformalCDCWireHitCreator {
    public:
      /// Compile the hitList from the wire hit topology.
      static void initializeQuadTreeHitWrappers(std::vector<ConformalCDCWireHit>& conformalCDCWireHitList);

      /// Create CDCRecoHit3D.
      static const CDCRecoHit3D reconstructWireHit(const CDCTrajectory2D& trackTrajectory2D, ConformalCDCWireHit* hit);

      /// Create CDCRecoHit3D.
      static const CDCRecoHit3D reconstructWireHit(const CDCTrajectory2D& trackTrajectory2D, const CDCWireHit* hit);

      /**
       * For the use in the QuadTree use this hit set.
       *
       * @return the hit set with axial hits to use in the QuadTree-Finding.
       */
      static std::vector<ConformalCDCWireHit*> createConformalCDCWireHitListForQT(std::vector<ConformalCDCWireHit>&
          conformalCDCWireHitList,
          bool useSegmentsOnly = false);

      /// Reset all masked hits.
      static void resetMaskedHits(std::list<CDCTrack>& cdcTracks, std::vector<ConformalCDCWireHit>& conformalCDCWireHitList);

    private:
      /// Static only.
      ConformalCDCWireHitCreator() { } ;

      /// Do not copy this class.
      ConformalCDCWireHitCreator(const ConformalCDCWireHitCreator& copy) = delete;

      /// Do not copy this class.
      ConformalCDCWireHitCreator& operator=(const ConformalCDCWireHitCreator&) = delete;
    };
  }
}
