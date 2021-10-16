/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/dqmUtils/DQMEventProcessorBase.h>
#include <tracking/dqmUtils/DQMHistoModuleBase.h>

#include <framework/datastore/StoreArray.h>
#include <vxd/geometry/GeoTools.h>
#include <vxd/geometry/SensorInfoBase.h>

using namespace std;
using namespace Belle2;

void DQMEventProcessorBase::Run()
{
  StoreArray<RecoTrack> recoTracks(m_recoTracksStoreArrayName);
  if (!recoTracks.isOptional()) {
    B2DEBUG(22, "Missing recoTracks array in event() for " + m_histoModule->getName() + " module.");
    return;
  }
  StoreArray<Track> tracks(m_tracksStoreArrayName);
  if (!tracks.isOptional()) {
    B2DEBUG(22, "Missing recoTracks array in event() for " + m_histoModule->getName() + " module.");
    return;
  }

  try {
    m_iTrack = 0;
    m_iTrackVXD = 0;
    m_iTrackCDC = 0;
    m_iTrackVXDCDC = 0;

    for (const Track& track : tracks) {
      ProcessTrack(track);
    }

    m_histoModule->FillTrackIndexes(m_iTrack, m_iTrackVXD, m_iTrackCDC, m_iTrackVXDCDC);
  } catch (const std::exception& e) {
    B2WARNING("Exception " + std::string(e.what()) + " in " + m_histoModule->getName() + " module!");
  } catch (...) {
    B2WARNING("Unhandled exception in " + m_histoModule->getName() + " module!");
  }
}

void DQMEventProcessorBase::ProcessTrack(const Track& track)
{
  auto recoTracksVector = track.getRelationsTo<RecoTrack>(m_recoTracksStoreArrayName);
  if (!recoTracksVector.size())
    return;

  m_recoTrack = recoTracksVector[0];

  int nPXDClusters = 0;
  if (!m_runningOnHLT) {
    RelationVector<PXDCluster> pxdClusters = DataStore::getRelationsWithObj<PXDCluster>(m_recoTrack);
    nPXDClusters = (int)pxdClusters.size();
  }
  RelationVector<SVDCluster> svdClusters = DataStore::getRelationsWithObj<SVDCluster>(m_recoTrack);
  int nSVDClusters = (int)svdClusters.size();
  RelationVector<CDCHit> cdcHits = DataStore::getRelationsWithObj<CDCHit>(m_recoTrack);
  int nCDCHits = (int)cdcHits.size();

  // This method allways returns TrackFitResult so there's no need to check if it's not nullptr
  auto trackFitResult = track.getTrackFitResultWithClosestMass(Const::pion);

  TString message = ConstructMessage(trackFitResult, nPXDClusters, nSVDClusters, nCDCHits);
  B2DEBUG(20, message.Data());

  FillTrackFitResult(trackFitResult);
  m_histoModule->FillHitNumbers(nPXDClusters, nSVDClusters, nCDCHits);

  if (m_recoTrack->wasFitSuccessful())
    ProcessSuccessfulFit();

  m_iTrack++;
  if (((nPXDClusters > 0) || (nSVDClusters > 0)) && (nCDCHits == 0))
    m_iTrackVXD++;
  if (((nPXDClusters == 0) && (nSVDClusters == 0)) && (nCDCHits > 0))
    m_iTrackCDC++;
  if (((nPXDClusters > 0) || (nSVDClusters > 0)) && (nCDCHits > 0))
    m_iTrackVXDCDC++;
}

TString DQMEventProcessorBase::ConstructMessage(const TrackFitResult* trackFitResult, int nPXDClusters, int nSVDClusters,
                                                int nCDCHits)
{
  return Form("%s: track %3i, Mom: %f, %f, %f, Pt: %f, Mag: %f, Hits: PXD %i SVD %i CDC %i Suma %i\n",
              m_histoModule->getName().c_str(),
              m_iTrack,
              (float)trackFitResult->getMomentum().Px(),
              (float)trackFitResult->getMomentum().Py(),
              (float)trackFitResult->getMomentum().Pz(),
              (float)trackFitResult->getMomentum().Pt(),
              (float)trackFitResult->getMomentum().Mag(),
              nPXDClusters, nSVDClusters, nCDCHits, nPXDClusters + nSVDClusters + nCDCHits
             );
}

void DQMEventProcessorBase::FillTrackFitResult(const TrackFitResult* trackFitResult)
{
  m_histoModule->FillMomentumAngles(trackFitResult);
  m_histoModule->FillMomentumCoordinates(trackFitResult);
  m_histoModule->FillHelixParametersAndCorrelations(trackFitResult);
}

void DQMEventProcessorBase::ProcessSuccessfulFit()
{
  // function wasFitSuccessful already checked if TrackFitStatus is not nullptr so it's not necessary to do so
  m_histoModule->FillTrackFitStatus(m_recoTrack->getTrackFitStatus());

  m_isNotFirstHit = false;

  for (auto recoHitInfo : m_recoTrack->getRecoHitInformations(true)) {
    ProcessRecoHit(recoHitInfo);
  }
}

void DQMEventProcessorBase::ProcessRecoHit(RecoHitInformation* recoHitInfo)
{
  if (!recoHitInfo) {
    B2DEBUG(20, "Missing genfit::pxd recoHitInfo in event() for " + m_histoModule->getName() + " module.");
    return;
  }

  if (!recoHitInfo->useInFit())
    return;

  bool isPXD = recoHitInfo->getTrackingDetector() == RecoHitInformation::c_PXD;
  bool isSVD = recoHitInfo->getTrackingDetector() == RecoHitInformation::c_SVD;
  if (!isPXD && !isSVD)
    return;

  auto fitterInfo = m_recoTrack->getCreatedTrackPoint(recoHitInfo)->getFitterInfo();
  if (!fitterInfo)
    return;

  m_rawSensorResidual = new TVectorT<double>(fitterInfo->getResidual(0, false).getState());

  if (isPXD && ! m_runningOnHLT) {
    ProcessPXDRecoHit(recoHitInfo);
  } else if (isSVD) {
    ProcessSVDRecoHit(recoHitInfo);
  }

  delete m_rawSensorResidual;
}

void DQMEventProcessorBase::ProcessPXDRecoHit(RecoHitInformation* recoHitInfo)
{
  m_position.SetX(recoHitInfo->getRelatedTo<PXDCluster>()->getU());
  m_position.SetY(recoHitInfo->getRelatedTo<PXDCluster>()->getV());
  m_residual_um.SetX(m_rawSensorResidual->GetMatrixArray()[0] / Unit::um);
  m_residual_um.SetY(m_rawSensorResidual->GetMatrixArray()[1] / Unit::um);

  m_sensorID = recoHitInfo->getRelatedTo<PXDCluster>()->getSensorID();
  ComputeCommonVariables();

  FillCommonHistograms();

  m_histoModule->FillUBResidualsPXD(m_residual_um);
  m_histoModule->FillHalfShellsPXD(m_globalResidual_um, IsNotYang(m_sensorID.getLadderNumber(), m_layerNumber));

  SetCommonPrevVariables();
}

void DQMEventProcessorBase::ProcessSVDRecoHit(RecoHitInformation* recoHitInfo)
{
  if (recoHitInfo->getRelatedTo<SVDCluster>()->isUCluster()) {
    m_position.SetX(recoHitInfo->getRelatedTo<SVDCluster>()->getPosition());
    m_residual_um.SetX(m_rawSensorResidual->GetMatrixArray()[0] / Unit::um);
  } else {
    m_position.SetY(recoHitInfo->getRelatedTo<SVDCluster>()->getPosition());
    m_residual_um.SetY(m_rawSensorResidual->GetMatrixArray()[0] / Unit::um);
  }

  m_sensorID = recoHitInfo->getRelatedTo<SVDCluster>()->getSensorID();
  if (m_sensorIDPrev == m_sensorID) {
    ComputeCommonVariables();

    if (! m_runningOnHLT) {
      FillCommonHistograms();

      m_histoModule->FillUBResidualsSVD(m_residual_um);
      m_histoModule->FillHalfShellsSVD(m_globalResidual_um, IsNotMat(m_sensorID.getLadderNumber(), m_layerNumber));
    }

    SetCommonPrevVariables();
  }

  m_sensorIDPrev = m_sensorID;
}

void DQMEventProcessorBase::ComputeCommonVariables()
{
  auto sensorInfo = &VXD::GeoCache::get(m_sensorID);
  m_globalResidual_um = sensorInfo->vectorToGlobal(m_residual_um, true);
  TVector3 globalPosition = sensorInfo->pointToGlobal(m_position, true);

  m_phi_deg = globalPosition.Phi() / Unit::deg;
  m_theta_deg = globalPosition.Theta() / Unit::deg;

  m_layerNumber = m_sensorID.getLayerNumber();

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  m_layerIndex = gTools->getLayerIndex(m_layerNumber);
  m_correlationIndex = m_layerIndex - gTools->getFirstLayer();
  m_sensorIndex = gTools->getSensorIndex(m_sensorID);
}

void DQMEventProcessorBase::FillCommonHistograms()
{
  if (m_isNotFirstHit && ((m_layerNumber - m_layerNumberPrev) == 1)) {
    m_histoModule->FillTRClusterCorrelations(m_phi_deg, m_phiPrev_deg, m_theta_deg, m_thetaPrev_deg, m_correlationIndex);
  } else {
    m_isNotFirstHit = true;
  }

  //  m_histoModule->FillUBResidualsSensor(m_residual_um, m_sensorIndex);
  m_histoModule->FillTRClusterHitmap(m_phi_deg, m_theta_deg, m_layerIndex);
}

void DQMEventProcessorBase::SetCommonPrevVariables()
{
  m_layerNumberPrev = m_layerNumber;
  m_phiPrev_deg = m_phi_deg;
  m_thetaPrev_deg = m_theta_deg;
}

bool DQMEventProcessorBase::IsNotYang(int ladderNumber, int layerNumber)
{
  switch (layerNumber) {
    case 1:
      return ladderNumber < 5 || ladderNumber > 8;
    case 2:
      return ladderNumber < 7 || ladderNumber > 12;
    default:
      return true;
  }
}

bool DQMEventProcessorBase::IsNotMat(int ladderNumber, int layerNumber)
{
  switch (layerNumber) {
    case 3:
      return ladderNumber < 3 || ladderNumber > 5;
    case 4:
      return ladderNumber < 4 || ladderNumber > 8;
    case 5:
      return ladderNumber < 5 || ladderNumber > 10;
    case 6:
      return ladderNumber < 6 || ladderNumber > 13;
    default:
      return true;
  }
}
