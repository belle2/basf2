/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>
#include <vector>
#include <sstream>

#include <framework/logging/Logger.h>

namespace Belle2 {

  /** Path finder for generic ContainerType.
   *
   * Uses recursive collection process and returns vector of paths, which are vectors of NodeType*.
   *
   * Requirements for ContainerType:
   * - must have begin() and end() with iterator pointing to pointers of entries ( = ContainerType< NodeType*>)
   *
   * Requirements for NodeType:
   * - must have function: bool NodeType::getMetaInfo().isSeed()
   * - must have function: NeighbourContainerType& NodeType::getInnerNodes()
   * - must have function: bool NodeType::getOuterNodes().empty()
   * - other requirements depend on NodeCompatibilityCheckerType used.
   *
   * Requirements for NeighbourContainerType:
   * - must have function: bool NeighbourContainerType::empty()
   * - must have function: unsigned int (or comparable) NeighbourContainerType::size()
   * - must have access operator:  NeighbourContainerType: operator [] returning a NodeType*
   *
   * Requirements for NodeCompatibilityCheckerType:
   * - must have function bool areCompatible(NodeType* outerNode, NodeType* innerNode);
   */
  template<class ContainerType, class NodeType, class NeighbourContainerType, class NodeCompatibilityCheckerType>
  class PathCollectorRecursive {
  public:
    /// Using Path for vector of pointers to NodeTypes
    using Path = std::vector<NodeType*>;


    /** Main functionality of this class
     * Evaluates provided network and creates all allowed paths.
     * All found paths are filled into the provided vector 'paths'.
     * If storeSubsets is turned on, also the sub-paths are saved to vector 'paths'.
     * If a defined limit on the number of possible paths is exceeded, the search is aborted, and false is returned.
     */
    // cppcheck-suppress constParameter
    bool findPaths(ContainerType& aNetwork, std::vector<Path>& paths, unsigned int pathLimit, bool storeSubsets = false)
    {
      m_storeSubsets = storeSubsets;

      std::vector<Path> allNodePaths;
      for (NodeType* aNode : aNetwork) {
        if (aNode->getMetaInfo().isSeed() == false) {
          continue;
        }

        NeighbourContainerType& innerNeighbours = aNode->getInnerNodes();
        if (innerNeighbours.empty()) {
          continue;
        }
        if (aNode->getOuterNodes().empty()) {
          nTrees++;
        }

        // creating unique_ptr of a new path:
        Path newPath = Path{aNode};

        findPathsRecursive(allNodePaths, newPath, innerNeighbours);
        storeAcceptedPath(newPath, allNodePaths);

        if (allNodePaths.size() > pathLimit) {
          B2WARNING("Number of collected paths is too large: skipping the event and not processing it."
                    << LogVar("Number of node paths", allNodePaths.size()) << LogVar("Current limit of paths", pathLimit));
          return false;
        }
      }
      paths = allNodePaths;
      return true;
    }


    /// Prints information about all paths provided in a vector of paths
    static std::string printPaths(const std::vector<Path>& allPaths)
    {
      std::stringstream out;
      unsigned int longestPath = 0, longesPathIndex = 0, index = 0;
      out << "Print " << allPaths.size() << " paths:";
      for (Path const& aPath : allPaths) {
        if (longestPath < aPath.size()) {
          longestPath = aPath.size();
          longesPathIndex = index;
        }
        out << "\n" << "path " << index << ": length " << aPath.size() << ", entries:\n";
        for (auto* entry : aPath) {
          out << entry->getEntry() << "| ";
        }
        index++;
      }
      out << "\n" << "longest path was " << longesPathIndex << " with length of " << longestPath << "\n";

      return out.str();
    }


  protected:
    /// Copies path to create an identical one
    Path clone(const Path& aPath) const
    {
      Path newPath = Path();
      for (auto* entry : aPath) {
        newPath.push_back(entry);
      }
      return newPath;
    }


    /// Tests length requirement on a path before adding it to path vector
    void storeAcceptedPath(Path newPath, std::vector<Path>& allNodePaths) const
    {
      if (newPath.size() >= minPathLength) {
        allNodePaths.push_back(newPath);
      }
    }


    /// Recursive pathFinder: Collects all possible segment combinations to build paths.
    void findPathsRecursive(std::vector<Path >& allNodePaths, Path& currentPath, NeighbourContainerType& innerNeighbours)
    {
      nRecursiveCalls++;

      if (currentPath.size() > 30) {
        B2WARNING("findPathsRecursive reached a path length of over 30. Stopping Path here!");
        return;
      }

      // Test if there are viable neighbours to current node
      NeighbourContainerType viableNeighbours;
      for (size_t iNeighbour = 0; iNeighbour < innerNeighbours.size(); ++iNeighbour) {
        if (m_compatibilityChecker.areCompatible(currentPath.back(), innerNeighbours[iNeighbour])) {
          viableNeighbours.push_back(innerNeighbours[iNeighbour]);
        }
      }

      // If current path will continue, optionally store the subpath up to current node
      if (m_storeSubsets && viableNeighbours.size() > 0) {
        Path newPath = clone(currentPath);
        storeAcceptedPath(newPath, allNodePaths);
      }

      for (size_t iNeighbour = 0; iNeighbour < viableNeighbours.size(); ++iNeighbour) {
        // the last alternative is assigned to the existing path.
        if (iNeighbour == viableNeighbours.size() - 1) {
          currentPath.push_back(viableNeighbours[iNeighbour]);
          NeighbourContainerType& newNeighbours = viableNeighbours[iNeighbour]->getInnerNodes();

          findPathsRecursive(allNodePaths, currentPath, newNeighbours);
        } else {
          Path newPath = clone(currentPath);

          newPath.push_back(viableNeighbours[iNeighbour]);
          NeighbourContainerType& newNeighbours = viableNeighbours[iNeighbour]->getInnerNodes();

          findPathsRecursive(allNodePaths, newPath, newNeighbours);
          storeAcceptedPath(newPath, allNodePaths);
        }
      }
    }

  public:
    /// public Data members:
    /** parameter for setting minimal path length:
     * path length == number of nodes collected in a row from given network, this is not necessarily number of hits! */
    unsigned int minPathLength = 2;

    /// Counter for number of trees found
    unsigned int nTrees = 0;

    /// Counter for number of recursive calls
    unsigned int nRecursiveCalls = 0;

    /// flag if subsets should be stored or not
    bool m_storeSubsets = false;

  protected:
    /// protected Data members:
    /** Stores mini-Class for checking compatibility of two nodes passed. */
    NodeCompatibilityCheckerType m_compatibilityChecker;
  };
}
