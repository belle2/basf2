/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <gtest/gtest.h>

#include <tracking/spacePointCreation/PurityCalculatorTools.h>

#include <algorithm>

#include <framework/logging/Logger.h> // for B2INFO, etc...
#include <vxd/geometry/SensorInfoBase.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationVector.h>
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
        clusters.push_back(new SVDCluster(sensorId, true, u, 1.0, 0.1, 0.001, 1, 1, 1, 1.0));
        clusters.push_back(new SVDCluster(sensorId, false, v, 1.0, 0.1, 0.001, 1, 1, 1, 1.0));
      } else { // set only one coordinate
        bool setU = (nClusters > 0);
        clusters.push_back(new SVDCluster(sensorId, setU, u, 1.0, 0.1, 0.001, 1, 1, 1, 1.0)); // position doesnot actually matter!
      }
      return SpacePoint(clusters, &sensorInfo);
    }
    return SpacePoint(); // empty SP as default but should not happen!
  }

  /**
   * small helper functor to find a pair in a container of pairs, where .first matches the passed integer
   * COULDDO: make this templated to take any arguments to the pair, instead of only ints and shorts
   */
  struct compFirst {
    explicit compFirst(int j) : i(j) { } /**< explicit constructor for use as functor with argument */
    bool operator()(const pair<int, short>& p)
    {
      return p.first == i;
    } /**< comparison operator returning true if .first is j (from ctor) */
  private:
    int i; /**< only member holding the value that has to be matched by the .first element of a pair */
  };

  /**
   * small helper functor to get the MCVXDPurityInfo with the passed ParticleId from a container of MCVXDPurityInfos
   */
  struct compMCId {
    explicit compMCId(int j) : i(j) { } /**< explicit constructor for use as functor with arguments */
    bool operator()(const MCVXDPurityInfo& info)
    {
      return info.getParticleID() == i;
    } /**< operator comparing the particle Id and the passed argument */
  private:
    int i; /**< member holding the value that has to be mateched by the .first element of a pair */
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
      m_spacePoints.appendNew(createSpacePoint(pxdId, true));
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
      spacePoint->addRelationTo(sTH2MC1, 11); // U-Cluster
      spacePoint->addRelationTo(sTH2MC2, 21); // V-Cluster

      // add a SVD (non-ideal case: no related TrueHits)
      m_spacePoints.appendNew(createSpacePoint(svdId, false, 0));   // add twoCluster SP
      m_assignedClusters.push_back(2);

      // add a SVD (non-ideal case: twoClusters -> 2 TrueHits -> 2 MCParticles with different numbers of related Clusters
      // for TrueHits)
      spacePoint = m_spacePoints.appendNew(createSpacePoint(svdId, false, 0));   // add twoCluster SP
      m_assignedClusters.push_back(2);
      spacePoint->addRelationTo(sTH2MC1, 2); // both Clusters related to this TrueHit
      spacePoint->addRelationTo(sTH2MC2, 11); // only one Cluster related to TrueHit (U-Cluster in this case)

      // add a SVD (non-ideal case: only one Cluster with relation to ONE TrueHit: V-Cluster related)
      spacePoint = m_spacePoints.appendNew(createSpacePoint(svdId, false, 0));   // add twoCluster SP
      m_assignedClusters.push_back(2);
      spacePoint->addRelationTo(sTH2MC1, 21); // one V-Cluster with relation to TrueHit with relation to MCParticle

      // add a SVD (non-ideal case: only one Cluster with relation to ONE TrueHit: U-Cluster related)
      spacePoint = m_spacePoints.appendNew(createSpacePoint(svdId, false, 0));   // add twoCluster SP
      m_assignedClusters.push_back(2);
      spacePoint->addRelationTo(sTH2MC2, 11); // one U-Cluster with relation to TrueHit with relation to MCParticle

      // add a SVD (non-ideal case: only one TrueHit has a relation to a MCParticle)
      spacePoint = m_spacePoints.appendNew(createSpacePoint(svdId, false, 0));   // add twoCluster SP
      m_assignedClusters.push_back(2);
      spacePoint->addRelationTo(sTH2MC1, 11); // U-Cluster
      spacePoint->addRelationTo(sTH2None, 21); // V-Cluster without relation to MCParticle

      // add a SVD (non-ideal case: one Cluster has relations to more than one TrueHit, while the other has not)
      spacePoint = m_spacePoints.appendNew(createSpacePoint(svdId, false, 0)); // add twoCluster SP
      m_assignedClusters.push_back(2);
      spacePoint->addRelationTo(sTH2MC1, 11); // U-Cluster
      spacePoint->addRelationTo(sTH2MC2, 11); // U-Cluster, no relation for V-Cluster

      // add singleCluster SpacePoints for testing the increaseCounterMethod. Do not need any relations!
      // NOTE: these shall not be included in the testing of the other methods!
      m_spacePoints.appendNew(createSpacePoint(svdId, false, -1)); // add V-Cluster set only
      m_assignedClusters.push_back(1);
      m_spacePoints.appendNew(createSpacePoint(svdId, false, 1)); // add U-Cluster set only
      m_assignedClusters.push_back(1);
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
    B2INFO("Contents of DataStore after SetUp: pxdTrueHits: " << m_pxdTrueHits.getEntries() <<
           " svdTrueHits: " << m_svdTrueHits.getEntries() << " mcParticles: " << m_mcParticles.getEntries() <<
           " spacePoints: " << m_spacePoints.getEntries());

    // test if StoreArray has all the entries that should be there
    EXPECT_EQ(m_pxdTrueHits.getEntries(), 3);
    EXPECT_EQ(m_svdTrueHits.getEntries(), 3);
    EXPECT_EQ(m_spacePoints.getEntries(), 14);
    EXPECT_EQ(m_mcParticles.getEntries(), 2);

    // test if the SpacePoints have the expected number of assigned Clusters
    for (int i = 0; i < m_spacePoints.getEntries(); ++i) {
      EXPECT_EQ(m_spacePoints[i]->getNClustersAssigned(), m_assignedClusters[i]);
    }

    // test if the relations are set according to the set-up (mainly testing if the methods for setting up are working properly)
    // NOTE: only sampling here, not testing all SpacePoints! (but still tried to cover all important relations (i.e. TH -> MCPart))
    RelationVector<PXDTrueHit> pxdTrueHits = m_spacePoints[0]->getRelationsTo<PXDTrueHit>("ALL");
    ASSERT_EQ(pxdTrueHits.size(), 1);
    EXPECT_EQ(pxdTrueHits[0]->getArrayIndex(), 0);
    EXPECT_EQ(pxdTrueHits.weight(0), 1);

    pxdTrueHits = m_spacePoints[2]->getRelationsTo<PXDTrueHit>("ALL");
    ASSERT_EQ(pxdTrueHits.size(), 2);
    EXPECT_EQ(pxdTrueHits[0]->getArrayIndex(), 0);
    EXPECT_EQ(pxdTrueHits[0]->getRelatedFrom<MCParticle>("ALL")->getArrayIndex(), 0);
    EXPECT_EQ(pxdTrueHits.weight(0), 1);
    EXPECT_EQ(pxdTrueHits[1]->getArrayIndex(), 1);
    EXPECT_EQ(pxdTrueHits[1]->getRelatedFrom<MCParticle>("ALL")->getArrayIndex(), 1);
    EXPECT_EQ(pxdTrueHits.weight(1), 1);

    RelationVector<SVDTrueHit> svdTrueHits = m_spacePoints[4]->getRelationsTo<SVDTrueHit>("ALL");
    ASSERT_EQ(svdTrueHits.size(), 1);
    EXPECT_EQ(svdTrueHits[0]->getArrayIndex(), 0);
    EXPECT_EQ(svdTrueHits[0]->getRelatedFrom<MCParticle>("ALL")->getArrayIndex(), 0);
    EXPECT_EQ(svdTrueHits.weight(0), 2);

    svdTrueHits = m_spacePoints[5]->getRelationsTo<SVDTrueHit>("ALL");
    ASSERT_EQ(svdTrueHits.size(), 2);
    EXPECT_EQ(svdTrueHits[0]->getArrayIndex(), 0); // MCParticle relation already tested!
    EXPECT_EQ(svdTrueHits.weight(0), 11);
    EXPECT_EQ(svdTrueHits[1]->getArrayIndex(), 1);
    EXPECT_EQ(svdTrueHits.weight(1), 21);
    EXPECT_EQ(svdTrueHits[1]->getRelatedFrom<MCParticle>("ALL")->getArrayIndex(), 1);

    svdTrueHits = m_spacePoints[6]->getRelationsTo<SVDTrueHit>("ALL");
    ASSERT_EQ(svdTrueHits.size(), 0);

    svdTrueHits = m_spacePoints[7]->getRelationsTo<SVDTrueHit>("ALL");
    ASSERT_EQ(svdTrueHits.size(), 2);
    EXPECT_EQ(svdTrueHits[0]->getArrayIndex(), 0); // MCParticle relation already tested!
    EXPECT_EQ(svdTrueHits.weight(0), 2);
    EXPECT_EQ(svdTrueHits.weight(1), 11); // only related to U-Cluster
    EXPECT_EQ(svdTrueHits[1]->getArrayIndex(), 1); // MCParticle relation already tested!
  }

  /**
   * test if the findWeightInVector acts as advertised by its documentation
   */
  TEST_F(PurityCalculatorToolsTest, testFindWeightInVector)
  {
    std::vector<std::pair<int, double> > vec;
    vec.push_back(std::make_pair(1, 0));
    vec.push_back(std::make_pair(1, 1.1));
    vec.push_back(std::make_pair(1, 2));
    vec.push_back(std::make_pair(1, 11));
    vec.push_back(std::make_pair(1, 21));
    vec.push_back(std::make_pair(1, 11));
    vec.push_back(std::make_pair(1, 2.5));

    EXPECT_TRUE(findWeightInVector(vec, 1e-4));
    EXPECT_TRUE(findWeightInVector(vec, 1.1));
    EXPECT_FALSE(findWeightInVector(vec, 11.1));
    EXPECT_FALSE(findWeightInVector(vec, 2.501));
  }

  /**
   * test the getAccessorsFromWeight method with the values that are encountered
   */
  TEST_F(PurityCalculatorToolsTest, testGetAccessorsFromWeight)
  {
    // empty returns first
    std::vector<size_t> accs = getAccessorsFromWeight(0);
    EXPECT_TRUE(accs.empty());
    accs = getAccessorsFromWeight(30);
    EXPECT_TRUE(accs.empty());

    // cases that occur in "real life"
    accs = getAccessorsFromWeight(1);
    EXPECT_EQ(accs[0], 0); // -> PXD
    EXPECT_EQ(accs.size(), 1);
    accs = getAccessorsFromWeight(2);
    EXPECT_EQ(accs.size(), 2);
    EXPECT_EQ(accs[0], 1);
    EXPECT_EQ(accs[1], 2);
    accs = getAccessorsFromWeight(11);
    EXPECT_EQ(accs[0], 1); // -> SVD U Cluster
    EXPECT_EQ(accs.size(), 1);
    accs = getAccessorsFromWeight(21);
    EXPECT_EQ(accs[0], 2);
    EXPECT_EQ(accs.size(), 1);
  }

  /**
   * test if the increasClusterCounters increases the appropriate counters using SpacePoints that have been setup in the test
   */
  TEST_F(PurityCalculatorToolsTest, testIncreaseClusterCounters)
  {
    // create Array and test its initialization
    std::array<unsigned, 3> ctrArray = { {0, 0, 0} };
    for (size_t i = 0; i < 3; ++i) { EXPECT_EQ(ctrArray[i], 0); }

    increaseClusterCounters(m_spacePoints[0], ctrArray); // PXD
    EXPECT_EQ(ctrArray[0], 1);
    increaseClusterCounters(m_spacePoints[1], ctrArray); // PXD
    EXPECT_EQ(ctrArray[0], 2);

    increaseClusterCounters(m_spacePoints[5], ctrArray); // SVD (two Cluster)
    EXPECT_EQ(ctrArray[1], 1);
    EXPECT_EQ(ctrArray[2], 1);

    increaseClusterCounters(m_spacePoints[12], ctrArray); // SVD (only V set)
    EXPECT_EQ(ctrArray[2], 2);
    EXPECT_EQ(ctrArray[1], 1);
    increaseClusterCounters(m_spacePoints[13], ctrArray); // SVD (only U set)
    EXPECT_EQ(ctrArray[1], 2);
    EXPECT_EQ(ctrArray[2], 2);
  }

  /**
   * test the getMCParticles method. Desired outputs:
   * + MCParticleID if there is a MCParticle to a TrueHit
   * + -1, if there is no MCParticle (but a TrueHit)
   * + -2, if there is no TrueHit (but a Cluster in the SpacePoint)
   * NOTE: partly tests mock-up
   * NOTE: seperate tests for SVD and PXD for better readability
   */
  TEST_F(PurityCalculatorToolsTest, testGetMCParticlesPXD)
  {
    std::vector<std::pair<int, double> > mcParts = getMCParticles<PXDTrueHit>(m_spacePoints[0]); // ideal PXD
    ASSERT_EQ(mcParts.size(), 1);
    EXPECT_EQ(mcParts[0].first, 0); // related to MCParticle with Id 0
    EXPECT_DOUBLE_EQ(mcParts[0].second, 1);

    mcParts = getMCParticles<PXDTrueHit>(m_spacePoints[1]); // PXD with no relation to TrueHit
    ASSERT_EQ(mcParts.size(), 1);
    EXPECT_EQ(mcParts[0].first, -2); // no TrueHit to Cluster
    EXPECT_EQ(mcParts[0].second, 1);

    mcParts = getMCParticles<PXDTrueHit>(m_spacePoints[2]); // PXD with two different TrueHits (to two different MCParticle Ids
    ASSERT_EQ(mcParts.size(), 2);
    auto findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(0)); // one MCParticle with Id 0
    ASSERT_FALSE(findIt == mcParts.end());
    EXPECT_DOUBLE_EQ(findIt->second, 1);
    findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(1)); // one MCParticle with Id 1
    ASSERT_FALSE(findIt == mcParts.end());
    EXPECT_DOUBLE_EQ(findIt->second, 1);

    B2INFO("The occuring error message is expected! It is used to discover some corner cases in real usage!");
    mcParts = getMCParticles<PXDTrueHit>(m_spacePoints[3]); // PXD with two TrueHits pointing to one MCParticle
    ASSERT_EQ(mcParts.size(), 1);
    EXPECT_EQ(mcParts[0].first, 1); // related to MCParticle 2
    EXPECT_DOUBLE_EQ(mcParts[0].second, 1);
  }

  /**
   * test the getMCParticles method. Desired outputs:
   * + MCParticleID if there is a MCParticle to a TrueHit
   * + -1, if there is no MCParticle (but a TrueHit)
   * + -2, if there is no TrueHit (but a Cluster in the SpacePoint)
   * NOTE: partly tests mock-up
   * NOTE: seperate tests for SVD and PXD for better readability
   */
  TEST_F(PurityCalculatorToolsTest, testGetMCParticlesSVD)
  {
    std::vector<std::pair<int, double> > mcParts = getMCParticles<SVDTrueHit>(m_spacePoints[4]); // ideal SVD
    ASSERT_EQ(mcParts.size(), 1);
    EXPECT_EQ(mcParts[0].first, 0);
    EXPECT_DOUBLE_EQ(mcParts[0].second, 2);

    mcParts = getMCParticles<SVDTrueHit>(m_spacePoints[5]); // SVD with two THs to two different MCParticles
    ASSERT_EQ(mcParts.size(), 2);
    auto findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(0)); // one MCParticle with Id 0
    ASSERT_FALSE(findIt == mcParts.end());
    EXPECT_DOUBLE_EQ(findIt->second, 11); // U-Cluster to this MCParticle
    findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(1)); // one MCParticle with Id 1
    ASSERT_FALSE(findIt == mcParts.end());
    EXPECT_DOUBLE_EQ(findIt->second, 21); // V-Cluster to this MCParticle

    mcParts = getMCParticles<SVDTrueHit>(m_spacePoints[6]); // SVD with no TrueHit relations
    ASSERT_EQ(mcParts.size(), 1);
    EXPECT_EQ(mcParts[0].first, -2);
    EXPECT_DOUBLE_EQ(mcParts[0].second, 2);

    mcParts = getMCParticles<SVDTrueHit>(m_spacePoints[7]);
    ASSERT_EQ(mcParts.size(), 2);
    findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(1)); // one MCParticle with Id 1
    ASSERT_FALSE(findIt == mcParts.end());
    EXPECT_DOUBLE_EQ(findIt->second, 11); // only U-Cluster has connection to this MCParticle
    findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(0)); // one MCParticle with Id 0
    ASSERT_FALSE(findIt == mcParts.end());
    EXPECT_DOUBLE_EQ(findIt->second, 2); // both Clusters with relation to this MCParticle

    mcParts = getMCParticles<SVDTrueHit>(m_spacePoints[8]); // only V-Cluster has relation to TrueHit
    ASSERT_EQ(mcParts.size(), 2);
    findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(-2));
    ASSERT_FALSE(findIt == mcParts.end());
    EXPECT_DOUBLE_EQ(findIt->second, 11);
    findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(0));
    ASSERT_FALSE(findIt == mcParts.end());
    EXPECT_DOUBLE_EQ(findIt->second, 21);

    mcParts = getMCParticles<SVDTrueHit>(m_spacePoints[9]); // only U-Cluster has relation to TrueHit
    ASSERT_EQ(mcParts.size(), 2);
    findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(-2));
    ASSERT_FALSE(findIt == mcParts.end());
    EXPECT_DOUBLE_EQ(findIt->second, 21);
    findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(1));
    ASSERT_FALSE(findIt == mcParts.end());
    EXPECT_DOUBLE_EQ(findIt->second, 11);

    mcParts = getMCParticles<SVDTrueHit>(m_spacePoints[11]); // U-Cluster has relations to two TrueHits, while V-Cluster has none
    ASSERT_EQ(mcParts.size(), 3);
    findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(0));
    ASSERT_FALSE(findIt == mcParts.end());
    EXPECT_DOUBLE_EQ(findIt->second, 11);
    findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(1));
    ASSERT_FALSE(findIt == mcParts.end());
    EXPECT_DOUBLE_EQ(findIt->second, 11);
    findIt = std::find_if(mcParts.begin(), mcParts.end(), compFirst(-2));
    ASSERT_FALSE(findIt == mcParts.end());
    EXPECT_DOUBLE_EQ(findIt->second, 21);

    // NOTE: the rest of the occuring set-up cases should be covered by the above tests!
  }

  /**
   * test the createPurityInfos function by comparing the created MCVXDpurityinfo objects with the expected ones (kown
   * from mockup-construction)
   */
  TEST_F(PurityCalculatorToolsTest, testCreatePurityInfos)
  {
    // first create a SpacePointTrackCand (as container of SpacePoints) containing all the wanted hits
    std::vector<const SpacePoint*> spacePoints;
    for (size_t i = 0; i < 12; ++i) { spacePoints.push_back(m_spacePoints[i]); } // do not use the last two SpacePoints from the setup
    SpacePointTrackCand sptc(spacePoints);
    EXPECT_EQ(sptc.getNHits(), 12); // check if the creation worked
    unsigned totCls = 20;
    float ndf = 4 * 2 + 8 * 2; // 4 PXD SpacePoints, and 8 two Cluster SVD-SpacePoints

    B2INFO("There will be WARNING and ERROR messages! Those are expected!");
    std::vector<MCVXDPurityInfo> purities = createPurityInfos(sptc); // create the purityInfos

    EXPECT_EQ(purities.size(), 4); // 4 different Particle Ids: 0, 1, -1, -2

    // check if the vector is sorted from highest to lowest overall purity
    for (size_t i = 0; i < purities.size() - 1; ++i) {
      EXPECT_TRUE(purities[i].getPurity().second >= purities[i + 1].getPurity().second);
    }

    auto findIt = find_if(purities.begin(), purities.end(), compMCId(1)); // get MCParticle with Id 1
    ASSERT_FALSE(findIt == purities.end());
    EXPECT_EQ(findIt->getNClustersTotal(), totCls);
    EXPECT_EQ(findIt->getNPXDClustersTotal(), 4); // 4 PXD Clusters were in container
    EXPECT_EQ(findIt->getNClustersFound(), 6);
    EXPECT_EQ(findIt->getNPXDClusters(), 2);
    EXPECT_EQ(findIt->getNSVDUClusters(), 3);
    EXPECT_EQ(findIt->getNSVDVClusters(), 1);
    EXPECT_FLOAT_EQ(findIt->getPurity().second, 8. / ndf);

    findIt = find_if(purities.begin(), purities.end(), compMCId(0)); // get Id 0
    ASSERT_FALSE(findIt == purities.end());
    EXPECT_EQ(findIt->getNClustersTotal(), totCls);
    EXPECT_EQ(findIt->getNSVDUClustersTotal(), 8); // 8 SVD U Clusters are in container
    EXPECT_EQ(findIt->getNClustersFound(), 10);
    EXPECT_EQ(findIt->getNPXDClusters(), 2);
    EXPECT_EQ(findIt->getNSVDUClusters(), 5);
    EXPECT_EQ(findIt->getNSVDVClusters(), 3);
    EXPECT_FLOAT_EQ(findIt->getPurity().second, 12. / ndf);

    findIt = find_if(purities.begin(), purities.end(), compMCId(-1));
    ASSERT_FALSE(findIt == purities.end());
    EXPECT_EQ(findIt->getNClustersTotal(), totCls);
    EXPECT_EQ(findIt->getNSVDVClustersTotal(), 8); // 8 SVD U Clusters are in container
    EXPECT_EQ(findIt->getNClustersFound(), 1);
    EXPECT_EQ(findIt->getNPXDClusters(), 0);
    EXPECT_EQ(findIt->getNSVDUClusters(), 0);
    EXPECT_EQ(findIt->getNSVDVClusters(), 1);
    EXPECT_FLOAT_EQ(findIt->getPurity().second, 1. / ndf);

    findIt = find_if(purities.begin(), purities.end(), compMCId(-2));
    ASSERT_FALSE(findIt == purities.end());
    EXPECT_EQ(findIt->getNClustersTotal(), totCls);
    EXPECT_EQ(findIt->getNClustersFound(), 6);
    EXPECT_EQ(findIt->getNPXDClusters(), 1);
    EXPECT_EQ(findIt->getNSVDUClusters(), 2);
    EXPECT_EQ(findIt->getNSVDVClusters(), 3);
    EXPECT_FLOAT_EQ(findIt->getPurity().second, 7. / ndf);
  }

} // namespace
