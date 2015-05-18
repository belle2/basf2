/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <gtest/gtest.h>

#include <tracking/spacePointCreation/PurityCalculatorTools.h>

#include <algorithm>

#include <framework/logging/Logger.h> // for B2INFO, etc...
#include <vxd/geometry/SensorInfoBase.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <tracking/spacePointCreation/SpacePoint.h>

// SpacePointTrackCand is at the moment the only container that gets tested
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

using namespace std;
using namespace Belle2;

namespace PurityCalcTests {

  /**
   * create a (very basic) TrueHit
   * NOTE: test do not need information from the TrueHits below, only the relations and the weights of the relations are important!
   */
  template<typename TrueHitType>
  TrueHitType createTrueHit(VxdID sensorId, float u = 0.1, float v = 0.2)
  {
    float position[3] = { u, v, 0. };
    float momentum[3] = { 0.1, 0.1, 0.1 };

    return TrueHitType(sensorId, position, position, position, momentum, momentum, momentum, 0.2, 0.1);
  }

  /**
   * create a MCParticle. although this returns only the empty constructor of the MCParticle a function is
   * wrapped around it in case some information has to be retrieved from the MCParticle -> can then be added here!
   */
  MCParticle createMCParticle()
  {
    return MCParticle();
  }

  /**
   * create a SensorInfoBase (needed for creation of SpacePoints)
   */
  VXD::SensorInfoBase createSensorInfo(VxdID sensorID, VXD::SensorInfoBase::SensorType type)
  {
    VXD::SensorInfoBase sensorInfoBase(type, sensorID, 2.3, 4.2, 0.3, 2, 4, -1);

    TGeoRotation r1;
    r1.SetAngles(45, 20, 30);      // rotation defined by Euler angles
    TGeoTranslation t1(0., 0., -0.);
    TGeoCombiTrans c1(t1, r1);
    TGeoHMatrix transform = c1;
    sensorInfoBase.setTransformation(transform);

    return sensorInfoBase;
  }

  /**
   * create a SpacePoint on the given sensor.
   * @param pxd indicates if the underlying Clusters should be pxd or svd
   * @param nClusters: only used for SVD! if 0 U & V set, if < 0 only V set, if > 0 only U set
   */
  SpacePoint createSpacePoint(VxdID sensorId, bool pxd, short nClusters = 0, double u = 0.1, double v = 0.2)
  {
    if (pxd) {
      VXD::SensorInfoBase sensorInfo = createSensorInfo(sensorId, VXD::SensorInfoBase::PXD);
      PXDCluster pxdCluster(sensorId, u, v, 0.1, 0.1, 0, 0, 1, 1, 1, 1, 1, 1);
      return SpacePoint(&pxdCluster, &sensorInfo);
    } else {
      VXD::SensorInfoBase sensorInfo = createSensorInfo(sensorId, VXD::SensorInfoBase::SVD);
      vector<const SVDCluster*> clusters;

      if (!nClusters) { // if 0 set both
        clusters.push_back(new SVDCluster(sensorId, true, u, 0.1, 0.001, 1, 1, 1));
        clusters.push_back(new SVDCluster(sensorId, false, v, 0.1, 0.001, 1, 1, 1));
      } else { // set only one coordinate
        bool setU = (nClusters > 0);
        clusters.push_back(new SVDCluster(sensorId, setU, u, 0.1, 0.001, 1, 1, 1)); // position doesnot actually matter!
      }
      return SpacePoint(clusters, &sensorInfo);
    }
    return SpacePoint(); // empty SP as default but should not happen!
  }

  struct compFirst {
    explicit compFirst(int j) : i(j) { }
    bool operator()(const pair<int, short>& p)
    {
      return p.first == i;
    }
  private:
    int i;
  };

  /**
   * class for testing the purity calculator tools (i.e. functions) defined in the spacePointCreation includes
   */
  class PurityCalculatorToolsTest : public ::testing::Test {
  protected:

    /** set up the mockup that is needed by the tests
     * + a mini StoreArray with SpacePoints, TrueHits and MCParticles
     * + relations between TrueHits and MCparticles and between SpacePoints and TrueHits
     */
    virtual void SetUp()
    {
      DataStore::Instance().setInitializeActive(true);

      // register everything necessary in the datastore
      m_pxdTrueHits.registerInDataStore("PXDTHs");
      m_svdTrueHits.registerInDataStore("SVDTHs");
      m_spacePoints.registerInDataStore("SPs");
      m_mcParticles.registerInDataStore("MCPs");

      // register the needed relations
      m_spacePoints.registerRelationTo(m_svdTrueHits);
      m_spacePoints.registerRelationTo(m_pxdTrueHits);
      m_mcParticles.registerRelationTo(m_svdTrueHits);
      m_mcParticles.registerRelationTo(m_pxdTrueHits);

      DataStore::Instance().setInitializeActive(false);

      // populate datastore
      MCParticle* mcPart1 = m_mcParticles.appendNew(createMCParticle());
      MCParticle* mcPart2 = m_mcParticles.appendNew(createMCParticle());

      // add a PXD (ideal case: one TrueHit -> one MCParticle)
      VxdID pxdId(1, 1, 1);
      PXDTrueHit* pTrueHit = m_pxdTrueHits.appendNew(createTrueHit<PXDTrueHit>(pxdId));
      mcPart1->addRelationTo(pTrueHit);
      SpacePoint* spacePoint = m_spacePoints.appendNew(createSpacePoint(pxdId, true));
      spacePoint->addRelationTo(pTrueHit, 1); // PXD has only one Cluster
      m_assignedClusters.push_back(1);

      // add a PXD (non-ideal case: no TrueHit)
      spacePoint = m_spacePoints.appendNew(createSpacePoint(pxdId, true));
      m_assignedClusters.push_back(1);

      // add a PXD (non-ideal case: more than one (i.e. two for the test) TrueHits -> 2 MCParticles)
      PXDTrueHit* pTrueHit2 = m_pxdTrueHits.appendNew(createTrueHit<PXDTrueHit>(pxdId));
      mcPart2->addRelationTo(pTrueHit2);
      spacePoint = m_spacePoints.appendNew(createSpacePoint(pxdId, true));
      spacePoint->addRelationTo(pTrueHit, 1);
      spacePoint->addRelationTo(pTrueHit2, 1);
      m_assignedClusters.push_back(1);

      // add a PXD (non-ideal case: more than one (i.e. two for the test) TrueHits -> 1 MCParticles)
      // -> both TrueHits are related with the same MCParticle (probably a very rare corner case, but this is what testing is for)
      PXDTrueHit* pTrueHit3 = m_pxdTrueHits.appendNew(createTrueHit<PXDTrueHit>(pxdId));
      mcPart2->addRelationTo(pTrueHit3);
      spacePoint = m_spacePoints.appendNew(createSpacePoint(pxdId, true));
      spacePoint->addRelationTo(pTrueHit3, 1);
      spacePoint->addRelationTo(pTrueHit2, 1);
      m_assignedClusters.push_back(1);

      // NOTE: SVD only testing two Clusters SpacePoints since single Clusters are essentially the same as PXDs!
      // SVD: first set up some SVDTrueHits with differing relations to MCParticles
      VxdID svdId(3, 1, 1);
      SVDTrueHit* sTH2MC1 = m_svdTrueHits.appendNew(createTrueHit<SVDTrueHit>(svdId)); // related to mcParticle1
      mcPart1->addRelationTo(sTH2MC1);
      SVDTrueHit* sTH2MC2 = m_svdTrueHits.appendNew(createTrueHit<SVDTrueHit>(svdId)); // related to mcParticle2
      mcPart2->addRelationTo(sTH2MC2);
      SVDTrueHit* sTH2None = m_svdTrueHits.appendNew(createTrueHit<SVDTrueHit>(svdId)); // related to no MCParticle

      // add a SVD (ideal case: two Clusters -> 1 TrueHit -> 1 MCParticle)
      spacePoint = m_spacePoints.appendNew(createSpacePoint(svdId, false, 0));   // add twoCluster SP
      spacePoint->addRelationTo(sTH2MC1, 2); // both Clusters to one TrueHit
      m_assignedClusters.push_back(2);

      // add a SVD (non-ideal case: twoClusters -> 2 TrueHits -> 2 MCParticles)
      spacePoint = m_spacePoints.appendNew(createSpacePoint(svdId, false, 0));   // add twoCluster SP
      m_assignedClusters.push_back(2);
      spacePoint->addRelationTo(sTH2MC1, 1);
      spacePoint->addRelationTo(sTH2MC2, 1);

      // add a SVD (non-ideal case: no related TrueHits)
      spacePoint = m_spacePoints.appendNew(createSpacePoint(svdId, false, 0));   // add twoCluster SP
      m_assignedClusters.push_back(2);

      // add a SVD (non-ideal case: twoClusters -> 2 TrueHits -> 2 MCParticles with different numbers of related Clusters
      // for TrueHits)
      spacePoint = m_spacePoints.appendNew(createSpacePoint(svdId, false, 0));   // add twoCluster SP
      m_assignedClusters.push_back(2);
      spacePoint->addRelationTo(sTH2MC1, 2); // both Clusters related to this TrueHit
      spacePoint->addRelationTo(sTH2MC2, 1); // only one Cluster related to TrueHit

      // add a SVD (non-ideal case: only one Cluster with relation to ONE TrueHit)
      spacePoint = m_spacePoints.appendNew(createSpacePoint(svdId, false, 0));   // add twoCluster SP
      m_assignedClusters.push_back(2);
      spacePoint->addRelationTo(sTH2MC1, 1); // one Cluster with relation to TrueHit with relation to MCParticle

      // add a SVD (non-ideal case: only one TrueHit has a relation to a MCParticle)
      // add a SVD (non-ideal case: no related TrueHits)
      spacePoint = m_spacePoints.appendNew(createSpacePoint(svdId, false, 0));   // add twoCluster SP
      m_assignedClusters.push_back(2);
      spacePoint->addRelationTo(sTH2MC1, 1);
      spacePoint->addRelationTo(sTH2None, 1);


      // add a SVD combo with a normal two Cluster SpacePoint

      // SVDTrueHit* sTrueHit = m_svdTrueHits.appendNew(createTrueHit<SVDTrueHit>(svdId));
      // mcPart1->addRelationTo(sTrueHit);
      // spacePoint = m_spacePoints.appendNew(createSpacePoint(svdId, false, 0));   // add twoCluster SP


      // // add a SVD combo with a 'ghost hit'
      // sTrueHit = m_svdTrueHits.appendNew(createTrueHit<SVDTrueHit>(svdId));
      // mcPart1->addRelationTo(sTrueHit);
      // SVDTrueHit* sTrueHit2 = m_svdTrueHits.appendNew(createTrueHit<SVDTrueHit>(svdId));
      // mcPart2->addRelationTo(sTrueHit2);
      // spacePoint = m_spacePoints.appendNew(createSpacePoint(svdId, false, 0)); // add two cluster SP
      // spacePoint->addRelationTo(sTrueHit, 1);
      // spacePoint->addRelationTo(sTrueHit2, 1);

      // // add a SVD combo where one of the TrueHits has no relation to a MCParticle
      // sTrueHit = m_svdTrueHits.appendNew(createTrueHit<SVDTrueHit>(svdId));
      // mcPart2->addRelationTo(sTrueHit);
      // sTrueHit2 = m_svdTrueHits.appendNew(createTrueHit<SVDTrueHit>(svdId)); // make TH without relation to MCPart
      // spacePoint = m_spacePoints.appendNew(createSpacePoint(svdId, false, 0));
      // spacePoint->addRelationTo(sTrueHit);
      // spacePoint->addRelationTo(sTrueHit2);

      // // add a single Cluster SVD SpacePoint to check if those are handled right
      // // NOTE: this SP is related to a TrueHit that is also connected to another SpacePoint to check if there is no interference
      // spacePoint = m_spacePoints.appendNew(createSpacePoint(svdId, false, 1)); // make one Cluster SP
      // spacePoint->addRelationTo(sTrueHit);

      // // add a SpacePoint that has only one Cluster connected to a TrueHit
      // spacePoint = m_spacePoints.appendNew(createSpacePoint(svdId, true, 0));
      // sTrueHit = m_svdTrueHits.appendNew(createTrueHit<SVDTrueHit>(svdId));
      // spacePoint->addRelationTo(sTrueHit);

      // add a SpacePoint that has a Cluster with two TrueHits



      B2INFO("Contents of DataStore after SetUp: pxdTrueHits: " << m_pxdTrueHits.getEntries() <<
             " svdTrueHits: " << m_svdTrueHits.getEntries() << " mcParticles: " << m_mcParticles.getEntries() <<
             " spacePoints: " << m_spacePoints.getEntries());
    }

    /** tear down environment after test -> clear datastore */
    virtual void TearDown() { DataStore::Instance().reset(); }

    // members to be used in the tests
    StoreArray<Belle2::PXDTrueHit> m_pxdTrueHits; /**< some PXDTrueHits for testing */
    StoreArray<Belle2::SVDTrueHit> m_svdTrueHits; /**< some SVDTrueHits for testing */
    StoreArray<Belle2::SpacePoint> m_spacePoints; /**< some SpacePoints for testing */
    StoreArray<Belle2::MCParticle> m_mcParticles; /**< some MCParticles for testing */

    std::vector<short> m_assignedClusters; /**< store the expected number of assigned Clusters in vector to be able to loop over it */
  };

  /**
   * test that checks if the setup works as expected
   */
  TEST_F(PurityCalculatorToolsTest, testSetUp)
  {
    // test if StoreArray has all the entries that should be there
    EXPECT_EQ(m_pxdTrueHits.getEntries(), 3);
    EXPECT_EQ(m_svdTrueHits.getEntries(), 3);
    EXPECT_EQ(m_spacePoints.getEntries(), 10);
    EXPECT_EQ(m_mcParticles.getEntries(), 2);

    // test if the SpacePoints have the expected number of assigned Clusters
    for (int i = 0; i < m_spacePoints.getEntries(); ++i) {
      EXPECT_EQ(m_spacePoints[i]->getNClustersAssigned(), m_assignedClusters[i]);
    }
  }

  /** test if the getMCParticles function returns the expected values. This test can also be seen as a check if
   * mock up works!
   * CAUTION: changing the SetUp might break things here as the order is assumed as in SetUp!
   */
  // TEST_F(PurityCalculatorToolsTest, testGetMCParticles)
  // {
  //   // check first SP in StoreArray
  //   vector<pair<int, short> > mcParts = getMCParticles<PXDTrueHit>(m_spacePoints[0]);
  //   EXPECT_EQ(0, mcParts[0].first);
  //   EXPECT_EQ(1, mcParts[0].second);
  //   EXPECT_EQ(1, mcParts.size());

  //   // check two Cluster SpacePoint in StoreArray
  //   mcParts = getMCParticles<SVDTrueHit>(m_spacePoints[1]);
  //   EXPECT_EQ(0, mcParts[0].first);
  //   EXPECT_EQ(2, mcParts[0].second);


  //   // check ghost hit SP in store Array
  //   mcParts = getMCParticles<SVDTrueHit>(m_spacePoints[2]);
  //   EXPECT_EQ(2, mcParts.size());

  //   auto findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(1));
  //   EXPECT_FALSE(findIt == mcParts.end());
  //   findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(0));
  //   EXPECT_FALSE(findIt == mcParts.end());

  //   EXPECT_EQ(1, mcParts[0].second);
  //   EXPECT_EQ(1, mcParts[1].second);

  //   // check two Cluster SpacePoint with one TH without relation to MCParticle
  //   mcParts = getMCParticles<SVDTrueHit>(m_spacePoints[3]);
  //   EXPECT_EQ(2, mcParts.size());
  //   findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(-1));
  //   EXPECT_FALSE(findIt == mcParts.end());
  //   EXPECT_EQ(findIt->second, 1);
  //   findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(1));
  //   EXPECT_FALSE(findIt == mcParts.end());
  //   EXPECT_EQ(findIt->second, 1);

  //   // check single Cluster SP
  //   mcParts = getMCParticles<SVDTrueHit>(m_spacePoints[4]);
  //   EXPECT_EQ(mcParts[0].first, 1);
  //   EXPECT_EQ(mcParts[0].second, 1);

  //   // TODO: check for SpacePoint with no TH relation
  //   mcParts = getMCParticles<SVDTrueHit>(m_spacePoints[5]);
  //   EXPECT_EQ(1, mcParts.size());
  //   findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(-1)); // TrueHit has no connection to a MCParticle
  //   EXPECT_FALSE(findIt == mcParts.end());
  //   EXPECT_EQ(1, findIt->second);
  //   findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(-2)); // Cluster has no relation to TrueHit
  //   EXPECT_FALSE(findIt == mcParts.end());
  //   EXPECT_EQ(1, findIt->second);

  // }

  /**
   * test the calculatePurity function by comparing the calculated purities with expected values (known from mockup construction)
   */
  TEST_F(PurityCalculatorToolsTest, testCalculatePurity)
  {
    // TODO !!
  }

} // namespace
