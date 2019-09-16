/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <array>
#include <iostream>
#include <gtest/gtest.h>

#include <framework/logging/Logger.h>

#include <tracking/trackFindingVXD/segmentNetwork/DirectedNode.h>
#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetwork.h>
#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetworkContainer.h>
#include <tracking/trackFindingVXD/segmentNetwork/TrackNode.h>
#include <tracking/trackFindingVXD/segmentNetwork/NodeNetworkHelperFunctions.h>
#include <tracking/trackFindingVXD/segmentNetwork/CACell.h>
#include <tracking/trackFindingVXD/algorithms/CellularAutomaton.h>
#include <tracking/trackFindingVXD/algorithms/CAValidator.h>
#include <tracking/trackFindingVXD/algorithms/PathCollectorRecursive.h>
#include <tracking/trackFindingVXD/algorithms/NodeCompatibilityCheckerPathCollector.h>


using namespace std;
using namespace Belle2;

/**
 * These tests cover the functionality of the classes:
 * DirectedNode, DirectedNodeNetwork. TODO
 */
namespace CellularAutomatonTests {


  /// Test class demonstrating the behavior of The Cellular Automaton and the PathCollectorRecursive
  class CellularAutomatonTest : public ::testing::Test {
  protected:

  };


  /** Test without external mockup. Fills a DirectedNodeNetwork< int, CACell> to be able to apply a CA,
  * find seeds and collect the Paths using PathCollectorRecursive:
  */
  TEST(CellularAutomatonTest, TestCAAndPathCollectorRecursiveUsingDirectedNodeNetworkInt)
  {
    // just some input for testing (same as in DirectedNodeNetwork-tests):
    std::array<int, 5> intArray  = { { 2, 5, 3, 4, 99} };
    std::array<int, 5> intArray2  = { { 144, 121, 33, 35, 31415} }; // these entries are independent of the first intArray-entries
    std::array<int, 5> intArray3  = { { 1440, 1210, 3, 33, 3141529} }; // entry 2 crosses intArray, entry 3 crosses intArray2
    std::vector<int> onTheFlyCreatedInts; // the user has to take care about the lifetime of the objects to be linked in the network!
    onTheFlyCreatedInts.reserve(4);


    DirectedNodeNetwork<int, CACell> intNetwork;
    EXPECT_EQ(0, intNetwork.size());

    // filling network:
    for (unsigned int index = 1 ; index < 5; index++) {
      // correct order: outerEntry, innerEntry:
      intNetwork.addNode(intArray.at(index - 1), intArray.at(index - 1));
      intNetwork.addNode(intArray.at(index), intArray.at(index));

      intNetwork.linkNodes(intArray.at(index - 1), intArray.at(index));
    }

    for (unsigned int index = 1 ; index < 5; index++) {
      intNetwork.addNode(intArray2.at(index - 1), intArray2.at(index - 1));
      intNetwork.addNode(intArray2.at(index), intArray2.at(index));

      intNetwork.linkNodes(intArray2.at(index - 1), intArray2.at(index));
    }

    for (unsigned int index = 1 ; index < 5; index++) {
      intNetwork.addNode(intArray3.at(index - 1), intArray3.at(index - 1));
      intNetwork.addNode(intArray3.at(index), intArray3.at(index));

      intNetwork.linkNodes(intArray3.at(index - 1), intArray3.at(index));
    }

    {
      int oldOuterInt = 2;
      onTheFlyCreatedInts.push_back(42);
      int& newInnerInt = onTheFlyCreatedInts.back();
      intNetwork.addNode(newInnerInt, newInnerInt);
      intNetwork.linkNodes(newInnerInt, oldOuterInt);
    }

    {
      onTheFlyCreatedInts.push_back(23);
      int& newOuterInt = onTheFlyCreatedInts.back();
      int& existingInt = intArray.at(1); // neither an outer nor an inner end before.
      intNetwork.addNode(newOuterInt, newOuterInt);
      intNetwork.linkNodes(newOuterInt, existingInt);
    }

    intNetwork.linkNodes(intArray.at(0), intArray.at(2));
    intNetwork.addInnerToLastOuterNode(intArray.at(3));

    {
      onTheFlyCreatedInts.push_back(31);
      int& newInnerInt = onTheFlyCreatedInts.back();
      intNetwork.addNode(newInnerInt, newInnerInt);
      intNetwork.addInnerToLastOuterNode(newInnerInt);
    }

    intNetwork.addOuterToLastInnerNode(intArray2.at(1));

    {
      onTheFlyCreatedInts.push_back(66);
      int& newOuterInt = onTheFlyCreatedInts.back();
      intNetwork.addNode(newOuterInt, newOuterInt);
      intNetwork.addOuterToLastInnerNode(newOuterInt);
    }
    // filling network - end.
    EXPECT_EQ(17, intNetwork.size());
    // uncomment following line if you want to stream a directed-graph-print of the network into "outputFile".gv
    //  DNN::printNetwork(intNetwork, "outputFile");


    /// CA:
    CellularAutomaton<DirectedNodeNetwork<int, CACell>, CAValidator<CACell>> cellularAutomaton;

    int nRounds = cellularAutomaton.apply(intNetwork);
    unsigned int nSeeds = cellularAutomaton.findSeeds(intNetwork);
    EXPECT_EQ(8,
              nRounds); // CA starts counting with 1, not with 0, the length of the paths is the number of Cells stored in it. the last round is an empty round.
    EXPECT_EQ(13, nSeeds);

    typedef PathCollectorRecursive <
    DirectedNodeNetwork<int, CACell>,
                        DirectedNode<int, CACell>,
                        std::vector<DirectedNode<int, CACell>*>,
                        NodeCompatibilityCheckerPathCollector<DirectedNode<int, CACell>> > PathCollectorType;


    /// PathCollector:
    PathCollectorType pathCollector;

    std::vector< PathCollectorType::Path> paths;
    pathCollector.findPaths(intNetwork, paths, 100000000);

    std::string out = PathCollectorType::printPaths(paths);
    B2INFO(out);

    // there could be more paths than seeds: why?
    // -> the pathCollectorRecursive based on the CA also adds alternative paths with the same length.
    EXPECT_EQ(13, paths.size());
    unsigned int longestPath = 0;
    for (auto& aPath : paths) {
      if (longestPath < aPath.size()) {
        longestPath = aPath.size();
      }
    }

    EXPECT_EQ(7, longestPath);  // TODO: fix
    EXPECT_EQ(nRounds, longestPath +
              1); // CA starts counting with 1, not with 0, the length of the paths is the number of Cells stored in it.

    // also collect subpaths
    paths.clear();
    bool test = pathCollector.findPaths(intNetwork, paths, 50, true);
    EXPECT_EQ(44, paths.size()); // Out of the 13 paths one gets 31 subpaths -> 44 in total
    EXPECT_EQ(true, test); // Should return true, as 44 paths do not exceed the given limit of 50

    // Checking if limit works
    paths.clear();
    test = pathCollector.findPaths(intNetwork, paths, 10);
    EXPECT_EQ(false, test); // Should return false, as 13 paths exceed the given limit of 10
  }
}
