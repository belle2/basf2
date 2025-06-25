/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * *
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

namespace {

  const Belle2::VxdID defaultVxdID = Belle2::VxdID("1.1.1");
  const bool defaultIsU = true;
  const double defaultPosition = 1.0;
  const double defaultPositionErr = 0.01;
  const double defaultClsTime = 20.0;
  const double defaultClsTimeErr = 5.0;
  const double defaultClsCharge = 23e3;
  const double defaultSeedCharge = 5e3;
  const int defaultClsSize = 2;
  const double defaultClsSNR = 25.0;
  const double defaultClsChi2 = 1.0;
  const int defaultFirstFrame = 0;

  auto dummySVDCluster = []() -> Belle2::SVDCluster
  {
    return Belle2::SVDCluster(
             defaultVxdID, defaultIsU, defaultPosition, defaultPositionErr,
             defaultClsTime, defaultClsTimeErr, defaultClsCharge, defaultSeedCharge,
             defaultClsSize, defaultClsSNR, defaultClsChi2, defaultFirstFrame
           );
  }();
}

namespace Belle2::SVD {

  class SVDVariableTest : public ::testing::Test {
  public:
    void SetUp() override
    {
      std::string storeArrayNameOfParticles = "Particles";
      std::string storeArrayNameOfTracks = "Tracks";
      std::string storeArrayNameOfRecoTracks = "RecoTracks";
      std::string storeArrayNameOfPXDHits = "PXDClusters";
      std::string storeArrayNameOfSVDHits = "SVDClusters";
      std::string storeArrayNameOfCDCHits = "CDCClusters";
      std::string storeArrayNameOfBKLMHits = "BKLMClusters";
      std::string storeArrayNameOfEKLMHits = "EKLMClusters";
      std::string storeArrayNameOfRecoHitInformations = "RecoHitInformations";
      std::string storeArrayNameOfTrackFitResults = "TrackFitResults";

      DataStore::Instance().setInitializeActive(true);
      m_particles = StoreArray<Particle>(storeArrayNameOfParticles);
      m_tracks = StoreArray<Track>(storeArrayNameOfTracks);
      m_recoTracks = StoreArray<RecoTrack>(storeArrayNameOfRecoTracks);
      m_svdClusters = StoreArray<SVDCluster>(storeArrayNameOfSVDHits);
      m_recoHitInformations = StoreArray<RecoHitInformation>(storeArrayNameOfRecoHitInformations);
      m_trackFitResults = StoreArray<TrackFitResult>(storeArrayNameOfTrackFitResults);

      m_particles.registerInDataStore();
      m_tracks.registerInDataStore();
      m_recoTracks.registerInDataStore();
      m_svdClusters.registerInDataStore();
      m_recoHitInformations.registerInDataStore();
      m_trackFitResults.registerInDataStore();

      m_tracks.registerRelationTo(m_recoTracks);
      m_recoTracks.registerRelationTo(m_recoHitInformations);
      m_recoHitInformations.registerRelationTo(m_svdClusters);
      m_svdClusters.registerRelationTo(m_recoTracks);

      m_trackFitResults.appendNew(ROOT::Math::XYZVector(0.1, 0.1, 0.1), ROOT::Math::XYZVector(0.1, 0.0, 0.0),
                                  TMatrixDSym(6), 1, Const::pion, 0.01, 1.5, 0, 0, 0);

      m_tracks.appendNew(Track());
      m_tracks[0]->setTrackFitResultIndex(Const::pion, 0);

      m_particles.appendNew(Particle(m_tracks[0], Const::pion));

      m_recoTracks.appendNew(ROOT::Math::XYZVector(0.1, 0.1, 0.1), ROOT::Math::XYZVector(0.1, 0.0, 0.0), 1,
                             storeArrayNameOfPXDHits, storeArrayNameOfSVDHits, storeArrayNameOfCDCHits,
                             storeArrayNameOfBKLMHits, storeArrayNameOfEKLMHits, storeArrayNameOfRecoHitInformations);

      m_tracks[0]->addRelationTo(m_recoTracks[0]);
      m_svdClusters.appendNew(dummySVDCluster);
      m_recoTracks[0]->addSVDHit(m_svdClusters[0], 0);
    }

    StoreArray<Particle> m_particles;
    StoreArray<Track> m_tracks;
    StoreArray<RecoTrack> m_recoTracks;
    StoreArray<SVDCluster> m_svdClusters;
    StoreArray<TrackFitResult> m_trackFitResults;
    StoreArray<RecoHitInformation> m_recoHitInformations;
  };

  struct SVDVariableParam {
    std::function<Belle2::Variable::Manager::FunctionPtr(const std::vector<std::string>&)> variableFunction;
    std::variant<double, int, bool> expectedValue;
    std::string testName;
  };

  class SVDParameterizedVariableTest : public SVDVariableTest,
    public ::testing::WithParamInterface<SVDVariableParam> {
  };

  TEST_P(SVDParameterizedVariableTest, CheckDefaultValues)
  {
    const auto& param = GetParam();

    auto varFunc = param.variableFunction({"0"});
    auto resultVariant = varFunc(m_particles[0]);

    std::visit([&resultVariant](auto&& expected) {
      using T = std::decay_t<decltype(expected)>;
      try {
        const T& actual = std::get<T>(resultVariant);
        if constexpr(std::is_same_v<T, double>) {
          EXPECT_DOUBLE_EQ(actual, expected);
        } else {
          EXPECT_EQ(actual, expected);
        }
      } catch (const std::bad_variant_access& ex) {
        FAIL() << "Mismatched variant types in test. Expected " << typeid(T).name()
               << " but got another type from the variable function.";
      }
    }, param.expectedValue);
  }

  INSTANTIATE_TEST_SUITE_P(
    SVDVariableTests,
    SVDParameterizedVariableTest,
    ::testing::Values(
      SVDVariableParam{Belle2::Variable::SVDClusterCharge, defaultClsCharge, "SVDClusterCharge"},
      SVDVariableParam{Belle2::Variable::SVDClusterSNR, defaultClsSNR, "SVDClusterSNR"},
      SVDVariableParam{Belle2::Variable::SVDClusterSize, defaultClsSize, "SVDClusterSize"},
      SVDVariableParam{Belle2::Variable::SVDClusterTime, defaultClsTime, "SVDClusterTime"}
    ),
    [](const ::testing::TestParamInfo<SVDVariableParam>& paramInfo)
  {
    return paramInfo.param.testName;
  }
  );
} // namespace Belle2::SVD