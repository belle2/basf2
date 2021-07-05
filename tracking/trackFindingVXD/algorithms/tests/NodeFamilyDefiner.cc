/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <gtest/gtest.h>

#include <tracking/trackFindingVXD/segmentNetwork/DirectedNode.h>
#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetwork.h>

#include <tracking/trackFindingVXD/segmentNetwork/CACell.h>

#include <tracking/trackFindingVXD/algorithms/NodeCompatibilityCheckerPathCollector.h>
#include <tracking/trackFindingVXD/algorithms/NodeFamilyDefiner.h>

#include <array>
#include <iostream>


using namespace std;
using namespace Belle2;

/**
 * These tests cover the functionality of the classes:
 * DirectedNode, DirectedNodeNetwork. TODO
 */
namespace NodeFamilyTests {

  /// Unit test for NodeFamilyDefiner
  TEST(NodeFamilyDefinerTest, TestNodeFamiliesUsingDirectedNodeNetworkInt)
  {
    // just some input for testing (same as in DirectedNodeNetwork-tests):
    std::array<int, 5> intArray = { { 1, 5, 3, 4, 2} }; /**< A array of integers for test */
    std::array<int, 5> intArray2 = { { 0, 3, 4, 6, 7} }; /**< these entries are independent of the first intArray-entries */
    std::array<int, 5> intArray3 = { { 99, 101, 103, 104, 105} }; /**< entry 2 crosses intArray, entry 3 crosses intArray2 */
    std::array<int, 3> intArray4 = { { 100, 101, 102} }; /**< another array for tests */

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

    for (unsigned int index = 1 ; index < 3; index++) {
      intNetwork.addNode(intArray4.at(index - 1), intArray4.at(index - 1));
      intNetwork.addNode(intArray4.at(index), intArray4.at(index));

      intNetwork.linkNodes(intArray4.at(index - 1), intArray4.at(index));
    }

    // filling network - end.
    EXPECT_EQ(15, intNetwork.size());

    NodeFamilyDefiner <
    DirectedNodeNetwork<int, CACell>,
                        DirectedNode<int, CACell>,
                        std::vector<DirectedNode<int, CACell>* >> familyDefiner;

    short nFamilies = familyDefiner.defineFamilies(intNetwork);

    EXPECT_EQ(2, nFamilies);
  }
}
