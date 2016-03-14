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


#include <vxd/geometry/SensorInfoBase.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/spacePointCreation/sptcNetwork/TCNetworkContainer.h>
#include <tracking/spacePointCreation/sptcNetwork/TCCompetitorGuard.h>
#include <tracking/spacePointCreation/sptcNetwork/SPTCAvatar.h>
#include <tracking/spacePointCreation/sptcNetwork/SpTcNetwork.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
// #include <framework/dataobjects/EventMetaData.h>
// #include <framework/dataobjects/ProfileInfo.h>
// #include <framework/datastore/RelationsObject.h>



using namespace std;
using namespace Belle2;

/**
 *
 * These tests cover the functionality of the classes:
 * SPTCAvatar, TCCompetitor, TCNetworkContainer.
 *
 */
namespace TCNetworkContainerTests {


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
  class TCNetworkContainerTest : public ::testing::Test {
  protected:

    /** SetUp environment - prepare related storearrays of SpacePoints and PXDClusters */
    virtual void SetUp()
    {
      spacePointData.registerInDataStore();
      pxdClusterData.registerInDataStore();
      spacePointTrackCandData.registerInDataStore();

      spacePointData.registerRelationTo(pxdClusterData);

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

      B2DEBUG(10, "TCNetworkContainerTest:SetUP: created " << pxdClusterData.getEntries() << "/" << spacePointData.getEntries() <<
              " pxdClusters/SpacePoints")

      /// prepare some SpacePointTrackCands partially overlapping:
      vector<SpacePoint*> allSpacePoints;
      for (SpacePoint& aSP : spacePointData) {
        allSpacePoints.push_back(&aSP);
      }

      vector<const SpacePoint*> sps4TC1 = { allSpacePoints.at(0), allSpacePoints.at(1)};
      spacePointTrackCandData.appendNew((sps4TC1)); // shares hits with no one

      vector<const SpacePoint*> sps4TC2 = { allSpacePoints.at(2), allSpacePoints.at(3)};
      spacePointTrackCandData.appendNew((sps4TC2)); // shares a hit with tc3

      vector<const SpacePoint*> sps4TC3 = { allSpacePoints.at(3), allSpacePoints.at(4)};
      spacePointTrackCandData.appendNew((sps4TC3)); // shares a hit with tc2
    }

    /** TearDown environment - clear datastore */
    virtual void TearDown() { DataStore::Instance().reset(); }

    StoreArray<SpacePoint> spacePointData; /**< some spacePoints for testing. */
    StoreArray<PXDCluster> pxdClusterData; /**< some pxd clusters for testing. */
    StoreArray<SpacePointTrackCand> spacePointTrackCandData; /**< some spacePointTrackCands for testing. */
  };


  /** small observer class to test basic functionality of SpTcAvatar */
  class MicroObserver {
  public:

    MicroObserver() {}

    template<typename vecType>
    MicroObserver(vecType& observedVector) { B2INFO("MicroObserver:constructor: nEntries found in observedVector " << observedVector.size()); }


    void notifyRemove(unsigned int iD)
    {
      B2INFO("MicroObserver:notivyRemove: given iD: " << iD)
    }
  };


  /** basic sanity checks: */
  TEST_F(TCNetworkContainerTest, MockupSanityCheck)
  {
    EXPECT_EQ(5, spacePointData.getEntries());
    EXPECT_EQ(pxdClusterData.getEntries(), spacePointData.getEntries());
    EXPECT_EQ(3, spacePointTrackCandData.getEntries());
  }


  /** test SPTCAvatar to be used with a generic observer */
  TEST_F(TCNetworkContainerTest, BasicTestWithGenericObserver)
  {
    SpacePointTrackCand newSPTC, newSPTC2;

    MicroObserver newObserver;

    ///SPTCAvatar(SpacePointTrackCand& myOriginal, ObserverType& myObserver, unsigned int myID)
    SPTCAvatar<MicroObserver> avatar0(newSPTC, newObserver, 0);

    TCNetworkContainer<SPTCAvatar<MicroObserver>, MicroObserver > newNetwork;

    newNetwork.add(avatar0);

    newNetwork.add(SPTCAvatar<MicroObserver>(newSPTC2, newObserver, newNetwork.size()));

    EXPECT_EQ(2, newNetwork.getNTCsAlive());
    EXPECT_EQ(0, newNetwork.getNCompetitors());

    newNetwork.killTC(0); // informs observer too
    EXPECT_EQ(1, newNetwork.getNTCsAlive());
    EXPECT_EQ(0, newNetwork.getNCompetitors()); // didn't change
  }





  /** test SPTCAvatar to be used with a realistic observer */
  TEST_F(TCNetworkContainerTest, BasicTestWithTCCompetitorObserver)
  {
    SpacePointTrackCand newSPTC;

    TCNetworkContainer<SPTCAvatar<TCCompetitorGuard >, TCCompetitorGuard > newNetwork;

    // _not_ recommendable way to add new avatars to TCNetwork:
    SPTCAvatar<TCCompetitorGuard> avatar0(newSPTC, newNetwork.getObserver(), newNetwork.size());
    newNetwork.add(avatar0);

    // recommended way to add new avatars to TCNetwork:
    newNetwork.add(SPTCAvatar<TCCompetitorGuard >(newSPTC, newNetwork.getObserver(), newNetwork.size()));

    EXPECT_EQ(2, newNetwork.getNTCsAlive());
    EXPECT_EQ(0, newNetwork.getNCompetitors());

    newNetwork.killTC(0); // informs observer too
    EXPECT_EQ(1, newNetwork.getNTCsAlive());
    EXPECT_EQ(0, newNetwork.getNCompetitors()); // didn't change
  }


  /** test SPTCAvatar to be used with a realistic observer and realistic SPTCs - with mockup set up */
  TEST_F(TCNetworkContainerTest, TestWithRealisticSPTCs)
  {
    /// create and fill network with SpacePointTrackCands
    TCNetworkContainer<SPTCAvatar<TCCompetitorGuard >, TCCompetitorGuard > newNetwork;
    EXPECT_EQ(0, newNetwork.size());
    EXPECT_EQ(0, newNetwork.getNCompetitors());
    EXPECT_EQ(0, newNetwork.getNTCsAlive());
    newNetwork.add(SPTCAvatar<TCCompetitorGuard >(*spacePointTrackCandData[0], newNetwork.getObserver(),
                                                  newNetwork.size()));    // tc0: shares hits with no one
    newNetwork.add(SPTCAvatar<TCCompetitorGuard >(*spacePointTrackCandData[1], newNetwork.getObserver(),
                                                  newNetwork.size()));   // tc1: shares a hit with tc2
    newNetwork.add(SPTCAvatar<TCCompetitorGuard >(*spacePointTrackCandData[2], newNetwork.getObserver(),
                                                  newNetwork.size()));   // tc2: shares a hit with tc1
    EXPECT_EQ(3, newNetwork.size());
    EXPECT_EQ(2, newNetwork.getNCompetitors());
    EXPECT_EQ(3, newNetwork.getNTCsAlive());

    newNetwork.killTC(0); // informs observer too
    EXPECT_EQ(2, newNetwork.getNCompetitors()); // TCacAvatar 0 was not competing with anyone
    EXPECT_EQ(2, newNetwork.getNTCsAlive());

    newNetwork.killTC(1); // informs observer too
    EXPECT_EQ(0, newNetwork.getNCompetitors()); // now no one is competing anymore
    EXPECT_EQ(1, newNetwork.getNTCsAlive());

  }


  /** test basic input and storeOBjPtr-feature for TestSpTCNetwork - with mockup set up */
  TEST_F(TCNetworkContainerTest, TestSpTcNetwork)
  {
    /// create and fill network with SpacePointTrackCands
    SpTcNetwork realisticNetwork;
    EXPECT_EQ(0, realisticNetwork.size());
    EXPECT_EQ(0, realisticNetwork.getNCompetitors());
    EXPECT_EQ(0, realisticNetwork.getNTCsAlive());

    realisticNetwork.add(*spacePointTrackCandData[0]);  // tc0: shares hits with no one
    realisticNetwork.add(*spacePointTrackCandData[1]); // tc1: shares a hit with tc2
    realisticNetwork.add(*spacePointTrackCandData[2]); // tc2: shares a hit with tc1
    EXPECT_EQ(3, realisticNetwork.size());
    EXPECT_EQ(2, realisticNetwork.getNCompetitors());
    EXPECT_EQ(3, realisticNetwork.getNTCsAlive());

    realisticNetwork.killTC(0); // informs observer too
    EXPECT_EQ(2, realisticNetwork.getNCompetitors()); // TCacAvatar 0 was not competing with anyone
    EXPECT_EQ(2, realisticNetwork.getNTCsAlive());

    realisticNetwork.killTC(1); // informs observer too
    EXPECT_EQ(0, realisticNetwork.getNCompetitors()); // now no one is competing anymore
    EXPECT_EQ(1, realisticNetwork.getNTCsAlive());
  }

} // end namespace



