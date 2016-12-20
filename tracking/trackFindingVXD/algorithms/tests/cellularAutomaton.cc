/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>

#include <framework/logging/Logger.h>

#include <tracking/trackFindingVXD/segmentNetwork/DirectedNode.h>
#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetwork.h>
#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetworkContainer.h>

#include <tracking/trackFindingVXD/segmentNetwork/TrackNode.h>

#include <tracking/trackFindingVXD/segmentNetwork/NodeNetworkHelperFunctions.h>

#include <tracking/trackFindingVXD/segmentNetwork/CACell.h>
#include <tracking/trackFindingVXD/algorithms/CellularAutomaton.h>
#include <tracking/trackFindingVXD/algorithms/CALogger.h>
#include <tracking/trackFindingVXD/algorithms/CAValidator.h>
#include <tracking/trackFindingVXD/algorithms/PathCollectorRecursive.h>
#include <tracking/trackFindingVXD/algorithms/NodeCompatibilityCheckerPathCollector.h>



#include <array>
#include <iostream>


using namespace std;
using namespace Belle2;

/**
 *
 * These tests cover the functionality of the classes:
 * DirectedNode, DirectedNodeNetwork. TODO
 *
 */
namespace CellularAutomatonTests {


  /** Test class demonstrating the behavior of The Cellular Automaton and the PathCollectorRecursive
   * */
  class CellularAutomatonTest : public ::testing::Test {
  protected:

  };




  /** Test without external mockup. Fills a DirectedNodeNetwork< int, CACell> to be able to apply a CA, find seeds and collect the Paths using PathCollectorRecursive:
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
      intNetwork.linkTheseEntries(intArray.at(index - 1), intArray.at(index));
    }

    for (unsigned int index = 1 ; index < 5; index++) {
      intNetwork.linkTheseEntries(intArray2.at(index - 1), intArray2.at(index));
    }

    for (unsigned int index = 1 ; index < 5; index++) {
      intNetwork.linkTheseEntries(intArray3.at(index - 1), intArray3.at(index));
    }

    {
      int oldOuterInt = intNetwork.getOuterEnds().at(0)->getEntry();
      onTheFlyCreatedInts.push_back(42);
      int& newInnerInt = onTheFlyCreatedInts.back();;
      intNetwork.linkTheseEntries(newInnerInt, oldOuterInt);
    }

    {
      onTheFlyCreatedInts.push_back(23);
      int& newOuterInt = onTheFlyCreatedInts.back();
      int& existingInt = intArray.at(1); // neither an outer nor an inner end before.
      intNetwork.linkTheseEntries(newOuterInt, existingInt);
    }

    intNetwork.linkTheseEntries(intArray.at(0), intArray.at(2));
    intNetwork.addInnerToLastOuterNode(intArray.at(3));

    {
      onTheFlyCreatedInts.push_back(31);
      int& newInnerInt = onTheFlyCreatedInts.back();
      intNetwork.addInnerToLastOuterNode(newInnerInt);
    }

    intNetwork.addOuterToLastInnerNode(intArray2.at(1));

    {
      onTheFlyCreatedInts.push_back(66);
      int& newOuterInt = onTheFlyCreatedInts.back();
      intNetwork.addOuterToLastInnerNode(newOuterInt);
    }
    // filling network - end.
    EXPECT_EQ(17, intNetwork.size());
    // uncomment following line if you want to stream a directed-graph-print of the network into "outputFile".gv
    //  DNN::printNetwork(intNetwork, "outputFile");


    /// CA:
    CellularAutomaton<DirectedNodeNetwork<int, CACell>, CAValidator<CACell>, CALogger> cellularAutomaton;

    int nRounds = cellularAutomaton.apply(intNetwork);
    unsigned int nSeeds = cellularAutomaton.findSeeds(intNetwork);
    EXPECT_EQ(8,
              nRounds); // CA starts counting with 1, not with 0, the length of the paths is the number of Cells stored in it. the last round is an empty round.
    EXPECT_EQ(10, nSeeds);

    typedef PathCollectorRecursive <
    DirectedNodeNetwork<int, CACell>,
                        DirectedNode<int, CACell>,
                        std::vector<DirectedNode<int, CACell>*>,
                        NodeCompatibilityCheckerPathCollector<DirectedNode<int, CACell>> > PathCollectorType;


    /// PathCollector:
    PathCollectorType pathCollector;

    std::vector< PathCollectorType::PathPtr> paths = pathCollector.findPaths(intNetwork);

    std::string out = PathCollectorType::printPaths(paths);
    B2INFO(out);


    EXPECT_EQ(10,
              paths.size());  // there could be more paths than seeds: why? ->the pathCollectorRecursive based on the CA also adds alternative paths with the same length.
    unsigned int longestPath = 0;
    for (PathCollectorType::PathPtr& aPath : paths) {
      if (longestPath < aPath->size()) { longestPath = aPath->size(); }
    }

    EXPECT_EQ(7, longestPath);  // TODO: fix
    EXPECT_EQ(nRounds, longestPath +
              1); // CA starts counting with 1, not with 0, the length of the paths is the number of Cells stored in it.
  }

} // end namespace




