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
  namespace TrackFindingCDC {
    class ConformalCDCWireHit;

    /**
     * Class which creates hit objects used during track finding procedure.
     *
     * The class takes CDCWireHit objects from the DataStore (vie the CDCWireHitTopology)
     * and convert them into ConformalCDCWireHit objects.
     * It also contains a method to prepare a vector of ConformalCDCWireHit objects for the quadtree.
     */
    class ConformalCDCWireHitCreator {
    public:
      /**
       * Compile the hitList from the wire hit topology.
       * The method goes through all wire hits from the wire hit topology and creates a ConformalCDCWireHit for each.
       * This ConformalCDCWireHit is however only added to the list, if is fulfills some quality criteria.
       */
      static void copyHitsFromTopology(std::vector<ConformalCDCWireHit>& conformalCDCWireHitList);

      /**
       * For the use in the QuadTree use this hit set.
       * @return the hit set with axial hits to use in the QuadTree-Finding.
       */
      static std::vector<ConformalCDCWireHit*> createConformalCDCWireHitListForQT(
        std::vector<ConformalCDCWireHit>& conformalCDCWireHitList);

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
