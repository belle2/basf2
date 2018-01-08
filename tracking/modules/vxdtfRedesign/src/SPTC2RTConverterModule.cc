/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/vxdtfRedesign/SPTC2RTConverterModule.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

REG_MODULE(SPTC2RTConverter)

SPTC2RTConverterModule::SPTC2RTConverterModule() : Module()
{
  setDescription("Converts the given SpacePointTrackCandidates to RecoTracks and stores them in the given RecoTracksStoreArray");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("spacePointsTCsStoreArrayName", m_param_spacePointTCsStoreArrayName,
           "StoreArray name of the input SpacePointTrackCandidates.", std::string(""));
  addParam("recoTracksStoreArrayName", m_param_recoTracksStoreArrayName,
           "StoreArray name of the output RecoTracks.", std::string(""));

  addParam("recoHitInformationStoreArrayName", m_param_recoHitInformationStoreArrayName,
           "StoreArray name of the output RecoHitInformation.", std::string(""));
  addParam("pxdHitsStoreArrayName", m_param_pxdHitsStoreArrayName, "StoreArray name of the related pxd hits.",
           std::string(""));
  addParam("svdHitsStoreArrayName", m_param_svdHitsStoreArrayName, "StoreArray name of the related svd hits.",
           std::string(""));

  addParam("pxdClustersName", m_param_pxdClustersName, "StoreArray name of PXDClusters related to SpacePoints.",
           std::string(""));
  addParam("svdClustersName", m_param_svdClustersName, "StoreArray name of SVDClusters related to SpacePoints.",
           std::string(""));

  initializeCounters();
}

void SPTC2RTConverterModule::initialize()
{
  // Reset Counters
  initializeCounters();

  // Read in SpacePointTrackCandidates
  m_spacePointTCs = StoreArray<SpacePointTrackCand>(m_param_spacePointTCsStoreArrayName);
  m_spacePointTCs.isRequired();

  // Write out RecoTracks
  m_recoTracks = StoreArray<RecoTrack>(m_param_recoTracksStoreArrayName);
  m_recoTracks.registerInDataStore();
  RecoTrack::registerRequiredRelations(m_recoTracks,
                                       "",
                                       m_param_svdHitsStoreArrayName,
                                       m_param_pxdHitsStoreArrayName,
                                       "",
                                       "",
                                       m_param_recoHitInformationStoreArrayName);
  m_recoTracks.registerRelationTo(m_spacePointTCs);

  StoreArray<PXDCluster>::optional(m_param_pxdClustersName);
  StoreArray<SVDCluster>::optional(m_param_svdClustersName);

}

void SPTC2RTConverterModule::event()
{

  for (const SpacePointTrackCand& spacePointTC : m_spacePointTCs) {
    m_SPTCCtr++;
    if (spacePointTC.getRefereeStatus() < SpacePointTrackCand::c_isActive) {
      B2DEBUG(100, "SPTC2RTConverter::event: SpacePointTrackCandidate not active or reserved. RefereeStatus: " <<
              spacePointTC.getRefereeStatus());
      continue; // Ignore SpacePointTrackCandidate
    } else if (spacePointTC.getNHits() < 3) {
      B2WARNING("SPTC2RTConverter::event: Number of SpacePoints of track candidate is smaller than 3. Not creating RecoTrack out of it.");
      continue; // Ignore SpacePointTrackCandidate
    }
    createRecoTrack(spacePointTC);
    m_RTCtr++;
  }
}

void SPTC2RTConverterModule::createRecoTrack(const SpacePointTrackCand& spacePointTC)
{

  // Determine the tracking parameters
  const TVector3& position = spacePointTC.getPosSeed();
  const TVector3& momentum = spacePointTC.getMomSeed();
  const short int charge = spacePointTC.getChargeSeed();
  const TMatrixDSym& covSeed = spacePointTC.getCovSeed();


  // Create and append new RecoTrack
  RecoTrack* newRecoTrack = m_recoTracks.appendNew(position, momentum, charge,
                                                   "", m_param_svdHitsStoreArrayName,
                                                   m_param_pxdHitsStoreArrayName, "", "", m_param_recoHitInformationStoreArrayName);

  // Set information not required by constructor
  newRecoTrack->setSeedCovariance(covSeed);

  // Add individual Hits/Clusters
  unsigned int sortingParameter = 0; // Recreate sorting since there are two cluster per SVD hit.
  for (auto spacePoint : spacePointTC.getSortedHits()) {
    B2DEBUG(110, "SPTC2RTConverter::event: Converting spacepoint: " << spacePoint->getArrayIndex());

    int detID = spacePoint->getType();

    if (detID == VXD::SensorInfoBase::PXD) {
      RelationVector<PXDCluster> relatedClusters = spacePoint->getRelationsTo<PXDCluster>(m_param_pxdClustersName);
      B2DEBUG(110, "SPTC2RTConverter::event: Number of related PXD Clusters: " << relatedClusters.size());
      // relatedClusters should only contain 1 cluster for pxdHits. Loop over them to be robust against missing relations.
      for (const PXDCluster& cluster : relatedClusters) {
        newRecoTrack->addPXDHit(&cluster, sortingParameter, Belle2::RecoHitInformation::c_VXDTrackFinder);
        sortingParameter++;
      }
    } else if (detID == VXD::SensorInfoBase::SVD) {
      RelationVector<SVDCluster> relatedClusters = spacePoint->getRelationsTo<SVDCluster>(m_param_svdClustersName);
      B2DEBUG(110, "SPTC2RTConverter::event: Number of related SVD Clusters: " << relatedClusters.size());
      // relatedClusters should contain 2 clusters for svdHits. Loop over them to be robust against missing relations.
      for (const SVDCluster& cluster : relatedClusters) {
        newRecoTrack->addSVDHit(&cluster, sortingParameter, Belle2::RecoHitInformation::c_VXDTrackFinder);
        sortingParameter++;
      }
    } else {
      B2WARNING("SPTC2RTConverter::event: SpacePointTrackCandidate containing SpacePoint of unrecognised detector ID: " << detID <<
                ". Created RecoTrack doesn't contain these SpacePoints!");
    }

    spacePoint->setAssignmentState(true);
  }

  // Add relation to SpacePointTrackCandidate
  newRecoTrack->addRelationTo(&spacePointTC);

  B2DEBUG(110, "SPTC2RTConverter::event: nHits: " << spacePointTC.getNHits() <<
          " ChargeSeed: " << charge <<
          " PositionSeed: " << position.X() << ", " << position.Y() << ", " << position.Z() <<
          " MomentumSeed: " << momentum.X() << ", " << momentum.Y() << ", " << momentum.Z());
}

void SPTC2RTConverterModule::terminate()
{
  B2DEBUG(100, "SPTC2RTConverter::terminate: Got " << m_SPTCCtr << " SpacePointTrackCands and created " << m_RTCtr << " RecoTracks";);
}

void SPTC2RTConverterModule::initializeCounters()
{
  m_SPTCCtr = 0;
  m_RTCtr = 0;
}
