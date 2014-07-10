/*
 * QuadTreeNeighborFinder.cpp
 *
 *  Created on: Apr 15, 2014
 *      Author: vtrusov
 */

#include "tracking/cdcLegendreTracking/CDCLegendreQuadTreeNeighborFinder.h"
#include <tracking/cdcLegendreTracking/CDCLegendreQuadTree.h>
#include <algorithm>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace TrackFinderCDCLegendre;

QuadTreeNeighborFinder* QuadTreeNeighborFinder::s_cdcLegendreQuadTreeNeighborFinder = 0;


QuadTreeNeighborFinder& QuadTreeNeighborFinder::Instance()
{
  if (!s_cdcLegendreQuadTreeNeighborFinder) s_cdcLegendreQuadTreeNeighborFinder = new QuadTreeNeighborFinder();
  return *s_cdcLegendreQuadTreeNeighborFinder;
}

void QuadTreeNeighborFinder::controller(QuadTree* origin_node, QuadTree* caller_node, QuadTree* node)
{
  /*  B2INFO("LOOKING for neighbors");
    B2INFO("LEVEL = " << node->getLevel());
    B2INFO("Node: " << node->getThetaMin() << "x" << node->getThetaMax() << "x" << node->getRMin() << "x" << node->getRMax());
    B2INFO("Caller node: " << caller_node->getThetaMin() << "x" << caller_node->getThetaMax() << "x" << caller_node->getRMin() << "x" << caller_node->getRMax());
    B2INFO("Origin node: " << origin_node->getThetaMin() << "x" << origin_node->getThetaMax() << "x" << origin_node->getRMin() << "x" << origin_node->getRMax());
  */
//  B2INFO("LEVEL= " << node->getLevel());
//  if(origin_node == node){
//    levelUp(origin_node, origin_node);
//    return;
//  }

  findNeighbors(origin_node, caller_node, node);

//  findNeighbors(origin_node, caller_node, node);

  if (((origin_node->getThetaMin() ==  0 || origin_node->getThetaMax() ==  8192) && origin_node->getNneighbors() < 5) || (origin_node->getNneighbors() < 8)) {
//    B2INFO("Number of neighbors: "<< origin_node->getNneighbors());
    if (node->getParent() != NULL)
      levelUp(origin_node, node);
  }

}

void QuadTreeNeighborFinder::levelUp(QuadTree* origin_node, QuadTree* caller_node)
{
  controller(origin_node, caller_node, caller_node->getParent());
}

void QuadTreeNeighborFinder::levelDown(QuadTree* origin_node, QuadTree* node)
{
  for (int t_index = 0; t_index < node->getThetaNbins(); ++t_index) {
    for (int r_index = 0; r_index < node->getRNbins(); ++r_index) {
      QuadTree* child_node = node->getChildren(t_index, r_index);

      if (child_node == origin_node) continue;

      if (child_node->isLeaf()) {
//        B2INFO("Child node: " << child_node->getThetaMin() << "x" << child_node->getThetaMax() << "x" << child_node->getRMin() << "x" << child_node->getRMax());
        if (compareNodes(child_node, origin_node))
          origin_node->addNeighbor(child_node);
      } else {
        levelDown(origin_node, child_node);
      }
    }
  }
//    controller(origin_node, caller_node, caller_node->getParent());
}



void QuadTreeNeighborFinder::findNeighbors(QuadTree* origin_node, QuadTree* caller_node, QuadTree* node)
{
  for (int t_index = 0; t_index < node->getThetaNbins(); ++t_index) {
    for (int r_index = 0; r_index < node->getRNbins(); ++r_index) {
      QuadTree* child_node = node->getChildren(t_index, r_index);

      if (child_node == origin_node) continue;

      if (child_node == caller_node) continue;

      if (child_node->isLeaf()) {
//        B2INFO("Child node: " << child_node->getThetaMin() << "x" << child_node->getThetaMax() << "x" << child_node->getRMin() << "x" << child_node->getRMax());
        if (compareNodes(child_node, origin_node))
          origin_node->addNeighbor(child_node);
      } else {
        findNeighbors(origin_node, caller_node, child_node);
      }
    }
  }
}

bool QuadTreeNeighborFinder::compareNodes(QuadTree* node1 , QuadTree* node2)
{
  /*check all possible combination borders; 8 combinations at total
   *  ________
   * |__|__|__|
   * |__|XX|__|
   * |__|__|__|
   * node1 - checked node;
   * node2 - node, for which we are trying to find neighbor
   */
  if (node1->getRMin() == node2->getRMax()) {
    if (node1->getThetaMax() == node2->getThetaMin())
      return true;
    else if (node1->getThetaMax() == node2->getThetaMax())
      return true;
    else if (node1->getThetaMin() == node2->getThetaMax())
      return true;
  } else if (node1->getRMax() == node2->getRMin()) {
    if (node1->getThetaMax() == node2->getThetaMin())
      return true;
    else if (node1->getThetaMax() == node2->getThetaMax())
      return true;
    else if (node1->getThetaMin() == node2->getThetaMax())
      return true;
  } else if (node1->getRMax() == node2->getRMax()) {
    if (node1->getThetaMax() == node2->getThetaMin())
      return true;
    else if (node1->getThetaMin() == node2->getThetaMax())
      return true;
  }
  return false;
}
