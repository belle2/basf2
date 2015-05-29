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

#include <vxd/geometry/SensorInfoBase.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationsObject.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>


#include <array>

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
      spacePointData.registerInDataStore();
      pxdClusterData.registerInDataStore();
      spacePointTrackCandData.registerInDataStore();

      spacePointData.registerRelationTo(pxdClusterData);

      networkContainerInDataStore.registerInDataStore();
      StoreObjPtr<DirectedNodeNetworkContainer> networkContainerInDataStore;

      DataStore::Instance().setInitializeActive(false);

      unsigned int nHits = 5;

      /// prepare pxdClusters and spacePoints related to them
      for (unsigned int i = 1; i <= nHits; ++i) { //

        VxdID aVxdID = VxdID(i, i, i);

        VXD::SensorInfoBase aSensorInfo = provideSensorInfo(aVxdID, (unsigned short)i, (unsigned short)i + 1., (unsigned short)i + 2.);

        const PXDCluster* pxdCluster = pxdClusterData.appendNew(providePXDCluster(float(i) / float(nHits), float(i) / float(nHits),
                                                                aVxdID));

        SpacePoint* newSP = spacePointData.appendNew(pxdCluster, &aSensorInfo);
        B2DEBUG(10, " setup: new spacePoint got arrayIndex: " << newSP->getArrayIndex() << " and VxdID " << newSP->getVxdID())
        newSP->addRelationTo(pxdCluster);
      }

      B2DEBUG(10, "DirectedNodeNetworkTest:SetUP: created " << pxdClusterData.getEntries() << "/" << spacePointData.getEntries() <<
              " pxdClusters/SpacePoints")

      /// prepare some SpacePointTrackCands partially overlapping:
      vector<SpacePoint*> allSpacePoints;
      for (SpacePoint& aSP : spacePointData) {
        allSpacePoints.push_back(&aSP);
      }

      vector<const SpacePoint*> sps4TC1 = { allSpacePoints.at(0), allSpacePoints.at(1)};
      SpacePointTrackCand* aSPTC1 = spacePointTrackCandData.appendNew((sps4TC1)); // shares hits with no one
      aSPTC1->setQualityIndex(0.92);

      vector<const SpacePoint*> sps4TC2 = { allSpacePoints.at(2), allSpacePoints.at(3)};
      SpacePointTrackCand* aSPTC2 = spacePointTrackCandData.appendNew((sps4TC2)); // shares a hit with tc3, tc4, tc5
      aSPTC2->setQualityIndex(0.9);

      vector<const SpacePoint*> sps4TC3 = { allSpacePoints.at(3), allSpacePoints.at(4)};
      SpacePointTrackCand* aSPTC3 = spacePointTrackCandData.appendNew((sps4TC3)); // shares a hit with tc2, tc5
      aSPTC3->setQualityIndex(0.8);

      vector<const SpacePoint*> sps4TC4 = { allSpacePoints.at(4)};
      SpacePointTrackCand* aSPTC4 = spacePointTrackCandData.appendNew((sps4TC4)); // shares a hit with tc3 too, but not with tc2
      aSPTC4->setQualityIndex(0.7);

      vector<const SpacePoint*> sps4TC5 = { allSpacePoints.at(2), allSpacePoints.at(4)};
      SpacePointTrackCand* aSPTC5 = spacePointTrackCandData.appendNew((sps4TC5)); // shares a hit with tc2 and tc3
      aSPTC5->setQualityIndex(0.65);
    }

    /** TearDown environment - clear datastore */
    virtual void TearDown() { DataStore::Instance().reset(); }

    StoreArray<SpacePoint> spacePointData; /**< some spacePoints for testing. */
    StoreArray<PXDCluster> pxdClusterData; /**< some pxd clusters for testing. */
    StoreArray<SpacePointTrackCand> spacePointTrackCandData; /**< some spacePointTrackCands for testing. */
    StoreArray<DirectedNodeNetworkContainer>
    networkContainerInDataStore; /**< testing to store a dummy class containing DirectedNodeNetwork as member . */
  };




  /** basic sanity checks: checks mockup for working properly */
  TEST_F(DirectedNodeNetworkTest, MockupSanityCheck)
  {
    EXPECT_EQ(5, pxdClusterData.getEntries());
    EXPECT_EQ(5, spacePointData.getEntries());
    EXPECT_EQ(pxdClusterData.getEntries(), spacePointData.getEntries());
    EXPECT_EQ(5, spacePointTrackCandData.getEntries());
  }



  /** testing basic functionality of the DirectedNodeNetwork when filled with a basic type - int */
  TEST_F(DirectedNodeNetworkTest, CreateNetworkInt)
  {
    // just some input for testing:
    std::array<int, 5> intArray  = { { 2, 5, 3, 4, 99} };
    EXPECT_EQ(5, intArray.size());

    DirectedNodeNetwork<int> intNetwork;
    EXPECT_EQ(0, intNetwork.size());

    // filling: correct usage:
    // tests case when both nodes are new and when inner one is new, but outer one not:
    for (unsigned int index = 1 ; index < 5; index++) {
      B2INFO("index " << index << " of array has entry: " << intArray.at(index));

      // correct order: outerEntry, innerEntry:
      intNetwork.linkTheseEntries(intArray.at(index - 1), intArray.at(index));

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
      EXPECT_EQ(*(innerNodes.at(0)), *(intNetwork.getNode(intArray.at(index)))); // TODO WARNING does not work yet (why?)
    }
    EXPECT_EQ(0, intNetwork.getNode(intArray.at(0))->getOuterNodes().size()); // the outermost node has no outerNodes
    EXPECT_EQ(0, intNetwork.getNode(intArray.at(4))->getInnerNodes().size()); // the innermost node has no innerNodes

    // some extra sanity checks, are inner- and outerEnds as expected?
    EXPECT_EQ(intArray.size(), intNetwork.size());
    std::vector<DirectedNode<int>*> outerEnds = intNetwork.getOuterEnds();
    std::vector<DirectedNode<int>*> innerEnds = intNetwork.getInnerEnds();
    EXPECT_EQ(1, outerEnds.size());
    EXPECT_EQ(1, innerEnds.size());

    DirectedNode<int>* outermostNode = outerEnds.at(0);
    EXPECT_EQ(intArray.at(0), outermostNode->getEntry());
    EXPECT_EQ(outermostNode->getEntry(), *outermostNode);
    EXPECT_EQ(intArray.at(1), *(outermostNode->getInnerNodes().at(0)));

    DirectedNode<int>* innermostNode = innerEnds.at(0);
    EXPECT_EQ(intArray.at(4), innermostNode->getEntry());
    EXPECT_EQ(innermostNode->getEntry(), *innermostNode);
    EXPECT_EQ(intArray.at(3), *(innermostNode->getOuterNodes().at(0)));


    // testing case, when outer node is new, but inner one not:
    int oldOuterInt =  outermostNode->getEntry();
    int shallBecomeNewOutermostInt = 42;
    intNetwork.linkTheseEntries(shallBecomeNewOutermostInt, oldOuterInt);
    EXPECT_EQ(6, intNetwork.size());

    std::vector<DirectedNode<int>*> newOuterEnds = intNetwork.getOuterEnds();
    EXPECT_EQ(1, newOuterEnds.size());
    DirectedNode<int>* newOutermostNode = newOuterEnds.at(0);
    EXPECT_NE(oldOuterInt, newOutermostNode->getEntry());
    EXPECT_EQ(shallBecomeNewOutermostInt, newOutermostNode->getEntry());
    EXPECT_EQ(intArray.at(0), *(newOutermostNode->getInnerNodes().at(0)));
    EXPECT_EQ(shallBecomeNewOutermostInt, *(outermostNode->getOuterNodes().at(0)));


    // testing case, when outer both were there, but not linked yet:
    intNetwork.linkTheseEntries(intArray.at(0), intArray.at(2));
    EXPECT_EQ(6, intNetwork.size()); // size of network does not change
    std::vector<DirectedNode<int>*> moreInnerEnds = intNetwork.getNode(intArray.at(0))->getInnerNodes();
    EXPECT_EQ(2, moreInnerEnds.size());
    EXPECT_EQ(intArray.at(1), *moreInnerEnds.at(0));
    EXPECT_EQ(intArray.at(2), *moreInnerEnds.at(1));


    // testing case, when outer both were there and already linked:
    intNetwork.linkTheseEntries(intArray.at(0), intArray.at(2));

    // nothing was added, everything the same as last case:
    EXPECT_EQ(6, intNetwork.size());
    std::vector<DirectedNode<int>*> evenMoreInnerEnds = intNetwork.getNode(intArray.at(0))->getInnerNodes();
    EXPECT_EQ(moreInnerEnds.size(), evenMoreInnerEnds.size());
    EXPECT_EQ(*moreInnerEnds.at(0), *evenMoreInnerEnds.at(0));
    EXPECT_EQ(*moreInnerEnds.at(1), *evenMoreInnerEnds.at(1));
  }


  /** testing full functionality of the DirectedNodeNetwork when filled with a complex type (including storing on the storeArray).
   * This is stored in the DirectedNetworkContainer, which will actually be used by some modules. */
  TEST_F(DirectedNodeNetworkTest, CreateRealisticNetwork)
  {
    // testing to write that container now onto the datastore:
    DirectedNodeNetworkContainer* realisticNetworkPtr = networkContainerInDataStore.appendNew();
    DirectedNodeNetwork<SpacePoint> spNetwork = realisticNetworkPtr->accessSpacePointNetwork();
    EXPECT_EQ(0, spNetwork.size());

    unsigned nEntries = spacePointData.getEntries();
    // filling: correct usage:
    // tests case when both nodes are new and when inner one is new, but outer one not:
    for (unsigned int index = 1 ; index < nEntries; index++) {
      SpacePoint& outerSP = *spacePointData[index - 1];
      SpacePoint& innerSP = *spacePointData[index];

      B2INFO("CreateRealisticNetwork: index " << index);

      // correct order: outerEntry, innerEntry:
      spNetwork.linkTheseEntries(outerSP, innerSP);

      // innerEnd has been updated:
      EXPECT_EQ(innerSP, spNetwork.getInnerEnds().at(0)->getEntry());

      // entries are now in network:
      EXPECT_EQ(outerSP, *spNetwork.getNode(outerSP));
      EXPECT_EQ(innerSP, *spNetwork.getNode(innerSP));

      // get all nodes of outer node, expected: 1 inner and no outerNodes:
      auto& innerNodes = spNetwork.getNode(outerSP)->getInnerNodes();
      auto& outerNodes = spNetwork.getNode(innerSP)->getOuterNodes();
      EXPECT_EQ(1, innerNodes.size());
      EXPECT_EQ(1, outerNodes.size());

      // array[index] is now linked as inner node of array[index-1]:
      EXPECT_EQ(*(innerNodes.at(0)), *(spNetwork.getNode(innerSP)));
    }

    EXPECT_EQ(0, spNetwork.getNode(*spacePointData[0])->getOuterNodes().size()); // the outermost node has no outerNodes
    EXPECT_EQ(0, spNetwork.getNode(*spacePointData[4])->getInnerNodes().size()); // the innermost node has no innerNodes

    // some extra sanity checks, are inner- and outerEnds as expected?
    EXPECT_EQ(nEntries, spNetwork.size());
    std::vector<DirectedNode<SpacePoint>*> outerEnds = spNetwork.getOuterEnds();
    std::vector<DirectedNode<SpacePoint>*> innerEnds = spNetwork.getInnerEnds();
    EXPECT_EQ(1, outerEnds.size());
    EXPECT_EQ(1, innerEnds.size());

    DirectedNode<SpacePoint>* outermostNode = outerEnds.at(0);
    EXPECT_EQ(*spacePointData[0], outermostNode->getEntry());
    EXPECT_EQ(outermostNode->getEntry(), *outermostNode);
    EXPECT_EQ(*spacePointData[1], *(outermostNode->getInnerNodes().at(0)));

    DirectedNode<SpacePoint>* innermostNode = innerEnds.at(0);
    EXPECT_EQ(*spacePointData[4], innermostNode->getEntry());
    EXPECT_EQ(innermostNode->getEntry(), *innermostNode);
    EXPECT_EQ(*spacePointData[3], *(innermostNode->getOuterNodes().at(0)));



    // preparing some extra data for testing:
    VxdID aID;
    VXD::SensorInfoBase aSensorInfo = provideSensorInfo(aID, 2, 4, 5);

    PXDCluster pxdCluster = providePXDCluster(2.3 , 4.2, aID);
    SpacePoint shallBecomeNewOutermostSP = SpacePoint(&pxdCluster, &aSensorInfo);


    // testing case, when outer node is new, but inner one not:
    SpacePoint& oldOuterSP = outermostNode->getEntry();;
    spNetwork.linkTheseEntries(shallBecomeNewOutermostSP, oldOuterSP);
    EXPECT_EQ(6, spNetwork.size());

    std::vector<DirectedNode<SpacePoint>*> newOuterEnds = spNetwork.getOuterEnds();
    EXPECT_EQ(1, newOuterEnds.size());
    DirectedNode<SpacePoint>* newOutermostNode = newOuterEnds.at(0);
    EXPECT_NE(oldOuterSP, newOutermostNode->getEntry());
    EXPECT_EQ(shallBecomeNewOutermostSP, newOutermostNode->getEntry());
    EXPECT_EQ(*spacePointData[0], *(newOutermostNode->getInnerNodes().at(0)));
    EXPECT_EQ(shallBecomeNewOutermostSP, *(outermostNode->getOuterNodes().at(0)));


    // testing case, when outer both were there, but not linked yet:
    spNetwork.linkTheseEntries(*spacePointData[0], *spacePointData[2]);
    EXPECT_EQ(6, spNetwork.size()); // size of network does not change
    std::vector<DirectedNode<SpacePoint>*> moreInnerEnds = spNetwork.getNode(*spacePointData[0])->getInnerNodes();
    EXPECT_EQ(2, moreInnerEnds.size());
    EXPECT_EQ(*spacePointData[1], *moreInnerEnds.at(0));
    EXPECT_EQ(*spacePointData[2], *moreInnerEnds.at(1));


    // testing case, when outer both were there and already linked:
    spNetwork.linkTheseEntries(*spacePointData[0], *spacePointData[2]);

    // nothing was added, everything the same as last case:
    EXPECT_EQ(6, spNetwork.size());
    std::vector<DirectedNode<SpacePoint>*> evenMoreInnerEnds = spNetwork.getNode(*spacePointData[0])->getInnerNodes();
    EXPECT_EQ(moreInnerEnds.size(), evenMoreInnerEnds.size());
    EXPECT_EQ(*moreInnerEnds.at(0), *evenMoreInnerEnds.at(0));
    EXPECT_EQ(*moreInnerEnds.at(1), *evenMoreInnerEnds.at(1));
  }
} // end namespace



