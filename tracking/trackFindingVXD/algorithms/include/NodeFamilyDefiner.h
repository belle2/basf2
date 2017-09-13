/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/logging/Logger.h>

#include <string>
#include <vector>

namespace Belle2 {

  /* This class assigns a common family identifier to all CACells in the network that are connected.
   *
   * Requirements for ContainerType:
   * - must have begin() and end() with iterator pointing to pointers of entries ( = ContainerType< NodeType*>)
   *
   * Requirements for NodeType:
   * - must have function: bool NodeType::getMetaInfo().setFamily()
   * - must have function: bool NodeType::getMetaInfo().getFamily()
   * - must have function: NeighbourContainerType& NodeType::getInnerNodes()
   * - must have function: NeighbourContainerType& NodeType::getOuterNodes()
   *
   * Requirements for NeighbourContainerType:
   * - must have function: unsigned int (or comparable) NeighbourContainerType::size()
   * - must support range based for loop
   */
  template<class ContainerType, class NodeType, class NeighbourContainerType>
  class NodeFamilyDefiner {
  public:

    /* Assign a common family identifier to all Nodes in the network that are connected.
     * Performs a width first flood fill algorithm.
     * Returns total number of defined families.
     */
    short defineFamilies(ContainerType& aNetwork)
    {
      short currentFamily = 0;
      for (NodeType* aNode : aNetwork) {
        auto& metaInfo = aNode->getMetaInfo();
        if (metaInfo.getFamily() != -1) {
          B2DEBUG(1, "Node already assigned to a family:" << metaInfo.getFamily());
          continue;
        }

        B2DEBUG(1, "Happy little family: " << currentFamily);
        metaInfo.setFamily(currentFamily);


        NeighbourContainerType& innerNeighbours = aNode->getInnerNodes();
        NeighbourContainerType& outerNeighbours = aNode->getOuterNodes();
        NeighbourContainerType neighbours;
        neighbours.reserve(innerNeighbours.size() + outerNeighbours.size());
        neighbours.insert(neighbours.end(), innerNeighbours.begin(), innerNeighbours.end());
        neighbours.insert(neighbours.end(), outerNeighbours.begin(), outerNeighbours.end());

        while (neighbours.size() != 0) {
          neighbours = markNodes(currentFamily, neighbours);
        }
        currentFamily++;
      }
      return currentFamily;
    }

  private:
    /* Assign family to all connected nodes and return their neighbours.*/
    NeighbourContainerType markNodes(short family, NeighbourContainerType& neighbours)
    {
      NeighbourContainerType newNeighbours;
      B2DEBUG(1, "Mark node for family: " << family << " with " << neighbours.size() << " neighbour nodes.");
      for (auto& neighbour : neighbours) {
        auto& metaInfo = neighbour->getMetaInfo();
        // If node was already touched continue;
        if (metaInfo.getFamily() != -1) {
          short tmpFamily = metaInfo.getFamily();
          if (tmpFamily != family) B2FATAL("Node already assigned to different family: " << family << ", " << tmpFamily);
          else continue;
        }
        metaInfo.setFamily(family);
        NeighbourContainerType& innerNeighbours = neighbour->getInnerNodes();
        NeighbourContainerType& outerNeighbours = neighbour->getOuterNodes();
        newNeighbours.reserve(innerNeighbours.size() + outerNeighbours.size());
        newNeighbours.insert(newNeighbours.end(), innerNeighbours.begin(), innerNeighbours.end());
        newNeighbours.insert(newNeighbours.end(), outerNeighbours.begin(), outerNeighbours.end());
      }
      return newNeighbours;
    }
  };
}
