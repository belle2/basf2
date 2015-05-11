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

      // add a PXD combo
      VxdID pxdId(1, 1, 1);
      PXDTrueHit* pTrueHit = m_pxdTrueHits.appendNew(createTrueHit<PXDTrueHit>(pxdId));
      mcPart1->addRelationTo(pTrueHit);
      SpacePoint* spacePoint = m_spacePoints.appendNew(createSpacePoint(pxdId, true));
      spacePoint->addRelationTo(pTrueHit, 1); // PXD has only one Cluster

      // add a SVD combo with a normal two Cluster SpacePoint
      VxdID svdId(3, 1, 1);
      SVDTrueHit* sTrueHit = m_svdTrueHits.appendNew(createTrueHit<SVDTrueHit>(svdId));
      mcPart1->addRelationTo(sTrueHit);
      spacePoint = m_spacePoints.appendNew(createSpacePoint(svdId, false, 0));   // add twoCluster SP
      spacePoint->addRelationTo(sTrueHit, 2); // both Clusters to one TrueHit

      // add a SVD combo with a 'ghost hit'
      sTrueHit = m_svdTrueHits.appendNew(createTrueHit<SVDTrueHit>(svdId));
      mcPart1->addRelationTo(sTrueHit);
      SVDTrueHit* sTrueHit2 = m_svdTrueHits.appendNew(createTrueHit<SVDTrueHit>(svdId));
      mcPart2->addRelationTo(sTrueHit2);
      spacePoint = m_spacePoints.appendNew(createSpacePoint(svdId, false, 0)); // add two cluster SP
      spacePoint->addRelationTo(sTrueHit, 1);
      spacePoint->addRelationTo(sTrueHit2, 1);

      // TODO: populate further to be able to get the corner cases


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
  };

  /**
   * test that checks if the setup works as expected
   */
  TEST_F(PurityCalculatorToolsTest, testSetUp)
  {
    EXPECT_EQ(m_pxdTrueHits.getEntries(), 1);
    EXPECT_EQ(m_svdTrueHits.getEntries(), 3);
    EXPECT_EQ(m_spacePoints.getEntries(), 3);
    EXPECT_EQ(m_mcParticles.getEntries(), 2);
  }

  /** test if the getMCParticles function returns the expected values. This test can also be seen as a check if
   * mock up works!
   * CAUTION: changing the SetUp might break things here as the order is assumed as in SetUp!
   */
  TEST_F(PurityCalculatorToolsTest, testGetMCParticles)
  {
    // check first SP in StoreArray
    vector<pair<int, short> > mcParts = getMCParticles<PXDTrueHit>(m_spacePoints[0]);
    EXPECT_EQ(0, mcParts[0].first);
    EXPECT_EQ(1, mcParts[0].second);
    EXPECT_EQ(1, mcParts.size());

    // check ghost hit SP in store Array
    mcParts = getMCParticles<SVDTrueHit>(m_spacePoints[2]);
    EXPECT_EQ(2, mcParts.size());

    auto findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(1));
    EXPECT_FALSE(findIt == mcParts.end());
    findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(0));
    EXPECT_FALSE(findIt == mcParts.end());

    EXPECT_EQ(1, mcParts[0].second);
    EXPECT_EQ(1, mcParts[1].second);

    // check two Cluster SpacePoint in StoreArray
    mcParts = getMCParticles<SVDTrueHit>(m_spacePoints[1]);
    EXPECT_EQ(0, mcParts[0].first);
    EXPECT_EQ(2, mcParts[0].second);
  }

  /**
   * test the calculatePurity function by comparing the calculated purities with expected values (known from mockup construction)
   */
  TEST_F(PurityCalculatorToolsTest, testCalculatePurity)
  {
    // TODO !!
  }

} // namespace
