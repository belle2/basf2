#include <tracking/dqmUtils/BaseDQMEventProcessor.h>
#include <tracking/dqmUtils/BaseDQMHistogramModule.h>

#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <vxd/geometry/GeoTools.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <TVectorD.h>

using namespace Belle2;

void BaseDQMEventProcessor::Run()
{
  StoreArray<Track> tracks(m_TracksStoreArrayName);
  if (!tracks.isOptional() || !tracks.getEntries())
    return;

  try {
    iTrack = 0;
    iTrackVXD = 0;
    iTrackCDC = 0;
    iTrackVXDCDC = 0;

    for (const Track& track : tracks) {
      ProcessOneTrack(track);
    }

    histoModule->FillTracks(iTrack, iTrackVXD, iTrackCDC, iTrackVXDCDC);
  } catch (...) {
    // TODO
    // B2DEBUG(70, "Some problem in Alignment DQM module!"); // this line is from AlignDQMModule
    B2DEBUG(20, "Some problem in Track DQM module!"); // this line is from TrackDQMModule
  }
}

void BaseDQMEventProcessor::ProcessOneTrack(const Track& track)
{
  RelationVector<RecoTrack> recoTracksVector = track.getRelationsTo<RecoTrack>(m_RecoTracksStoreArrayName);

  if (!recoTracksVector.size())
    return;

  recoTrack = recoTracksVector[0];

  RelationVector<PXDCluster> pxdClustersTrack = DataStore::getRelationsWithObj<PXDCluster>(recoTrack);
  nPXD = (int)pxdClustersTrack.size();
  RelationVector<SVDCluster> svdClustersTrack = DataStore::getRelationsWithObj<SVDCluster>(recoTrack);
  nSVD = (int)svdClustersTrack.size();
  RelationVector<CDCHit> cdcHitTrack = DataStore::getRelationsWithObj<CDCHit>(recoTrack);
  nCDC = (int)cdcHitTrack.size();

  trackFitResult = track.getTrackFitResultWithClosestMass(Const::pion);
  if (trackFitResult == nullptr)
    return;

  TString message = ConstructMessage();

  // TODO
  // B2DEBUG(230, message.Data()); // this line is from AlignDQMModule
  B2DEBUG(20, message.Data()); // this line is from TrackDQMModule
  iTrack++;

  histoModule->FillMomentum(trackFitResult);

  bool wasProcessingSuccessful = false;
  if (recoTrack->wasFitSuccessful())
    wasProcessingSuccessful = ProcessSuccessfulFit();

  if (!wasProcessingSuccessful)
    return;

  if (((nPXD > 0) || (nSVD > 0)) && (nCDC > 0))
    iTrackVXDCDC++;
  if (((nPXD > 0) || (nSVD > 0)) && (nCDC == 0))
    iTrackVXD++;
  if (((nPXD == 0) && (nSVD == 0)) && (nCDC > 0))
    iTrackCDC++;

  histoModule->FillHits(nPXD, nSVD, nCDC);
  histoModule->FillTrackFitResult(trackFitResult);
}

bool BaseDQMEventProcessor::ProcessSuccessfulFit()
{
  if (!recoTrack->getTrackFitStatus())
    return false;

  histoModule->FillTrackFitStatus(recoTrack->getTrackFitStatus());

  isNotFirstHit = false;

  for (auto recoHitInfo : recoTrack->getRecoHitInformations(true)) {
    ProcessOneRecoHit(recoHitInfo);
  }

  return true;
}

void BaseDQMEventProcessor::ProcessOneRecoHit(RecoHitInformation* recoHitInfo)
{
  if (!recoHitInfo) {
    // TODO
    // B2DEBUG(200, "No genfit::pxd recoHitInfo is missing."); // this line is from AlignDQMModule
    B2DEBUG(20, "No genfit::pxd recoHitInfo is missing."); // this line is from TrackDQMModule
    return;
  }

  if (!recoHitInfo->useInFit())
    return;

  if (!recoTrack->getCreatedTrackPoint(recoHitInfo)->getFitterInfo())
    return;

  bool isPXD = recoHitInfo->getTrackingDetector() == RecoHitInformation::c_PXD;
  bool isSVD = recoHitInfo->getTrackingDetector() == RecoHitInformation::c_SVD;
  if (!isPXD && !isSVD)
    return;

  bool biased = false;
  resUnBias = new TVectorT<double>(recoTrack->getCreatedTrackPoint(recoHitInfo)->getFitterInfo()->getResidual(0, biased).getState());

  if (isPXD) {
    ProcessPXDRecoHit(recoHitInfo);
  } else {
    ProcessSVDRecoHit(recoHitInfo);
  }

  delete resUnBias;
}

void BaseDQMEventProcessor::ProcessPXDRecoHit(RecoHitInformation* recoHitInfo)
{
  posU = recoHitInfo->getRelatedTo<PXDCluster>()->getU();
  posV = recoHitInfo->getRelatedTo<PXDCluster>()->getV();
  ResidUPlaneRHUnBias = resUnBias->GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
  ResidVPlaneRHUnBias = resUnBias->GetMatrixArray()[1] * Unit::convertValueToUnit(1.0, "um");

  sensorID = recoHitInfo->getRelatedTo<PXDCluster>()->getSensorID();
  ComputeCommonVariables();

  FillCommonHistograms();

  histoModule->FillUBResidualsPXD(ResidUPlaneRHUnBias, ResidVPlaneRHUnBias);
  histoModule->FillPXDHalfShells(ResidUPlaneRHUnBias, ResidVPlaneRHUnBias, sensorInfo, IsNotYang(sensorID.getLadderNumber(),
                                 sensorID.getLayerNumber()));

  SetCommonPrevVariables();
}

void BaseDQMEventProcessor::ProcessSVDRecoHit(RecoHitInformation* recoHitInfo)
{
  if (recoHitInfo->getRelatedTo<SVDCluster>()->isUCluster()) {
    posU = recoHitInfo->getRelatedTo<SVDCluster>()->getPosition();
    ResidUPlaneRHUnBias = resUnBias->GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
  } else {
    posV = recoHitInfo->getRelatedTo<SVDCluster>()->getPosition();
    ResidVPlaneRHUnBias = resUnBias->GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
  }

  sensorID = recoHitInfo->getRelatedTo<SVDCluster>()->getSensorID();
  if (sensorIDPrew == sensorID) {
    ComputeCommonVariables();

    FillCommonHistograms();

    histoModule->FillUBResidualsSVD(ResidUPlaneRHUnBias, ResidVPlaneRHUnBias);
    histoModule->FillSVDHalfShells(ResidUPlaneRHUnBias, ResidVPlaneRHUnBias, sensorInfo, IsNotMat(sensorID.getLadderNumber(),
                                   sensorID.getLayerNumber()));

    SetCommonPrevVariables();
  }

  sensorIDPrew = sensorID;
}

void BaseDQMEventProcessor::ComputeCommonVariables()
{
  sensorInfo = &VXD::GeoCache::get(sensorID);

  TVector3 rLocal(posU, posV, 0);
  TVector3 ral = sensorInfo->pointToGlobal(rLocal, true);

  fPosSPV = ral.Theta() / TMath::Pi() * 180;
  fPosSPU = ral.Phi() / TMath::Pi() * 180;

  layerNumber = sensorID.getLayerNumber();

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  layerIndex = gTools->getLayerIndex(layerNumber);
  correlationIndex = layerIndex - gTools->getFirstLayer();
  sensorIndex = gTools->getSensorIndex(sensorID);
}

void BaseDQMEventProcessor::FillCommonHistograms()
{
  if (isNotFirstHit && ((layerNumber - layerNumberPrev) == 1)) {
    histoModule->FillCorrelations(fPosSPU, fPosSPUPrev, fPosSPV, fPosSPVPrev, correlationIndex);
  } else {
    isNotFirstHit = true;
  }

  histoModule->FillUBResidualsSensor(ResidUPlaneRHUnBias, ResidVPlaneRHUnBias, sensorIndex);
  histoModule->FillTRClusterHitmap(fPosSPU, fPosSPV, layerIndex);
}

void BaseDQMEventProcessor::SetCommonPrevVariables()
{
  layerNumberPrev = layerNumber;
  fPosSPUPrev = fPosSPU;
  fPosSPVPrev = fPosSPV;
}

bool BaseDQMEventProcessor::IsNotYang(int ladder, int layer)
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

bool BaseDQMEventProcessor::IsNotMat(int ladder, int layer)
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