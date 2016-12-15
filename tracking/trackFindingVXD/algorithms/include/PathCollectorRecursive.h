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

#include <framework/logging/Logger.h>

#include <string>
#include <vector>
#include <utility>      // std::move
#include <memory> // std::unique_ptr

#include <sstream>

namespace Belle2 {

  /** Path finder for generic ContainerType.
   *
   * Uses recursive collection process and returns vector of paths, which are vectors of NodeType*.
   *
   * Requirements for ContainerType:
   * - must have begin() and end() with iterator pointing to pointers of entries ( = ContainerType< NodeType*>)
   *
   * Requirements for NodeType:
   * - musst have function: bool NodeType::getMetaInfo().isSeed()
   * - musst have function: NeighbourContainerType& NodeType::getInnerNodes()
   * - musst have function: bool NodeType::getOuterNodes().empty()
   * - other requirements depend on NodeCompatibilityCheckerType used.
   *
   * Requirements for NeighbourContainerType:
   * - musst have function: bool NeighbourContainerType::empty()
   * - musst have function: unsigned int (or comparable) NeighbourContainerType::size()
   * - musst have access operator:  NeighbourContainerType: operator [] returning a NodeType*
   *
   * Requirements for NodeCompatibilityCheckerType:
   * - musst have function bool areCompatible(NodeType* outerNode, NodeType* innerNode);
   */
  template<class ContainerType, class NodeType, class NeighbourContainerType, class NodeCompatibilityCheckerType>
  class PathCollectorRecursive {
  public:


    /** typedef to clarify what is a path here*/
    typedef std::vector<NodeType*> Path;


    /** typedef to clarify what is a pathPtr here*/
    typedef std::unique_ptr<Path> PathPtr;



    /** main function does need a network fulfilling some stuff which is fulfilled by any DirectedNode*/
    std::vector<PathPtr> findPaths(ContainerType& aNetwork)
    {
      B2DEBUG(25, "findPaths: executing now a network of size " << aNetwork.size());
      std::vector<PathPtr> allNodePaths;
      for (NodeType* aNode : aNetwork) {
        if (aNode->getMetaInfo().isSeed() == false) { B2DEBUG(75, "findPaths: current node is no seed!-> skipping"); continue; } // was: B2DEBUG(100,

        NeighbourContainerType& innerNeighbours = aNode->getInnerNodes();
        if (innerNeighbours.empty()) { B2DEBUG(75, "findPaths: current node has no inner neighbours!-> skipping"); continue; }
        if (aNode->getOuterNodes().empty()) { B2DEBUG(75, "findPaths: no outerNodes -> is a Tree."); nTrees++; }

        // creating unique_ptr of a new path:
        PathPtr newPath(new Path{aNode});

        nRecursiveCalls++;
        findPathsRecursive(allNodePaths, newPath, innerNeighbours);
        storeAcceptedPath(std::move(newPath), allNodePaths);
      }
      B2DEBUG(25, "findPaths-end: nTrees: " << nTrees << ", nRecursiveCalls: " << nRecursiveCalls << ", nNodesPassed: " << nNodesPassed);
      return std::move(allNodePaths);
    }



    /**  for debugging purposes, tries to print all given paths */
    static std::string printPaths(std::vector<PathPtr>& allPaths)
    {
      std::stringstream out;
      unsigned int longestPath = 0, longesPathIndex = 0, index = 0;
      out << "Print " << allPaths.size() << " paths:";
      for (PathPtr& aPath : allPaths) {
        if (longestPath < aPath->size()) { longestPath = aPath->size(); longesPathIndex = index; }
        out << "\n" << "path " << index << ": length " << aPath->size() << ", entries:\n";
        for (auto* entry : *aPath) {
          out << entry->getEntry() << "| ";
        }
        index++;
      }
      out << "\n" << "longest path was " << longesPathIndex << " with length of " << longestPath << "\n";

      return out.str();
    }


/// public Data members:

    /** parameter for setting minimal path length: (path length == number of nodes collected in a row from given network, this is not necessarily number of hits! */
    unsigned int minPathLength = 2;


    /** simple counter for number of trees found */
    unsigned int nTrees = 0;


    /** simple counter for number of recursive calls of triggered */
    unsigned int nRecursiveCalls = 0;


    /** simple counter for number of nodes passed during process (which is not the same as number of nodes of the network, since some nodes will be passed several times due to overlapping trees */
    unsigned int nNodesPassed = 0;

  protected:

    /** copies all pointers of given path to create an identical one */
    PathPtr clone(PathPtr& aPath) const
    {
      PathPtr newPath(new Path());
      for (auto* entry : *aPath) {
        newPath->push_back(entry);
      }
      return std::move(newPath);
    }



    /** takes care of storing only paths which are long enough. */
    void storeAcceptedPath(PathPtr newPath, std::vector<PathPtr >& allNodePaths) const
    {
      B2DEBUG(150, "storeAcceptedPath was started");
      if (!(newPath->size() < minPathLength)) {
        B2DEBUG(100, "storeAcceptedPath: path with size " << newPath->size() << " was accepted and will now be stored next to " <<
                allNodePaths.size() << " other paths");
        allNodePaths.push_back(std::move(newPath));
      }
    }



    /** recursice pathFinder collecting all possible combinations there are.  WARNING does not take care of metaInfo (where e.g. CA-cell-state is stored) */
    void findPathsRecursive(std::vector<PathPtr >& allNodePaths, PathPtr& currentPath, NeighbourContainerType& innerNeighbours)
    {
      B2DEBUG(150, "findPathsRecursive was started");

      if (innerNeighbours.empty()) {
        B2DEBUG(75, "findPathsRecursive: currentPath with nEntries: " << currentPath->size() <<
                "(of " << allNodePaths.size() <<
                " paths so far) has no inner neighbours!-> path complete, skipping");
        nNodesPassed++;
        return;
      } // path complete, end function

      // for loop only executed if (innerNeighbours.size() > 1)
      for (unsigned int n = 1; n < innerNeighbours.size(); ++n) {
        if (m_compatibilityChecker.areCompatible(currentPath->back(), innerNeighbours[n]) == false) { B2DEBUG(100, "findPathsRecursive: nodes are not compatible"); continue; }

        B2DEBUG(125, "findPathsRecursive: currentPath was cloned for neighbour " << n << " of " << innerNeighbours.size());
        PathPtr newPath = clone(currentPath); // deep copy of existing path

        newPath->push_back(innerNeighbours[n]);
        NeighbourContainerType& newNeighbours = innerNeighbours[n]->getInnerNodes();
        B2DEBUG(75, "findPathsRecursive: currentPath with nEntries: " << newPath->size() <<
                "(of " << allNodePaths.size() <<
                " paths so far) executing neighbour #" << n <<
                " of " << innerNeighbours.size() <<
                " neighbours. Found " << newNeighbours.size() <<
                " for it.");

        nNodesPassed++; nRecursiveCalls++;
        findPathsRecursive(allNodePaths, newPath, newNeighbours);               /// findPathsRecursive
        B2DEBUG(150, "findPathsRecursive: newPath: " << newPath->size() << ", currentPath.size: " << currentPath->size());
        storeAcceptedPath(std::move(newPath), allNodePaths);
      } // makes clones of current Path for each neighbour (excluding the first one) and adds it to the Paths if neighbour fits in the scheme...

      B2DEBUG(150, "findPathsRecursive: currentPath.size: " << currentPath->size() << ", innerNeighbours.size: " <<
              innerNeighbours.size());
      if (m_compatibilityChecker.areCompatible(currentPath->back(), innerNeighbours[0]) == false) { return; }

      //separate step for the first neighbour in line (has to be done after the cloning parts)...
      B2DEBUG(75, "findPathsRecursive: currentPath with nEntries: " << currentPath->size() <<
              "(of " << allNodePaths.size() <<
              " paths so far) executing neighbour #" << 0 <<
              " of " << innerNeighbours.size() <<
              " neighbours");
      currentPath->push_back(innerNeighbours[0]);
      NeighbourContainerType& newNeighbours = innerNeighbours[0]->getInnerNodes();

      nNodesPassed++; nRecursiveCalls++;
      findPathsRecursive(allNodePaths, currentPath, newNeighbours);               /// findPathsRecursive
    }


/// protected Data members:

    /** Stores mini-Class for checking compatibility of two nodes passed. */
    NodeCompatibilityCheckerType m_compatibilityChecker;
  };

} //Belle2 namespace
