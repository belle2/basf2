/*
 * QuadTreeNeighborFinder.cpp
 *
 *  Created on: Apr 15, 2014
 *      Author: vtrusov
 */

#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTreeNeighborFinder.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

QuadTreeNeighborFinder* QuadTreeNeighborFinder::s_cdcLegendreQuadTreeNeighborFinder = 0;


QuadTreeNeighborFinder& QuadTreeNeighborFinder::Instance()
{
  if (!s_cdcLegendreQuadTreeNeighborFinder) s_cdcLegendreQuadTreeNeighborFinder = new QuadTreeNeighborFinder();
  return *s_cdcLegendreQuadTreeNeighborFinder;
}

void QuadTreeNeighborFinder::controller(QuadTreeLegendre* /*origin_node*/, QuadTreeLegendre* /*caller_node*/,
                                        QuadTreeLegendre* /*node*/)
{
  // commented because the QuadTree data structure has been stripped of the neighbourhood
  // members.
  // If you need the NeighborFinder functionality again, add the appropriate data members
  // to the QuadTree and uncomment this lines.
  return;

  /*  B2INFO("LOOKING for neighbors");
    B2INFO("LEVEL = " << node->getLevel());
    B2INFO("Node: " << node->getYMin() << "x" << node->getYMax() << "x" << node->getXMin() << "x" << node->getXMax());
    B2INFO("Caller node: " << caller_node->getYMin() << "x" << caller_node->getYMax() << "x" << caller_node->getXMin() << "x" << caller_node->getXMax());
    B2INFO("Origin node: " << origin_node->getYMin() << "x" << origin_node->getYMax() << "x" << origin_node->getXMin() << "x" << origin_node->getXMax());
  */
//  B2INFO("LEVEL= " << node->getLevel());
//  if(origin_node == node){
//    levelUp(origin_node, origin_node);
//    return;
//  }

  /*  findNeighbors(origin_node, caller_node, node);

  //  findNeighbors(origin_node, caller_node, node);

    if (((origin_node->getYMin() ==  0 || origin_node->getYMax() ==  8192) && origin_node->getNneighbors() < 5) || (origin_node->getNneighbors() < 8)) {
  //    B2INFO("Number of neighbors: "<< origin_node->getNneighbors());
      if (node->getParent() != NULL)
        levelUp(origin_node, node);
    }
  */
}

void QuadTreeNeighborFinder::levelUp(QuadTreeLegendre* /*origin_node*/, QuadTreeLegendre* /*caller_node*/)
{
// controller(origin_node, caller_node, caller_node->getParent());
}

void QuadTreeNeighborFinder::levelDown(QuadTreeLegendre* /*origin_node*/, QuadTreeLegendre* /*node*/)
{
  /* for (int t_index = 0; t_index < node->getYNbins(); ++t_index) {
     for (int r_index = 0; r_index < node->getXNbins(); ++r_index) {
       QuadTreeLegendre* child_node = node->getChildren()->get(t_index, r_index);

       if (child_node == origin_node) continue;

       if (child_node->isLeaf()) {
  //        B2INFO("Child node: " << child_node->getYMin() << "x" << child_node->getYMax() << "x" << child_node->getXMin() << "x" << child_node->getXMax());
         if (compareNodes(child_node, origin_node))
           origin_node->addNeighbor(child_node);
       } else {
         levelDown(origin_node, child_node);
       }
     }
   }*/
//    controller(origin_node, caller_node, caller_node->getParent());
}



void QuadTreeNeighborFinder::findNeighbors(QuadTreeLegendre* /*origin_node*/, QuadTreeLegendre* /*caller_node*/,
                                           QuadTreeLegendre* /*node*/)
{
  /* for (int t_index = 0; t_index < node->getYNbins(); ++t_index) {
     for (int r_index = 0; r_index < node->getXNbins(); ++r_index) {
       QuadTreeLegendre* child_node = node->getChildren()->get(t_index, r_index);

       if (child_node == origin_node) continue;

       if (child_node == caller_node) continue;

       if (child_node->isLeaf()) {
  //        B2INFO("Child node: " << child_node->getYMin() << "x" << child_node->getYMax() << "x" << child_node->getXMin() << "x" << child_node->getXMax());
         if (compareNodes(child_node, origin_node))
           origin_node->addNeighbor(child_node);
       } else {
         findNeighbors(origin_node, caller_node, child_node);
       }
     }
   }*/
}

bool QuadTreeNeighborFinder::compareNodes(QuadTreeLegendre* node1 , QuadTreeLegendre* node2)
{
  /*check all possible combination borders; 8 combinations at total
   *  ________
   * |__|__|__|
   * |__|XX|__|
   * |__|__|__|
   * node1 - checked node;
   * node2 - node, for which we are trying to find neighbor
   */
  if (node1->getXMin() == node2->getXMax()) {
    if (node1->getYMax() == node2->getYMin())
      return true;
    else if (node1->getYMax() == node2->getYMax())
      return true;
    else if (node1->getYMin() == node2->getYMax())
      return true;
  } else if (node1->getXMax() == node2->getXMin()) {
    if (node1->getYMax() == node2->getYMin())
      return true;
    else if (node1->getYMax() == node2->getYMax())
      return true;
    else if (node1->getYMin() == node2->getYMax())
      return true;
  } else if (node1->getXMax() == node2->getXMax()) {
    if (node1->getYMax() == node2->getYMin())
      return true;
    else if (node1->getYMin() == node2->getYMax())
      return true;
  }
  return false;
}
