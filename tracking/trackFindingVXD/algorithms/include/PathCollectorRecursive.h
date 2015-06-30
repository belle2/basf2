/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <string>
#include <vector>
#include <utility>      // std::move

namespace Belle2 {

  /** Path finder for generic ContainerType.
   *
   * Uses recursive collection process and returns vector of paths, which are vectors of NodeEntryType*.
   *
   * Requirements for ContainerType:
   * - must have begin() and end() with iterator pointing to pointers of entries ( = ContainerType< NodeType*>)
   *
   * Requirements for NodeType:
   * - musst have function: bool NodeType::getMetaInfo().isSeed()
   * - musst have function: NeighbourContainerType& NodeType::getInnerNodes()
   * - musst have function: bool NodeType::getOuterNodes().empty()
   * - musst have function: NodeEntryType& NodeType::getEntry();
   *
   * Requirements for NodeEntryType:
   * - none
   *
   * Requirements for NeighbourContainerType:
   * - musst have function: bool NeighbourContainerType::empty()
   * - musst have function: unsigned int (or comparable) NeighbourContainerType::size()
   * - musst have access operator:  NeighbourContainerType: operator [] returning a NodeType*
   */
  template<class ContainerType, class NodeType, class NodeEntryType, class NeighbourContainerType>
  class PathCollectorRecursive {
  public:
    /** typedef to clarify what is a path here*/
    typedef std::vector<NodeEntryType*> Path;



    /** main function does need a network fulfilling some stuff which is fulfilled by any DirectedNode*/
    std::vector<Path > findPaths(ContainerType& aNetwork)
    {
      std::vector<Path > allNodePaths;
      for (NodeType* aNode : aNetwork) {
        if (aNode->getMetaInfo().isSeed() == false) { B2DEBUG(100, "findPaths: current node is no seed!-> skipping"); continue; }

        NeighbourContainerType& innerNeighbours = aNode->getInnerNodes();
        if (innerNeighbours.empty()) { B2DEBUG(100, "findPaths: current node has no inner neighbours!-> skipping"); continue; }
        if (aNode->getOuterNodes().empty()) { nTrees++; }

        allNodePaths.push_back({ & (aNode->getEntry())});

        findPathsRecursive(allNodePaths, allNodePaths.back(), innerNeighbours);
      }
      return std::move(allNodePaths);
    }



    /** simple counter for number of trees found */
    unsigned int nTrees = 0;


    /** simple counter for number of recursive calls of triggered */
    unsigned int nRecursiveCalls = 0;


    /** simple counter for number of nodes passed during process (which is not the same as number of nodes of the network, since some nodes will be passed several times due to overlapping trees */
    unsigned int nNodesPassed = 0;

  protected:

    /** recursice pathFinder collecting all possible combinations there are. */
    void findPathsRecursive(std::vector<Path >& allNodePaths, Path& currentPath, NeighbourContainerType& innerNeighbours)
    {
      if (innerNeighbours.empty()) {
        B2DEBUG(100,  "findPathsRecursive: currentPath (of " << allNodePaths.size() <<
                " so far) has no inner neighbours!-> path complete, skipping")
        nNodesPassed++;
        return;
      } // path complete, end function

      // for loop only executed if (innerNeighbours.size() > 1)
      for (unsigned int n = 1; n < innerNeighbours.size(); ++n) {

        allNodePaths.push_back(currentPath);
        Path newPath = allNodePaths.back();

        newPath.push_back(&(innerNeighbours[n]->getEntry()));
        NeighbourContainerType& newNeighbours = innerNeighbours[n]->getInnerNodes();
        B2DEBUG(100,  "findPathsRecursive: currentPath (of " << allNodePaths.size() <<
                " so far) executing neighbour #" << n <<
                " of " << innerNeighbours.size() <<
                " neighbours. Found " << newNeighbours.size() <<
                " for it.")

        nNodesPassed++; nRecursiveCalls++;
        findPathsRecursive(allNodePaths, newPath, newNeighbours);               /// findPathsRecursive
      } // makes clones of current Path for each neighbour (excluding the first one) and adds it to the Paths if neighbour fits in the scheme...

      //separate step for the first neighbour in line (has to be done after the cloning parts)...
      B2DEBUG(100,  "findPathsRecursive: currentPath (of " << allNodePaths.size() <<
              " so far) executing neighbour #" << 0 <<
              " of " << innerNeighbours.size() <<
              " neighbours")
      currentPath.push_back(&(innerNeighbours[0]->getEntry()));
      NeighbourContainerType& newNeighbours = innerNeighbours[0]->getInnerNodes();

      nNodesPassed++; nRecursiveCalls++;
      findPathsRecursive(allNodePaths, currentPath, newNeighbours);               /// findPathsRecursive
    }


  };

} //Belle2 namespace
