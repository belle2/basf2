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

#include <tracking/trackFindingVXD/segmentNetwork/DirectedNode.h>
#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetwork.h>
#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetworkContainer.h>

#include <tracking/trackFindingVXD/environment/VXDTFFilters.h>
#include <tracking/trackFindingVXD/segmentNetwork/TrackNode.h>

#include <vxd/geometry/SensorInfoBase.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationsObject.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>


#include <array>
#include <iostream>
#include <deque>

using namespace std;
using namespace Belle2;

/**
 *
 * These tests cover the functionality of the classes:
 * DirectedNode, DirectedNodeNetwork.
 *
 */
namespace DirectedNodeNetworkTests {


  /** helper function: create a sensorInfo to be used */
  VXD::SensorInfoBase provideSensorInfo(VxdID aVxdID, double globalX = 0., double globalY = 0., double globalZ = -0.)
  {
    // (SensorType type, VxdID id, double width, double length, double thickness, int uCells, int vCells, double width2=-1, double splitLength=-1, int vCells2=0)
    VXD::SensorInfoBase sensorInfoBase(VXD::SensorInfoBase::PXD, aVxdID, 2.3, 4.2, 0.3, 2, 4, -1);

    TGeoRotation r1;
    r1.SetAngles(45, 20, 30);      // rotation defined by Euler angles
    TGeoTranslation t1(globalX, globalY, globalZ);
    TGeoCombiTrans c1(t1, r1);
    TGeoHMatrix transform = c1;
    sensorInfoBase.setTransformation(transform);

    return sensorInfoBase;
  }


  /** helper function: returns a pxdCluster with given sensorID and local coordinates */
  PXDCluster providePXDCluster(double u, double v, VxdID aVxdID, double uError = 0.1, double vError = 0.1)
  {
    return PXDCluster(aVxdID, u, v, uError, vError, 0, 0, 1, 1, 1, 1 , 1, 1);
  }

  /** Test class demonstrating the behavior of TCNetworkContainer.
   *
   * used for preparing mockup.
   * */
  class DirectedNodeNetworkTest : public ::testing::Test {
  protected:

    /** SetUp environment - prepare related storearrays of SpacePoints and PXDClusters */
    virtual void SetUp()
    {
      DataStore::Instance().setInitializeActive(true);

      m_spacePointData.registerInDataStore();
      m_pxdClusterData.registerInDataStore();
      m_spacePointTrackCandData.registerInDataStore();

      m_spacePointData.registerRelationTo(m_pxdClusterData);

      m_networkContainerInDataStore.registerInDataStore();
      StoreObjPtr<DirectedNodeNetworkContainer> networkContainerInDataStore;

      DataStore::Instance().setInitializeActive(false);

      unsigned int nHits = 5;

      /// prepare pxdClusters and spacePoints related to them
      for (unsigned int i = 1; i <= nHits; ++i) { //

        VxdID aVxdID = VxdID(i, i, i);

        VXD::SensorInfoBase aSensorInfo = provideSensorInfo(aVxdID, (unsigned short)i, (unsigned short)i + 1., (unsigned short)i + 2.);

        const PXDCluster* pxdCluster = m_pxdClusterData.appendNew(providePXDCluster(float(i) / float(nHits), float(i) / float(nHits),
                                                                  aVxdID));

        SpacePoint* newSP = m_spacePointData.appendNew(pxdCluster, &aSensorInfo);
        B2DEBUG(10, " setup: new spacePoint got arrayIndex: " << newSP->getArrayIndex() << " and VxdID " << newSP->getVxdID());
        newSP->addRelationTo(pxdCluster);
      }

      B2DEBUG(10, "DirectedNodeNetworkTest:SetUP: created " << m_pxdClusterData.getEntries() << "/" << m_spacePointData.getEntries() <<
              " pxdClusters/SpacePoints");

      /// prepare some SpacePointTrackCands partially overlapping:
      vector<SpacePoint*> allSpacePoints;
      for (SpacePoint& aSP : m_spacePointData) {
        allSpacePoints.push_back(&aSP);
      }

      vector<const SpacePoint*> sps4TC1 = { allSpacePoints.at(0), allSpacePoints.at(1)};
      SpacePointTrackCand* aSPTC1 = m_spacePointTrackCandData.appendNew((sps4TC1)); // shares hits with no one
      aSPTC1->setQualityIndicator(0.92);

      vector<const SpacePoint*> sps4TC2 = { allSpacePoints.at(2), allSpacePoints.at(3)};
      SpacePointTrackCand* aSPTC2 = m_spacePointTrackCandData.appendNew((sps4TC2)); // shares a hit with tc3, tc4, tc5
      aSPTC2->setQualityIndicator(0.9);

      vector<const SpacePoint*> sps4TC3 = { allSpacePoints.at(3), allSpacePoints.at(4)};
      SpacePointTrackCand* aSPTC3 = m_spacePointTrackCandData.appendNew((sps4TC3)); // shares a hit with tc2, tc5
      aSPTC3->setQualityIndicator(0.8);

      vector<const SpacePoint*> sps4TC4 = { allSpacePoints.at(4)};
      SpacePointTrackCand* aSPTC4 = m_spacePointTrackCandData.appendNew((sps4TC4)); // shares a hit with tc3 too, but not with tc2
      aSPTC4->setQualityIndicator(0.7);

      vector<const SpacePoint*> sps4TC5 = { allSpacePoints.at(2), allSpacePoints.at(4)};
      SpacePointTrackCand* aSPTC5 = m_spacePointTrackCandData.appendNew((sps4TC5)); // shares a hit with tc2 and tc3
      aSPTC5->setQualityIndicator(0.65);
      // false positive due to new with placement (cppcheck issue #7163)
      // cppcheck-suppress memleak
    }

    /** TearDown environment - clear datastore */
    virtual void TearDown() { DataStore::Instance().reset(); }

    StoreArray<SpacePoint> m_spacePointData; /**< some spacePoints for testing. */
    StoreArray<PXDCluster> m_pxdClusterData; /**< some pxd clusters for testing. */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCandData; /**< some spacePointTrackCands for testing. */
    StoreArray<DirectedNodeNetworkContainer>
    m_networkContainerInDataStore; /**< testing to store a dummy class containing DirectedNodeNetwork as member . */
  };




  /** basic sanity checks: checks mockup for working properly */
  TEST_F(DirectedNodeNetworkTest, MockupSanityCheck)
  {
    EXPECT_EQ(5, m_pxdClusterData.getEntries());
    EXPECT_EQ(5, m_spacePointData.getEntries());
    EXPECT_EQ(m_pxdClusterData.getEntries(), m_spacePointData.getEntries());
    EXPECT_EQ(5, m_spacePointTrackCandData.getEntries());
  }



  /** testing basic functionality of the DirectedNodeNetwork when filled with a basic type - int.
   * This test is a bit overcrowded and therefore not an ideal test for learning the intended usage of the network.
   * This test tries to cover all cases relevant.
   * The correct usage of the network is better described in the test 'CreateRealisticNetwork'.
   */
  TEST_F(DirectedNodeNetworkTest, CreateNetworkIntAndThoroughlyTestIntendedBehavior)
  {
    // here some tool-definitions first:

    /** small lambda function for checking existence of a given node in a given vector */
    auto nodeWasFound = [&](std::vector<DirectedNode<int, VoidMetaInfo>*>& nodes, DirectedNode<int, VoidMetaInfo>* node) -> bool {
      for (DirectedNode<int, VoidMetaInfo>* otherNode : nodes)
      {
        if (node->getEntry() == otherNode->getEntry()) { return true; }
      }
      return false;
    };

    /** small lambda function for printing all the nodes for debugging */
    auto printNodeEntries = [&](std::vector<DirectedNode<int, VoidMetaInfo>*>& nodes) -> std::string {
      std::string output = "Nodes got the following entries: ";
      for (DirectedNode<int, VoidMetaInfo>* node : nodes)
      {
        output += std::to_string(node->getEntry()) + " ";
      }
      return output + "\n";
    };

    // just some input for testing:
    std::array<int, 5> intArray  = { { 2, 5, 3, 4, 99} };
    std::array<int, 5> intArray2  = { { 144, 121, 33, 35, 31415} }; // these entries are independent of the first intArray-entries
    std::array<int, 5> intArray3  = { { 1440, 1210, 3, 33, 3141529} }; // entry 2 crosses intArray, entry 3 crosses intArray2
    std::deque<int> onTheFlyCreatedInts; // the user has to take care about the lifetime of the objects to be linked in the network!
    EXPECT_EQ(5, intArray.size());

    DirectedNodeNetwork<int, VoidMetaInfo> intNetwork;
    EXPECT_EQ(0, intNetwork.size());
    B2INFO("tests case when both nodes are new and when inner one is new, but outer one not");
    for (unsigned int index = 1 ; index < 5; index++) {
      B2INFO("intArray-index " << index << " of array has entry: " << intArray.at(index));

      intNetwork.addNode(intArray.at(index - 1), intArray.at(index - 1));
      intNetwork.addNode(intArray.at(index), intArray.at(index));
      // correct order: outerEntry, innerEntry:
      intNetwork.linkNodes(intArray.at(index - 1), intArray.at(index));

      // innerEnd has been updated:
      EXPECT_EQ(intArray.at(index), intNetwork.getInnerEnds().at(0)->getEntry());

      // entries are now in network:
      EXPECT_EQ(intArray.at(index - 1), *intNetwork.getNode(intArray.at(index - 1)));
      EXPECT_EQ(intArray.at(index), *intNetwork.getNode(intArray.at(index)));

      // get all nodes of outer node, expected: 1 inner and no outerNodes:
      auto& innerNodes = intNetwork.getNode(intArray.at(index - 1))->getInnerNodes();
      auto& outerNodes = intNetwork.getNode(intArray.at(index))->getOuterNodes();
      EXPECT_EQ(1, innerNodes.size());
      EXPECT_EQ(1, outerNodes.size());

      // array[index] is now linked as inner node of array[index-1]:
      EXPECT_EQ(*(innerNodes.at(0)), *(intNetwork.getNode(intArray.at(index))));
    }
    EXPECT_EQ(0, intNetwork.getNode(intArray.at(0))->getOuterNodes().size()); // the outermost node has no outerNodes
    EXPECT_EQ(0, intNetwork.getNode(intArray.at(4))->getInnerNodes().size()); // the innermost node has no innerNodes

    // some extra sanity checks, are inner- and outerEnds as expected?
    EXPECT_EQ(intArray.size(), intNetwork.size());
    std::vector<DirectedNode<int, VoidMetaInfo>*> outerEnds_before = intNetwork.getOuterEnds();
    std::vector<DirectedNode<int, VoidMetaInfo>*> innerEnds_before = intNetwork.getInnerEnds();
    EXPECT_EQ(1, outerEnds_before.size());
    EXPECT_EQ(1, innerEnds_before.size());

    DirectedNode<int, VoidMetaInfo>* outermostNode = outerEnds_before.at(0);
    EXPECT_EQ(intArray.at(0), outermostNode->getEntry());
    EXPECT_EQ(outermostNode->getEntry(), *outermostNode);
    EXPECT_EQ(intArray.at(1), *(outermostNode->getInnerNodes().at(0)));

    DirectedNode<int, VoidMetaInfo>* innermostNode = innerEnds_before.at(0);
    EXPECT_EQ(intArray.at(4), innermostNode->getEntry());
    EXPECT_EQ(innermostNode->getEntry(), *innermostNode);
    EXPECT_EQ(intArray.at(3), *(innermostNode->getOuterNodes().at(0)));


    {
      B2INFO("building another path, which is independent of the first one:");
      for (unsigned int index = 1 ; index < 5; index++) {
        B2INFO("intArray2-index " << index << " of array has entry: " << intArray2.at(index));

        intNetwork.addNode(intArray2.at(index - 1), intArray2.at(index - 1));
        intNetwork.addNode(intArray2.at(index), intArray2.at(index));
        // correct order: outerEntry, innerEntry:
        intNetwork.linkNodes(intArray2.at(index - 1), intArray2.at(index));

        // entries are now in network:
        EXPECT_EQ(intArray2.at(index - 1), *intNetwork.getNode(intArray2.at(index - 1)));
        EXPECT_EQ(intArray2.at(index), *intNetwork.getNode(intArray2.at(index)));

        // get all nodes of outer node, expected: 1 inner and no outerNodes:
        auto& innerNodes = intNetwork.getNode(intArray2.at(index - 1))->getInnerNodes();
        auto& outerNodes = intNetwork.getNode(intArray2.at(index))->getOuterNodes();
        EXPECT_EQ(1, innerNodes.size());
        EXPECT_EQ(1, outerNodes.size());

        // array[index] is now linked as inner node of array[index-1]:
        EXPECT_TRUE(nodeWasFound(innerNodes, intNetwork.getNode(intArray2.at(index))));
        EXPECT_EQ(*(innerNodes.at(0)), *(intNetwork.getNode(intArray2.at(index))));
      }
      EXPECT_EQ(0, intNetwork.getNode(intArray2.at(0))->getOuterNodes().size()); // the outermost node has no outerNodes
      EXPECT_EQ(0, intNetwork.getNode(intArray2.at(4))->getInnerNodes().size()); // the innermost node has no innerNodes

      // some extra sanity checks, are inner- and outerEnds as expected?
      EXPECT_EQ(10, intNetwork.size());
      std::vector<DirectedNode<int, VoidMetaInfo>*> outerEnds = intNetwork.getOuterEnds();
      std::vector<DirectedNode<int, VoidMetaInfo>*> innerEnds = intNetwork.getInnerEnds();
      EXPECT_EQ(2, outerEnds.size());
      EXPECT_EQ(2, innerEnds.size());
    }


    B2INFO("building another path into the network which crosses the other paths:"); {
      B2INFO("tests cases: both new, outer new but inner not, inner new but outer not, both already existing:");
      for (unsigned int index = 1 ; index < 5; index++) {
        B2INFO("intArray3-indices " << index - 1 << "/" << index <<
               " of array have entries: " << intArray3.at(index - 1) << "/" << intArray3.at(index) << "\n"
              );

        intNetwork.addNode(intArray3.at(index - 1), intArray3.at(index - 1));
        intNetwork.addNode(intArray3.at(index), intArray3.at(index));
        // correct order: outerEntry, innerEntry:
        intNetwork.linkNodes(intArray3.at(index - 1), intArray3.at(index));

        // entries are now in network:
        EXPECT_EQ(intArray3.at(index - 1), *intNetwork.getNode(intArray3.at(index - 1)));
        EXPECT_EQ(intArray3.at(index), *intNetwork.getNode(intArray3.at(index)));

        // array[index] is now linked as an inner node of array[index-1]:
        EXPECT_TRUE(nodeWasFound(intNetwork.getNode(intArray3.at(index - 1))->getInnerNodes(),
                                 intNetwork.getNode(intArray3.at(index))));


        if (index > 1) continue; // the following tests do not work for paths crossing each other, therefore tests would fail
        // innerEnd has been updated:
        std::vector<DirectedNode<int, VoidMetaInfo>*> innerEnds = intNetwork.getInnerEnds();
        EXPECT_TRUE(nodeWasFound(innerEnds, intNetwork.getNode(intArray3.at(index))));
        B2INFO("innerEnds after indices " << index - 1 << "/" << index << " are: " << printNodeEntries(innerEnds));

        // get all nodes of outer node, expected: 1 inner and no outerNodes:
        auto& innerNodes = intNetwork.getNode(intArray3.at(index - 1))->getInnerNodes();
        auto& outerNodes = intNetwork.getNode(intArray3.at(index))->getOuterNodes();
        EXPECT_EQ(1, innerNodes.size());
        EXPECT_EQ(1, outerNodes.size());
      }
      EXPECT_EQ(0, intNetwork.getNode(intArray3.at(0))->getOuterNodes().size()); // the outermost node has no outerNodes
      EXPECT_EQ(0, intNetwork.getNode(intArray3.at(4))->getInnerNodes().size()); // the innermost node has no innerNodes

      // some extra sanity checks, are inner- and outerEnds as expected?
      EXPECT_EQ(13, intNetwork.size());
      //B2INFO(" theWholeNetwork: \n" << printNodeEntries(intNetwork.getNodes()));
      std::vector<DirectedNode<int, VoidMetaInfo>*> outerEnds = intNetwork.getOuterEnds();
      std::vector<DirectedNode<int, VoidMetaInfo>*> innerEnds = intNetwork.getInnerEnds();
      EXPECT_EQ(3, outerEnds.size());
      EXPECT_EQ(3, innerEnds.size());
      B2INFO("outerEnds are: \n" << printNodeEntries(outerEnds));
      B2INFO("innerEnds are: \n" << printNodeEntries(innerEnds));
    }

    B2INFO("case: when outer node is new, but inner one not: "); {
      B2INFO("  case: inner one was outer end before: ");
      auto* oldOuterMostNode = intNetwork.getOuterEnds().at(0);
      int oldOuterInt = oldOuterMostNode->getEntry();
      onTheFlyCreatedInts.push_back(42);
      int& newInnerInt = onTheFlyCreatedInts.back();
      EXPECT_TRUE(NULL == intNetwork.getNode(newInnerInt));
      intNetwork.addNode(newInnerInt, newInnerInt);
      intNetwork.linkNodes(newInnerInt, oldOuterInt);
      EXPECT_FALSE(NULL == intNetwork.getNode(newInnerInt));
      EXPECT_EQ(14, intNetwork.size());
      EXPECT_EQ(3, intNetwork.getInnerEnds().size());
      std::vector<DirectedNode<int, VoidMetaInfo>*> newOuterEnds = intNetwork.getOuterEnds();
      EXPECT_EQ(3, newOuterEnds.size());  // did not change, new outermost node replaced old one
      EXPECT_EQ(1, oldOuterMostNode->getOuterNodes().size());
      EXPECT_EQ(newInnerInt, *(oldOuterMostNode->getOuterNodes().at(0)));
    }

    {
      B2INFO("  case: inner one was no outer end before: ");
      onTheFlyCreatedInts.push_back(23);
      int& newOuterInt = onTheFlyCreatedInts.back();
      int& existingInt = intArray.at(1); // neither an outer nor an inner end before.
      std::vector<DirectedNode<int, VoidMetaInfo>*> oldOuterEnds = intNetwork.getOuterEnds();
      std::vector<DirectedNode<int, VoidMetaInfo>*> oldInnerEnds = intNetwork.getInnerEnds();

      EXPECT_EQ(3, oldOuterEnds.size());
      EXPECT_TRUE(NULL == intNetwork.getNode(newOuterInt));
      unsigned int sizeB4 = intNetwork.size();
      intNetwork.addNode(newOuterInt, newOuterInt);
      intNetwork.linkNodes(newOuterInt, existingInt);
      EXPECT_FALSE(NULL == intNetwork.getNode(newOuterInt));
      EXPECT_EQ(sizeB4 + 1, intNetwork.size());
      EXPECT_EQ(3, intNetwork.getInnerEnds().size());
      DirectedNode<int, VoidMetaInfo>* existingNode = intNetwork.getNode(existingInt);
      EXPECT_EQ(1, existingNode->getInnerNodes().size());
      EXPECT_EQ(2, existingNode->getOuterNodes().size());
      std::vector<DirectedNode<int, VoidMetaInfo>*> newOuterEnds = intNetwork.getOuterEnds();
      EXPECT_EQ(4, newOuterEnds.size());  // did change, new outermost node co-existing with old outermost one
      EXPECT_EQ(2, existingNode->getOuterNodes().size());

    }


    {
      B2INFO("case: when both were there, but not linked yet: ");
      unsigned int sizeB4 = intNetwork.size();
      intNetwork.linkNodes(intArray.at(0), intArray.at(2));
      EXPECT_EQ(sizeB4, intNetwork.size()); // size of network does not change
      EXPECT_EQ(3, intNetwork.getInnerEnds().size()); // this can not change here
      EXPECT_EQ(4, intNetwork.getOuterEnds().size());
      std::vector<DirectedNode<int, VoidMetaInfo>*> innerEnds = intNetwork.getNode(intArray.at(0))->getInnerNodes();
      EXPECT_EQ(2, innerEnds.size());
      EXPECT_TRUE(nodeWasFound(innerEnds, intNetwork.getNode(intArray.at(1))));
      EXPECT_TRUE(nodeWasFound(innerEnds, intNetwork.getNode(intArray.at(2))));
      EXPECT_TRUE(nodeWasFound(innerEnds, innerEnds.at(1)));


      B2INFO("case: when outer both were there and already linked: ");
      EXPECT_FALSE(intNetwork.linkNodes(intArray.at(0), intArray.at(2)));
      // nothing was added, everything the same as last case:
      EXPECT_EQ(sizeB4, intNetwork.size());
      EXPECT_EQ(3, intNetwork.getInnerEnds().size());
      EXPECT_EQ(4, intNetwork.getOuterEnds().size());
      std::vector<DirectedNode<int, VoidMetaInfo>*> moreInnerEnds = intNetwork.getNode(intArray.at(0))->getInnerNodes();
      EXPECT_EQ(innerEnds.size(), moreInnerEnds.size());
      EXPECT_TRUE(nodeWasFound(innerEnds, moreInnerEnds.at(0)));
      EXPECT_TRUE(nodeWasFound(innerEnds, moreInnerEnds.at(1)));
    }


    B2INFO("testing members for filling, when (at least) one entry was already there:"); {
      B2INFO("case: addInnerToLastOuterNode: both were there, but not linked yet: ");
      unsigned int networkSizeB4 = intNetwork.size();
      unsigned int nInnerEndsB4 = intNetwork.getInnerEnds().size();
      intNetwork.addInnerToLastOuterNode(intArray.at(3));
      EXPECT_EQ(networkSizeB4, intNetwork.size());
      EXPECT_EQ(nInnerEndsB4, intNetwork.getInnerEnds().size());
      std::vector<DirectedNode<int, VoidMetaInfo>*> innerEnds = intNetwork.getNode(intArray.at(0))->getInnerNodes();
      EXPECT_EQ(3, innerEnds.size());

      EXPECT_TRUE(nodeWasFound(innerEnds, intNetwork.getNode(intArray.at(1))));
      EXPECT_TRUE(nodeWasFound(innerEnds, intNetwork.getNode(intArray.at(2))));
      EXPECT_TRUE(nodeWasFound(innerEnds, intNetwork.getNode(intArray.at(3))));
    }


    {
      B2INFO("case: addInnerToLastOuterNode: both were there, but already linked (same results as before, but with an error for unintended behavior):");
      unsigned int networkSizeB4 = intNetwork.size();
      unsigned int nInnerEndsB4 = intNetwork.getInnerEnds().size();
      intNetwork.addInnerToLastOuterNode(intArray.at(3));
      EXPECT_EQ(networkSizeB4, intNetwork.size());
      EXPECT_EQ(nInnerEndsB4, intNetwork.getInnerEnds().size());
      std::vector<DirectedNode<int, VoidMetaInfo>*> innerEnds = intNetwork.getNode(intArray.at(0))->getInnerNodes();
      EXPECT_EQ(3, innerEnds.size());

      EXPECT_TRUE(nodeWasFound(innerEnds, intNetwork.getNode(intArray.at(1))));
      EXPECT_TRUE(nodeWasFound(innerEnds, intNetwork.getNode(intArray.at(2))));
      EXPECT_TRUE(nodeWasFound(innerEnds, intNetwork.getNode(intArray.at(3))));
    }
    /*
    {
      B2INFO("case: addInnerToLastOuterNode: inner was not there yet (innerEndsUpdate):");
      onTheFlyCreatedInts.push_back(31);
      int& lastOuterNodeInt = intNetwork.getLastOuterNode()->getEntry();
      int& newInnerInt = onTheFlyCreatedInts.back();
      unsigned int networkSizeB4 = intNetwork.size();
      unsigned int nInnerEndsB4 = intNetwork.getInnerEnds().size();
      unsigned int nInnerNodesB4 = intNetwork.getNode(lastOuterNodeInt)->getInnerNodes().size();
      intNetwork.addInnerToLastOuterNode(newInnerInt);
      EXPECT_EQ(networkSizeB4 + 1, intNetwork.size());
      EXPECT_EQ(nInnerEndsB4 + 1, intNetwork.getInnerEnds().size());
      EXPECT_EQ(nInnerNodesB4 + 1, intNetwork.getNode(lastOuterNodeInt)->getInnerNodes().size());

      EXPECT_TRUE(nodeWasFound(intNetwork.getNode(lastOuterNodeInt)->getInnerNodes(), intNetwork.getNode(newInnerInt)));
      auto innerEnds = intNetwork.getInnerEnds();
      EXPECT_TRUE(nodeWasFound(innerEnds, intNetwork.getNode(newInnerInt)));
    }

    {
      B2INFO("case: addOuterToLastInnerNode: both were there, but not linked yet:");
      int& lastInnerNodeInt = intNetwork.getLastInnerNode()->getEntry();
      unsigned int networkSizeB4 = intNetwork.size();
      unsigned int nOuterEndsB4 = intNetwork.getOuterEnds().size();
      intNetwork.addOuterToLastInnerNode(intArray2.at(1));
      EXPECT_EQ(networkSizeB4, intNetwork.size());
      EXPECT_EQ(nOuterEndsB4, intNetwork.getOuterEnds().size());
      std::vector<DirectedNode<int, VoidMetaInfo>*> outerNodes = intNetwork.getNode(lastInnerNodeInt)->getOuterNodes();
      EXPECT_EQ(2, outerNodes.size());
    }

    {
      B2INFO("case: addOuterToLastInnerNode: both were there, but already linked (same results as before, but with an error for unintended behavior):");
      int& lastInnerNodeInt = intNetwork.getLastInnerNode()->getEntry();
      unsigned int networkSizeB4 = intNetwork.size();
      unsigned int nOuterEndsB4 = intNetwork.getOuterEnds().size();
      intNetwork.addOuterToLastInnerNode(intArray2.at(1));
      EXPECT_EQ(networkSizeB4, intNetwork.size());
      EXPECT_EQ(nOuterEndsB4, intNetwork.getOuterEnds().size());
      std::vector<DirectedNode<int, VoidMetaInfo>*> outerNodes = intNetwork.getNode(lastInnerNodeInt)->getOuterNodes();
      EXPECT_EQ(2, outerNodes.size());
    }

    {
      B2INFO("case: addOuterToLastInnerNode: outer was not there yet (outerEndsUpdate):");
      onTheFlyCreatedInts.push_back(66);
      int& lastInnerNodeInt = intNetwork.getLastInnerNode()->getEntry();
      int& newOuterInt = onTheFlyCreatedInts.back();
      unsigned int networkSizeB4 = intNetwork.size();
      unsigned int nOuterEndsB4 = intNetwork.getOuterEnds().size();
      unsigned int nOuterNodesB4 = intNetwork.getNode(lastInnerNodeInt)->getOuterNodes().size();
      intNetwork.addOuterToLastInnerNode(newOuterInt);
      EXPECT_EQ(networkSizeB4 + 1, intNetwork.size());
      EXPECT_EQ(nOuterEndsB4 + 1, intNetwork.getOuterEnds().size());
      EXPECT_EQ(nOuterNodesB4 + 1, intNetwork.getNode(lastInnerNodeInt)->getOuterNodes().size());

      EXPECT_TRUE(nodeWasFound(intNetwork.getNode(lastInnerNodeInt)->getOuterNodes(), intNetwork.getNode(newOuterInt)));
      auto outerEnds = intNetwork.getOuterEnds();
      EXPECT_TRUE(nodeWasFound(outerEnds, intNetwork.getNode(newOuterInt)));
    }
    */
  }


  /** testing full functionality of the DirectedNodeNetwork when filled with a complex type (including storing on the storeArray).
   * This is stored in the DirectedNetworkContainer, which will actually be used by some modules.
   *  This test is intended as a usage example to find out how to use this network. */
  /*
  TEST_F(DirectedNodeNetworkTest, CreateRealisticNetwork)
  {
    // testing to write that container now onto the datastore:
    DirectedNodeNetworkContainer* realisticNetworkPtr = networkContainerInDataStore.appendNew();
    DirectedNodeNetwork<TrackNode, VoidMetaInfo> hitNetwork = realisticNetworkPtr->accessHitNetwork();
    vector<TrackNode*> trackNodes = realisticNetworkPtr->accessTrackNodes();

    const DirectedNodeNetworkContainer::StaticSectorType dummyStaticSector;
  //  const StaticSectorType * staticSectorPtr = &dummyStaticSector; // WARNING TODO warum funzt diese Konvertierung net?!?
    ActiveSector<DirectedNodeNetworkContainer::StaticSectorType, TrackNode> dummyActiveSector(&dummyStaticSector);

    EXPECT_EQ(0, hitNetwork.size());

    unsigned nEntries = spacePointData.getEntries();
    for (SpacePoint& aSP : spacePointData) {
      trackNodes.push_back(new TrackNode());
      TrackNode* node = trackNodes.back();
      node->spacePoint = &aSP;
      node->sector = &dummyActiveSector;
    }

    // filling: correct usage:
    // tests case when both nodes are new and when inner one is new, but outer one not:
    for (unsigned int index = 1 ; index < nEntries; index++) {
      TrackNode* outerNode = trackNodes[index - 1];
      TrackNode* innerNode = trackNodes[index];
      SpacePoint& outerSP = *(outerNode->spacePoint);
      SpacePoint& innerSP = *(innerNode->spacePoint);

      B2INFO("CreateRealisticNetwork: index " << index);

      // correct order: outerEntry, innerEntry:
      hitNetwork.linkTheseEntries(*outerNode, *innerNode);

      // innerEnd has been updated:
      EXPECT_EQ(innerSP, *(hitNetwork.getInnerEnds().at(0)->getEntry().spacePoint));

      // entries are now in network:
      EXPECT_EQ(outerSP, *(hitNetwork.getNode(*outerNode)->getEntry().spacePoint));
      EXPECT_EQ(innerSP, *(hitNetwork.getNode(*innerNode)->getEntry().spacePoint));
      EXPECT_EQ(*outerNode, *hitNetwork.getNode(*outerNode));
      EXPECT_EQ(*innerNode, *hitNetwork.getNode(*innerNode));

      // get all nodes of outer node, expected: 1 inner and no outerNodes:
      auto& innerNodes = hitNetwork.getNode(*outerNode)->getInnerNodes();
      auto& outerNodes = hitNetwork.getNode(*innerNode)->getOuterNodes();
      EXPECT_EQ(1, innerNodes.size());
      EXPECT_EQ(1, outerNodes.size());

      // array[index] is now linked as inner node of array[index-1]:
      EXPECT_EQ(*(innerNodes.at(0)), *(hitNetwork.getNode(*innerNode)));
    }

    EXPECT_EQ(0, hitNetwork.getNode(*trackNodes[0])->getOuterNodes().size()); // the outermost node has no outerNodes
    EXPECT_EQ(0, hitNetwork.getNode(*trackNodes[4])->getInnerNodes().size()); // the innermost node has no innerNodes

    // some extra sanity checks, are inner- and outerEnds as expected?
    EXPECT_EQ(nEntries, hitNetwork.size());
    std::vector<DirectedNode<TrackNode, VoidMetaInfo>*> outerEnds = hitNetwork.getOuterEnds();
    std::vector<DirectedNode<TrackNode, VoidMetaInfo>*> innerEnds = hitNetwork.getInnerEnds();
    EXPECT_EQ(1, outerEnds.size());
    EXPECT_EQ(1, innerEnds.size());

    DirectedNode<TrackNode, VoidMetaInfo>* outermostNode = outerEnds.at(0);
    EXPECT_EQ(*spacePointData[0], *(outermostNode->getEntry().spacePoint));
    EXPECT_EQ(*spacePointData[1], *(outermostNode->getInnerNodes().at(0)->getEntry().spacePoint));

    DirectedNode<TrackNode, VoidMetaInfo>* innermostNode = innerEnds.at(0);
    EXPECT_EQ(*spacePointData[4], *(innermostNode->getEntry().spacePoint));
    EXPECT_EQ(innermostNode->getEntry(), *innermostNode);
    EXPECT_EQ(*spacePointData[3], *(innermostNode->getOuterNodes().at(0)->getEntry().spacePoint));



    // preparing some extra data for testing:
    VxdID aID;
    VXD::SensorInfoBase aSensorInfo = provideSensorInfo(aID, 2, 4, 5);

    PXDCluster pxdCluster = providePXDCluster(2.3 , 4.2, aID);
    SpacePoint newSP = SpacePoint(&pxdCluster, &aSensorInfo);
    TrackNode newNode = TrackNode();
    newNode.spacePoint = & newSP;
    newNode.sector = &dummyActiveSector;


    // testing case, when outer node is new, but inner one not:
    TrackNode& oldNode = outermostNode->getEntry();
    hitNetwork.linkTheseEntries(newNode, oldNode);
    EXPECT_EQ(6, hitNetwork.size());

    std::vector<DirectedNode<TrackNode, VoidMetaInfo>*> newOuterEnds = hitNetwork.getOuterEnds();
    EXPECT_EQ(1, newOuterEnds.size());
    DirectedNode<TrackNode, VoidMetaInfo>* newOutermostNode = newOuterEnds.at(0);
    EXPECT_NE(oldNode, newOutermostNode->getEntry());
    EXPECT_EQ(newNode, newOutermostNode->getEntry());
    EXPECT_EQ(*spacePointData[0], *(newOutermostNode->getInnerNodes().at(0)->getEntry().spacePoint));
    EXPECT_EQ(newNode, *(outermostNode->getOuterNodes().at(0)));


    // testing case, when outer both were there, but not linked yet:
    hitNetwork.linkTheseEntries(*trackNodes[0], *trackNodes[2]);
    EXPECT_EQ(6, hitNetwork.size()); // size of network does not change
    std::vector<DirectedNode<TrackNode, VoidMetaInfo>*> moreInnerEnds = hitNetwork.getNode(*trackNodes[0])->getInnerNodes();
    EXPECT_EQ(2, moreInnerEnds.size());
    EXPECT_EQ(*trackNodes[1], *moreInnerEnds.at(0));
    EXPECT_EQ(*trackNodes[2], *moreInnerEnds.at(1));


    // testing case, when outer both were there and already linked:
    hitNetwork.linkTheseEntries(*trackNodes[0], *trackNodes[2]);

    // nothing was added, everything the same as last case:
    EXPECT_EQ(6, hitNetwork.size());
    std::vector<DirectedNode<TrackNode, VoidMetaInfo>*> evenMoreInnerEnds = hitNetwork.getNode(*trackNodes[0])->getInnerNodes();
    EXPECT_EQ(moreInnerEnds.size(), evenMoreInnerEnds.size());
    EXPECT_EQ(*moreInnerEnds.at(0), *evenMoreInnerEnds.at(0));
    EXPECT_EQ(*moreInnerEnds.at(1), *evenMoreInnerEnds.at(1));
  }*/
} // end namespace



