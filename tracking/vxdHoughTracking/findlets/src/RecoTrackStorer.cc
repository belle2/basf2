/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/vxdHoughTracking/findlets/RecoTrackStorer.h>
#include <framework/core/ModuleParamList.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/geometry/BFieldManager.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <svd/dataobjects/SVDCluster.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorMC.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace vxdHoughTracking;

RecoTrackStorer::~RecoTrackStorer() = default;

RecoTrackStorer::RecoTrackStorer() : Super()
{
}

void RecoTrackStorer::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "SVDClustersStoreArrayName"), m_SVDClustersStoreArrayName,
                                "Name of the SVDClusters Store Array.", m_SVDClustersStoreArrayName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "RecoTracksStoreArrayName"), m_RecoTracksStoreArrayName,
                                "Name of the RecoTracks Store Array.", m_RecoTracksStoreArrayName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "SVDSpacePointTrackCandsStoreArrayName"),
                                m_SVDSpacePointTrackCandsStoreArrayName,
                                "Name of the SpacePointTrackCand Store Array.", m_SVDSpacePointTrackCandsStoreArrayName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "ResultStorerQualityEstimationMethod"), m_EstimationMethod,
                                "Identifier which estimation method to use. Valid identifiers are: [mcInfo, circleFit, tripletFit, helixFit].",
                                m_EstimationMethod);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "ResultStorerMCRecoTracksStoreArrayName"),
                                m_MCRecoTracksStoreArrayName,
                                "Only required for MCInfo method. Name of StoreArray containing MCRecoTracks.",
                                m_MCRecoTracksStoreArrayName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "ResultStorerMCStrictQualityEstimator"),
                                m_MCStrictQualityEstimator,
                                "Only required for MCInfo method. If false combining several MCTracks is allowed.",
                                m_MCStrictQualityEstimator);
}

void RecoTrackStorer::initialize()
{
  Super::initialize();

  m_storeRecoTracks.registerInDataStore(m_RecoTracksStoreArrayName);
  RecoTrack::registerRequiredRelations(m_storeRecoTracks, "", m_SVDClustersStoreArrayName, "", "", "", "");
  m_storeSpacePointTrackCands.registerInDataStore(m_SVDSpacePointTrackCandsStoreArrayName,
                                                  DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
  m_storeRecoTracks.registerRelationTo(m_storeSpacePointTrackCands);

  // create pointer to chosen estimator
  if (m_EstimationMethod == "mcInfo") {
    StoreArray<RecoTrack> mcRecoTracks;
    mcRecoTracks.isRequired(m_MCRecoTracksStoreArrayName);
    m_estimator = std::make_unique<QualityEstimatorMC>(m_MCRecoTracksStoreArrayName, m_MCStrictQualityEstimator);
  } else if (m_EstimationMethod == "tripletFit") {
    m_estimator = std::make_unique<QualityEstimatorTripletFit>();
  } else if (m_EstimationMethod == "circleFit") {
    m_estimator = std::make_unique<QualityEstimatorCircleFit>();
  } else if (m_EstimationMethod == "helixFit") {
    m_estimator = std::make_unique<QualityEstimatorRiemannHelixFit>();
  }
  B2ASSERT("QualityEstimator could not be initialized with method: " << m_EstimationMethod, m_estimator);
}

void RecoTrackStorer::beginRun()
{
  Super::beginRun();

  // BField is required by all QualityEstimators
  double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  m_estimator->setMagneticFieldStrength(bFieldZ);

  if (m_EstimationMethod == "mcInfo") {
    QualityEstimatorMC* MCestimator = static_cast<QualityEstimatorMC*>(m_estimator.get());
    MCestimator->forceUpdateClusterNames();
  }
}

void RecoTrackStorer::beginEvent()
{
  Super::beginEvent();

  m_usedClusters.clear();
  m_usedSpacePoints.clear();
}

void RecoTrackStorer::apply(std::vector<SpacePointTrackCand>& finishedResults,
                            const std::vector<const SpacePoint*>& spacePoints)
{
  for (auto& thisSPTC : finishedResults) {
    // do nothing if the SpacePointTrackCand is not active
    if (!thisSPTC.hasRefereeStatus(SpacePointTrackCand::c_isActive)) {
      continue;
    }

    auto sortedHits = thisSPTC.getSortedHits();
    const auto& estimatorResult = m_estimator->estimateQualityAndProperties(sortedHits);

    const ROOT::Math::XYZVector trackPosition(sortedHits.front()->X(), sortedHits.front()->Y(), sortedHits.front()->Z());
    const ROOT::Math::XYZVector& trackMomentum = *estimatorResult.p;
    const short& trackChargeSeed = estimatorResult.curvatureSign ? -1 * (*(estimatorResult.curvatureSign)) : 0;
    const double qi = estimatorResult.qualityIndicator;

    RecoTrack* newRecoTrack = m_storeRecoTracks.appendNew(trackPosition, trackMomentum, trackChargeSeed, "",
                                                          m_SVDClustersStoreArrayName);

    // TODO: find out where these numbers come from!
    // This is copied from the VXDTF2 counterpart tracking/modules/spacePointCreator/SPTCmomentumSeedRetrieverModule !
    TVectorD stateSeed(6); //(x,y,z,px,py,pz)
    TMatrixDSym covSeed(6);
    covSeed(0, 0) = 0.01 ; covSeed(1, 1) = 0.01 ; covSeed(2, 2) = 0.04 ; // 0.01 = 0.1^2 = dx*dx =dy*dy. 0.04 = 0.2^2 = dz*dz
    covSeed(3, 3) = 0.01 ; covSeed(4, 4) = 0.01 ; covSeed(5, 5) = 0.04 ;

    stateSeed(0) = (sortedHits.front()->X());
    stateSeed(1) = (sortedHits.front()->Y());
    stateSeed(2) = (sortedHits.front()->Z());
    if (estimatorResult.p) {
      auto momentumSeed = *(estimatorResult.p);
      stateSeed(3) = momentumSeed.X();
      stateSeed(4) = momentumSeed.Y();
      stateSeed(5) = momentumSeed.Z();
    } else {
      stateSeed(3) = 0;
      stateSeed(4) = 0;
      stateSeed(5) = 0;
    }

    thisSPTC.set6DSeed(stateSeed);
    thisSPTC.setCovSeed(covSeed);
    thisSPTC.setChargeSeed(trackChargeSeed);
    // until here

    // Set information not required by constructor
    newRecoTrack->setSeedCovariance(covSeed);

    // Transfer quality indicator from SPTC to RecoTrack
    newRecoTrack->setQualityIndicator(qi);

    SpacePointTrackCand* newSPTC = m_storeSpacePointTrackCands.appendNew(thisSPTC);
    // Add relation to SpacePointTrackCandidate
    newRecoTrack->addRelationTo(newSPTC);


    unsigned int sortingParameter = 0;
    for (const SpacePoint* spacePoint : sortedHits) {
      m_usedSpacePoints.insert(spacePoint);

      RelationVector<SVDCluster> relatedClusters = spacePoint->getRelationsTo<SVDCluster>(m_SVDClustersStoreArrayName);
      for (const SVDCluster& relatedCluster : relatedClusters) {
        m_usedClusters.insert(&relatedCluster);
        newRecoTrack->addSVDHit(&relatedCluster, sortingParameter, Belle2::RecoHitInformation::c_SVDHough);
        sortingParameter++;
      }
    }
  }

  for (const SpacePoint* spacePoint : spacePoints) {
    if (TrackFindingCDC::is_in(spacePoint, m_usedSpacePoints)) {
      spacePoint->setAssignmentState(true);
      continue;
    }

    const auto& relatedClusters = spacePoint->getRelationsTo<SVDCluster>(m_SVDClustersStoreArrayName);
    for (const SVDCluster& relatedCluster : relatedClusters) {
      if (TrackFindingCDC::is_in(&relatedCluster, m_usedClusters)) {
        spacePoint->setAssignmentState(true);
        break;
      }
    }
  }

}
