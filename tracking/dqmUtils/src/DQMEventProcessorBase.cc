#include <tracking/dqmUtils/DQMEventProcessorBase.h>
#include <tracking/dqmUtils/DQMHistoModuleBase.h>

#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <vxd/geometry/GeoTools.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <TVectorD.h>

using namespace Belle2;

void DQMEventProcessorBase::Run()
{
  StoreArray<Track> tracks(m_tracksStoreArrayName);
  if (!tracks.isOptional() || !tracks.getEntries())
    return;

  try {
    m_iTrack = 0;
    m_iTrackVXD = 0;
    m_iTrackCDC = 0;
    m_iTrackVXDCDC = 0;

    for (const Track& track : tracks) {
      ProcessOneTrack(track);
    }

    m_histoModule->FillTracks(m_iTrack, m_iTrackVXD, m_iTrackCDC, m_iTrackVXDCDC);
  } catch (...) {
    // TODO
    // B2DEBUG(70, "Some problem in Alignment DQM module!"); // this line is from AlignDQMModule
    B2DEBUG(20, "Some problem in Track DQM module!"); // this line is from TrackDQMModule
  }
}

void DQMEventProcessorBase::ProcessOneTrack(const Track& track)
{
  RelationVector<RecoTrack> recoTracksVector = track.getRelationsTo<RecoTrack>(m_recoTracksStoreArrayName);

  if (!recoTracksVector.size())
    return;

  m_recoTrack = recoTracksVector[0];

  RelationVector<PXDCluster> pxdClustersTrack = DataStore::getRelationsWithObj<PXDCluster>(m_recoTrack);
  m_nPXD = (int)pxdClustersTrack.size();
  RelationVector<SVDCluster> svdClustersTrack = DataStore::getRelationsWithObj<SVDCluster>(m_recoTrack);
  m_nSVD = (int)svdClustersTrack.size();
  RelationVector<CDCHit> cdcHitTrack = DataStore::getRelationsWithObj<CDCHit>(m_recoTrack);
  m_nCDC = (int)cdcHitTrack.size();

  m_trackFitResult = track.getTrackFitResultWithClosestMass(Const::pion);
  if (m_trackFitResult == nullptr)
    return;

  TString message = ConstructMessage();

  // TODO
  // B2DEBUG(230, message.Data()); // this line is from AlignDQMModule
  B2DEBUG(20, message.Data()); // this line is from TrackDQMModule
  m_iTrack++;

  m_histoModule->FillMomentum(m_trackFitResult);

  bool wasProcessingSuccessful = false;
  if (m_recoTrack->wasFitSuccessful())
    wasProcessingSuccessful = ProcessSuccessfulFit();

  if (!wasProcessingSuccessful)
    return;

  if (((m_nPXD > 0) || (m_nSVD > 0)) && (m_nCDC > 0))
    m_iTrackVXDCDC++;
  if (((m_nPXD > 0) || (m_nSVD > 0)) && (m_nCDC == 0))
    m_iTrackVXD++;
  if (((m_nPXD == 0) && (m_nSVD == 0)) && (m_nCDC > 0))
    m_iTrackCDC++;

  m_histoModule->FillHits(m_nPXD, m_nSVD, m_nCDC);
  m_histoModule->FillTrackFitResult(m_trackFitResult);
}

bool DQMEventProcessorBase::ProcessSuccessfulFit()
{
  if (!m_recoTrack->getTrackFitStatus())
    return false;

  m_histoModule->FillTrackFitStatus(m_recoTrack->getTrackFitStatus());

  m_isNotFirstHit = false;

  for (auto recoHitInfo : m_recoTrack->getRecoHitInformations(true)) {
    ProcessOneRecoHit(recoHitInfo);
  }

  return true;
}

void DQMEventProcessorBase::ProcessOneRecoHit(RecoHitInformation* recoHitInfo)
{
  if (!recoHitInfo) {
    // TODO
    // B2DEBUG(200, "No genfit::pxd recoHitInfo is missing."); // this line is from AlignDQMModule
    B2DEBUG(20, "No genfit::pxd recoHitInfo is missing."); // this line is from TrackDQMModule
    return;
  }

  if (!recoHitInfo->useInFit())
    return;

  if (!m_recoTrack->getCreatedTrackPoint(recoHitInfo)->getFitterInfo())
    return;

  bool isPXD = recoHitInfo->getTrackingDetector() == RecoHitInformation::c_PXD;
  bool isSVD = recoHitInfo->getTrackingDetector() == RecoHitInformation::c_SVD;
  if (!isPXD && !isSVD)
    return;

  bool biased = false;
  m_resUnBias = new TVectorT<double>(m_recoTrack->getCreatedTrackPoint(recoHitInfo)->getFitterInfo()->getResidual(0,
                                     biased).getState());

  if (isPXD) {
    ProcessPXDRecoHit(recoHitInfo);
  } else {
    ProcessSVDRecoHit(recoHitInfo);
  }

  delete m_resUnBias;
}

void DQMEventProcessorBase::ProcessPXDRecoHit(RecoHitInformation* recoHitInfo)
{
  m_posU = recoHitInfo->getRelatedTo<PXDCluster>()->getU();
  m_posV = recoHitInfo->getRelatedTo<PXDCluster>()->getV();
  m_residUPlaneRHUnBias = m_resUnBias->GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
  m_residVPlaneRHUnBias = m_resUnBias->GetMatrixArray()[1] * Unit::convertValueToUnit(1.0, "um");

  m_sensorID = recoHitInfo->getRelatedTo<PXDCluster>()->getSensorID();
  ComputeCommonVariables();

  FillCommonHistograms();

  m_histoModule->FillUBResidualsPXD(m_residUPlaneRHUnBias, m_residVPlaneRHUnBias);
  m_histoModule->FillPXDHalfShells(m_residUPlaneRHUnBias, m_residVPlaneRHUnBias, m_sensorInfo, IsNotYang(m_sensorID.getLadderNumber(),
                                   m_sensorID.getLayerNumber()));

  SetCommonPrevVariables();
}

void DQMEventProcessorBase::ProcessSVDRecoHit(RecoHitInformation* recoHitInfo)
{
  if (recoHitInfo->getRelatedTo<SVDCluster>()->isUCluster()) {
    m_posU = recoHitInfo->getRelatedTo<SVDCluster>()->getPosition();
    m_residUPlaneRHUnBias = m_resUnBias->GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
  } else {
    m_posV = recoHitInfo->getRelatedTo<SVDCluster>()->getPosition();
    m_residVPlaneRHUnBias = m_resUnBias->GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
  }

  m_sensorID = recoHitInfo->getRelatedTo<SVDCluster>()->getSensorID();
  if (m_sensorIDPrew == m_sensorID) {
    ComputeCommonVariables();

    FillCommonHistograms();

    m_histoModule->FillUBResidualsSVD(m_residUPlaneRHUnBias, m_residVPlaneRHUnBias);
    m_histoModule->FillSVDHalfShells(m_residUPlaneRHUnBias, m_residVPlaneRHUnBias, m_sensorInfo, IsNotMat(m_sensorID.getLadderNumber(),
                                     m_sensorID.getLayerNumber()));

    SetCommonPrevVariables();
  }

  m_sensorIDPrew = m_sensorID;
}

void DQMEventProcessorBase::ComputeCommonVariables()
{
  m_sensorInfo = &VXD::GeoCache::get(m_sensorID);

  TVector3 rLocal(m_posU, m_posV, 0);
  TVector3 ral = m_sensorInfo->pointToGlobal(rLocal, true);

  m_fPosSPV = ral.Theta() / TMath::Pi() * 180;
  m_fPosSPU = ral.Phi() / TMath::Pi() * 180;

  m_layerNumber = m_sensorID.getLayerNumber();

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  m_layerIndex = gTools->getLayerIndex(m_layerNumber);
  m_correlationIndex = m_layerIndex - gTools->getFirstLayer();
  m_sensorIndex = gTools->getSensorIndex(m_sensorID);
}

void DQMEventProcessorBase::FillCommonHistograms()
{
  if (m_isNotFirstHit && ((m_layerNumber - m_layerNumberPrev) == 1)) {
    m_histoModule->FillCorrelations(m_fPosSPU, m_fPosSPUPrev, m_fPosSPV, m_fPosSPVPrev, m_correlationIndex);
  } else {
    m_isNotFirstHit = true;
  }

  m_histoModule->FillUBResidualsSensor(m_residUPlaneRHUnBias, m_residVPlaneRHUnBias, m_sensorIndex);
  m_histoModule->FillTRClusterHitmap(m_fPosSPU, m_fPosSPV, m_layerIndex);
}

void DQMEventProcessorBase::SetCommonPrevVariables()
{
  m_layerNumberPrev = m_layerNumber;
  m_fPosSPUPrev = m_fPosSPU;
  m_fPosSPVPrev = m_fPosSPV;
}

bool DQMEventProcessorBase::IsNotYang(int ladder, int layer)
{
  switch (layer) {
    case 1:
      return ladder < 5 || ladder > 8;
    case 2:
      return ladder < 7 || ladder > 12;
    default:
      return true;
  }
}

bool DQMEventProcessorBase::IsNotMat(int ladder, int layer)
{
  switch (layer) {
    case 3:
      return ladder < 3 || ladder > 5;
    case 4:
      return ladder < 4 || ladder > 8;
    case 5:
      return ladder < 5 || ladder > 10;
    case 6:
      return ladder < 6 || ladder > 13;
    default:
      return true;
  }
}