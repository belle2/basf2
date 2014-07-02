/*
 * CDCLegendreQuadTreeNeighborFinder.h
 *
 *  Created on: Apr 15, 2014
 *      Author: vtrusov
 */

#pragma once

#include <algorithm>


namespace Belle2 {

  class CDCLegendreQuadTree;
  class CDCLegendreQuadTreeNeighborFinder {
  public:
    CDCLegendreQuadTreeNeighborFinder() {};
    ~CDCLegendreQuadTreeNeighborFinder() {};

    /**
     * Get static instance of the class
     */
    static CDCLegendreQuadTreeNeighborFinder& Instance();

    /**
     * Controls process of neighbors finding
     */
    void controller(CDCLegendreQuadTree*, CDCLegendreQuadTree*, CDCLegendreQuadTree*);

    /**
     * One level up in the tree
     */
    void levelUp(CDCLegendreQuadTree*, CDCLegendreQuadTree*);

    /**
     * One level down in the tree
     */
    void levelDown(CDCLegendreQuadTree*, CDCLegendreQuadTree*);

    /**
     * Trying to find neighbors to node
     */
    void findNeighbors(CDCLegendreQuadTree*, CDCLegendreQuadTree*, CDCLegendreQuadTree*);

    /**
     * Checks if nodes are neighbors
     */
    bool compareNodes(CDCLegendreQuadTree*, CDCLegendreQuadTree*);

  private:

    static CDCLegendreQuadTreeNeighborFinder* s_cdcLegendreQuadTreeNeighborFinder; /**< Static instance of the class */


  };

}
