/*
 * CDCLegendreQuadTreeNeighborFinder.h
 *
 *  Created on: Apr 15, 2014
 *      Author: vtrusov
 */

#pragma once

#include <tracking/cdcLegendreTracking/CDCLegendreQuadTree.h>

#include <framework/logging/Logger.h>
#include <algorithm>


namespace Belle2 {
  namespace TrackFinderCDCLegendre {

    class QuadTree;

    class QuadTreeNeighborFinder {
    public:
      QuadTreeNeighborFinder() {};
      ~QuadTreeNeighborFinder() {};

      /** Get static instance of the class */
      static QuadTreeNeighborFinder& Instance();

      /** Controls process of neighbors finding */
      void controller(QuadTree*, QuadTree*, QuadTree*);

      /** One level up in the tree */
      void levelUp(QuadTree*, QuadTree*);

      /** One level down in the tree */
      void levelDown(QuadTree*, QuadTree*);

      /** Trying to find neighbors to node */
      void findNeighbors(QuadTree*, QuadTree*, QuadTree*);

      /** Checks if nodes are neighbors */
      bool compareNodes(QuadTree*, QuadTree*);

    private:

      static QuadTreeNeighborFinder* s_cdcLegendreQuadTreeNeighborFinder; /**< Static instance of the class */


    };
  }
}
