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

    static CDCLegendreQuadTreeNeighborFinder& Instance();

    void controller(CDCLegendreQuadTree*, CDCLegendreQuadTree*, CDCLegendreQuadTree*);

    void levelUp(CDCLegendreQuadTree*, CDCLegendreQuadTree*);

    void levelDown(CDCLegendreQuadTree*, CDCLegendreQuadTree*);

    void findNeighbors(CDCLegendreQuadTree*, CDCLegendreQuadTree*, CDCLegendreQuadTree*);

    bool compareNodes(CDCLegendreQuadTree*, CDCLegendreQuadTree*);

  private:

    static CDCLegendreQuadTreeNeighborFinder* s_cdcLegendreQuadTreeNeighborFinder;


  };

}
