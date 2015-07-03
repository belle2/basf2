/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingVXD/algorithms/PathCollectorRecursive.h>

// #include <sstream>
// using namespace std;
// using namespace Belle2;
//
// template<class ContainerType, class NodeType, class NeighbourContainerType, class NodeCompatibilityCheckerType>
// // std::string PathCollectorRecursive<ContainerType, NodeType, NeighbourContainerType, NodeCompatibilityCheckerType>::printPaths(vector<PathPtr>& allPaths) const
// std::string PathCollectorRecursive<ContainerType, NodeType, NeighbourContainerType, NodeCompatibilityCheckerType>::printPaths(vector<PathPtr>& allPaths)
// {
//   stringstream out;
//   unsigned int longestPath = 0, longesPathIndex = 0, index = 0;
//   out << "Print " << allPaths.size() << " paths:\n";
//   for(PathPtr& aPath : allPaths) {
//  if (longestPath < aPath->size()) { longestPath = aPath->size(); longesPathIndex = index; }
//  out << "path " << index << ": length " << aPath->size() << ", entries:\n";
//  for(auto* entry : *aPath) {
//    out << entry->getEntry() << "| ";
//  }
//  index++;
//   }
//   out << "\n" << "longest path was " << longesPathIndex << " with length of " << longestPath << "\n";
//
//   return out.str();
// }
