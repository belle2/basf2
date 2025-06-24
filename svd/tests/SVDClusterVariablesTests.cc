/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <variant>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <vxd/dataobjects/VxdID.h>
#include <genfit/TrackPoint.h>
#include <genfit/AbsFitterInfo.h>

#include <svd/variables/SVDClusterVariables.h>

using namespace Belle2;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::_;

class MockParticle : public Particle {
public:
  MOCK_CONST_METHOD0(getTrack, const Track * ());
};

class MockTrack : public Track {
public:
  template <class T>
  T* getRelatedTo() const
  {
    if constexpr(std::is_same_v<T, RecoTrack>) {
      return getRelatedToRecoTrack();
    }
    return nullptr;
  }
  MOCK_CONST_METHOD0(getRelatedToRecoTrack, RecoTrack * ());
};

class MockSVDCluster : public SVDCluster {
public:
  MOCK_CONST_METHOD0(getCharge, float());
  MOCK_CONST_METHOD0(getSNR, float());
  MOCK_CONST_METHOD0(getSize, unsigned int());
  MOCK_CONST_METHOD0(getClsTime, double());
  MOCK_CONST_METHOD0(getSensorID, VxdID());
  MOCK_CONST_METHOD0(isUCluster, bool());

  template <class T>
  T* getRelatedTo() const
  {
    if constexpr(std::is_same_v<T, SVDTrueHit>) {
      return getRelatedToSVDTrueHit();
    }
    return nullptr;
  }
  MOCK_CONST_METHOD0(getRelatedToSVDTrueHit, SVDTrueHit * ());
};

class MockRecoTrack : public RecoTrack {
public:
  MOCK_CONST_METHOD0(getSVDHitList, const std::vector<SVDCluster*>& ());
  MOCK_CONST_METHOD1(getRecoHitInformation, const RecoHitInformation * (const SVDCluster*));
  MOCK_CONST_METHOD1(getCreatedTrackPoint, genfit::TrackPoint * (const RecoHitInformation*));
  MOCK_CONST_METHOD1(getMeasuredStateOnPlaneFromRecoHit, genfit::MeasuredStateOnPlane(const RecoHitInformation*));
};

class MockSVDTrueHit : public SVDTrueHit {
public:
  MOCK_CONST_METHOD0(getU, double());
  MOCK_CONST_METHOD0(getV, double());
};

class MockAbsMeasurement : public genfit::AbsMeasurement {
public:
  // Default constructor if needed for some cases, but generally provide valid data
  MockAbsMeasurement() : genfit::AbsMeasurement(0) {} // Pass a dummy ID

  // Mock all pure virtual methods
  MOCK_CONST_METHOD0(clone, genfit::AbsMeasurement * ());
  MOCK_CONST_METHOD0(getDetId, int());
  MOCK_CONST_METHOD0(getMeasurementId, int());
  MOCK_CONST_METHOD0(getRawHitCoords, TVectorD());
  MOCK_CONST_METHOD0(getRawHitCov, TMatrixDSym());
  MOCK_CONST_METHOD0(getHMatrix, TMatrixDSym());
  MOCK_CONST_METHOD0(getDimension, int());
};

class MockTrackPoint : public genfit::TrackPoint {
public:
  MockTrackPoint() : genfit::TrackPoint(nullptr, nullptr) {}
  MOCK_CONST_METHOD0(getFitterInfo, genfit::AbsFitterInfo * ());
};

class MockAbsFitterInfo : public genfit::AbsFitterInfo {
public:
  MOCK_CONST_METHOD1(getFittedState, const genfit::MeasuredStateOnPlane & (bool));
  MOCK_CONST_METHOD3(getResidual, genfit::MeasurementOnPlane(unsigned int, bool, bool));

  MOCK_CONST_METHOD0(clone, genfit::AbsFitterInfo * ());
  MOCK_CONST_METHOD0(hasMeasurements, bool());
  MOCK_CONST_METHOD0(hasReferenceState, bool());
  MOCK_CONST_METHOD0(hasForwardPrediction, bool());
  MOCK_CONST_METHOD0(hasBackwardPrediction, bool());
  MOCK_CONST_METHOD0(hasForwardUpdate, bool());
  MOCK_CONST_METHOD0(hasBackwardUpdate, bool());
  MOCK_METHOD0(deleteForwardInfo, void());
  MOCK_METHOD0(deleteBackwardInfo, void());
  MOCK_METHOD0(deleteReferenceInfo, void());
  MOCK_METHOD0(deleteMeasurementInfo, void());
  MOCK_CONST_METHOD1(checkConsistency, bool(const genfit::PruneFlags*));
};

class SVDVariableTest : public ::testing::Test {
public:
  std::unique_ptr<MockParticle> m_mockParticle;
  std::unique_ptr<MockTrack> m_mockTrack;
  std::unique_ptr<MockRecoTrack> m_mockRecoTrack;
  std::unique_ptr<MockSVDCluster> m_mockSVDCluster;
  std::unique_ptr<MockSVDTrueHit> m_mockSVDTrueHit;
  std::unique_ptr<MockTrackPoint> m_mockTrackPoint;
  std::unique_ptr<MockAbsFitterInfo> m_mockAbsFitterInfo;


  std::vector<SVDCluster*> m_svdClusters;

  void SetUp() override
  {
    m_mockParticle = std::make_unique<MockParticle>();
    m_mockTrack = std::make_unique<MockTrack>();
    m_mockRecoTrack = std::make_unique<MockRecoTrack>();
    m_mockSVDCluster = std::make_unique<MockSVDCluster>();
    m_mockSVDTrueHit = std::make_unique<MockSVDTrueHit>();
    m_mockTrackPoint = std::make_unique<MockTrackPoint>();
    m_mockAbsFitterInfo = std::make_unique<MockAbsFitterInfo>();

    m_svdClusters.push_back(m_mockSVDCluster.get());

    // Wire up the mock expectations
    ON_CALL(*m_mockParticle, getTrack()).WillByDefault(Return(m_mockTrack.get()));
    ON_CALL(*m_mockTrack, getRelatedToRecoTrack()).WillByDefault(Return(m_mockRecoTrack.get()));
    ON_CALL(*m_mockRecoTrack, getSVDHitList()).WillByDefault(ReturnRef(m_svdClusters));
    ON_CALL(*m_mockSVDCluster, getRelatedToSVDTrueHit()).WillByDefault(Return(m_mockSVDTrueHit.get()));

    ON_CALL(*m_mockRecoTrack, getRecoHitInformation(_)).WillByDefault(Return(reinterpret_cast<const RecoHitInformation*>(0x1)));
    ON_CALL(*m_mockRecoTrack, getCreatedTrackPoint(_)).WillByDefault(Return(m_mockTrackPoint.get()));
    ON_CALL(*m_mockTrackPoint, getFitterInfo()).WillByDefault(Return(m_mockAbsFitterInfo.get()));
  }

  void TearDown() override
  {
    m_svdClusters.clear();
  }
};

TEST_F(SVDVariableTest, SVDClusterCharge)
{
  EXPECT_CALL(*m_mockSVDCluster, getCharge()).WillOnce(Return(23000.0f));

  auto varFunc = Belle2::Variable::SVDClusterCharge({"0"});
  auto result = varFunc(m_mockParticle.get());

  EXPECT_DOUBLE_EQ(std::get<double>(result), 23000.0);
}

TEST_F(SVDVariableTest, SVDClusterSNR)
{
  EXPECT_CALL(*m_mockSVDCluster, getSNR()).WillOnce(Return(42.5f));

  auto varFunc = Belle2::Variable::SVDClusterSNR({"0"});
  auto result = varFunc(m_mockParticle.get());

  EXPECT_DOUBLE_EQ(std::get<double>(result), 42.5);
}

TEST_F(SVDVariableTest, SVDClusterSize)
{
  EXPECT_CALL(*m_mockSVDCluster, getSize()).WillOnce(Return(3));

  auto varFunc = Belle2::Variable::SVDClusterSize({"0"});
  auto result = varFunc(m_mockParticle.get());

  EXPECT_EQ(std::get<int>(result), 3);
}

TEST_F(SVDVariableTest, SVDClusterTime)
{
  EXPECT_CALL(*m_mockSVDCluster, getClsTime()).WillOnce(Return(12.34));

  auto varFunc = Belle2::Variable::SVDClusterTime({"0"});
  auto result = varFunc(m_mockParticle.get());

  EXPECT_DOUBLE_EQ(std::get<double>(result), 12.34);
}

TEST_F(SVDVariableTest, SVDLayer)
{
  VxdID dummyVxdId(4, 5, 6);
  EXPECT_CALL(*m_mockSVDCluster, getSensorID()).WillOnce(Return(dummyVxdId));

  auto varFunc = Belle2::Variable::SVDLayer({"0"});
  auto result = varFunc(m_mockParticle.get());

  EXPECT_EQ(std::get<int>(result), 4);
}

TEST_F(SVDVariableTest, SVDLadder)
{
  VxdID dummyVxdId(4, 5, 6);
  EXPECT_CALL(*m_mockSVDCluster, getSensorID()).WillOnce(Return(dummyVxdId));

  auto varFunc = Belle2::Variable::SVDLadder({"0"});
  auto result = varFunc(m_mockParticle.get());

  EXPECT_EQ(std::get<int>(result), 5);
}

TEST_F(SVDVariableTest, SVDSensor)
{
  VxdID dummyVxdId(4, 5, 6);
  EXPECT_CALL(*m_mockSVDCluster, getSensorID()).WillOnce(Return(dummyVxdId));

  auto varFunc = Belle2::Variable::SVDSensor({"0"});
  auto result = varFunc(m_mockParticle.get());

  EXPECT_EQ(std::get<int>(result), 6);
}

TEST_F(SVDVariableTest, SVDSide)
{
  EXPECT_CALL(*m_mockSVDCluster, isUCluster()).WillOnce(Return(true));

  auto varFunc = Belle2::Variable::SVDSide({"0"});
  auto result = varFunc(m_mockParticle.get());

  EXPECT_TRUE(std::get<bool>(result));
}

TEST_F(SVDVariableTest, SVDTruePosition)
{
  EXPECT_CALL(*m_mockSVDCluster, isUCluster()).WillOnce(Return(true));
  EXPECT_CALL(*m_mockSVDTrueHit, getU()).WillOnce(Return(1.23));

  auto varFunc = Belle2::Variable::SVDTruePosition({"0"});
  auto result = varFunc(m_mockParticle.get());

  EXPECT_DOUBLE_EQ(std::get<double>(result), 1.23);
}

TEST_F(SVDVariableTest, SVDTrackPrime)
{
  genfit::MeasuredStateOnPlane state;
  TVectorD stateVec(6);
  stateVec[1] = 0.5;
  state.setState(stateVec);

  EXPECT_CALL(*m_mockSVDCluster, isUCluster()).WillOnce(Return(true));
  EXPECT_CALL(*m_mockRecoTrack, getMeasuredStateOnPlaneFromRecoHit(_)).WillOnce(Return(state));

  auto varFunc = Belle2::Variable::SVDTrackPrime({"0"});
  auto result = varFunc(m_mockParticle.get());

  EXPECT_DOUBLE_EQ(std::get<double>(result), 0.5);
}

TEST_F(SVDVariableTest, SVDTrackPositionErrorUnbiasedForUCluster)
{
  const double uPrimeVariance = 0.0025;
  const double uPrimeError = std::sqrt(uPrimeVariance);

  genfit::MeasuredStateOnPlane unbiasedState;
  TMatrixDSym cov(6);
  cov(3, 3) = uPrimeVariance;
  unbiasedState.setCov(cov);

  EXPECT_CALL(*m_mockSVDCluster, isUCluster()).WillOnce(Return(true));
  EXPECT_CALL(*m_mockAbsFitterInfo, getFittedState(false)).WillOnce(ReturnRef(unbiasedState));

  auto varFunc = Belle2::Variable::SVDTrackPositionErrorUnbiased({"0"});
  auto result = varFunc(m_mockParticle.get());

  EXPECT_DOUBLE_EQ(std::get<double>(result), uPrimeError);
}

TEST_F(SVDVariableTest, SVDResidual)
{
  const double expectedResidual_cm = -0.0015;

  genfit::MeasurementOnPlane measurement;
  TVectorD residualVec(1);
  residualVec[0] = expectedResidual_cm;
  measurement.setState(residualVec);

  EXPECT_CALL(*m_mockAbsFitterInfo, getResidual(_, false, _)).WillOnce(Return(measurement));

  auto varFunc = Belle2::Variable::SVDResidual({"0"});
  auto result = varFunc(m_mockParticle.get());
  const double expectedResidual_um = expectedResidual_cm * 1e4;
  EXPECT_DOUBLE_EQ(std::get<double>(result), expectedResidual_um);
}