/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for Tracking DQM                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <genfit/MeasurementOnPlane.h>
#include <tracking/modules/trackingDQM/TrackDQMModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/geometry/SensorInfo.h>
#include <pxd/geometry/SensorInfo.h>


#include <framework/database/DBObjPtr.h>

#include <TDirectory.h>
#include <TVectorD.h>

using namespace Belle2;
using namespace std;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrackDQM)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackDQMModule::TrackDQMModule() : HistoModule()
{
  //Set module properties
  setDescription("DQM of finding tracks, their momentum, "
                 "Number of hits in tracks, "
                 "Number of tracks. "
                );
  setPropertyFlags(c_ParallelProcessingCertified);

}


TrackDQMModule::~TrackDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void TrackDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

}

void TrackDQMModule::defineHisto()
{

  // basic constants presets:
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  c_nVXDLayers = geo.getLayers().size();
  c_firstVXDLayer = 1;  // counting start from 1...
  c_lastVXDLayer = c_nVXDLayers;
  c_nPXDLayers = geo.getLayers(VXD::SensorInfoBase::SensorType::PXD).size();
  c_firstPXDLayer = c_firstVXDLayer;
  c_lastPXDLayer = c_nPXDLayers;
  c_nSVDLayers = geo.getLayers(VXD::SensorInfoBase::SensorType::SVD).size();
  c_firstSVDLayer = c_nPXDLayers + c_firstPXDLayer;
  c_lastSVDLayer = c_firstSVDLayer + c_nSVDLayers;

  c_MaxLaddersInPXDLayer = 0;
  c_MaxLaddersInSVDLayer = 0;
  c_MaxSensorsInPXDLayer = 0;
  c_MaxSensorsInSVDLayer = 0;

  for (VxdID layer : geo.getLayers()) {
    for (VxdID ladder : geo.getLadders(layer)) {
      if (layer.getLayerNumber() <= c_lastPXDLayer) {  // PXD
        if (c_MaxLaddersInPXDLayer < geo.getLadders(layer).size())
          c_MaxLaddersInPXDLayer = geo.getLadders(layer).size();
        if (c_MaxSensorsInPXDLayer < geo.getSensors(ladder).size())
          c_MaxSensorsInPXDLayer = geo.getSensors(ladder).size();
      } else { // SVD
        if (c_MaxLaddersInSVDLayer < geo.getLadders(layer).size())
          c_MaxLaddersInSVDLayer = geo.getLadders(layer).size();
        if (c_MaxSensorsInSVDLayer < geo.getSensors(ladder).size())
          c_MaxSensorsInSVDLayer = geo.getSensors(ladder).size();
      }
      break;
    }
  }

  c_nPXDSensors = 0;
  for (VxdID layer : geo.getLayers()) {
    for (VxdID ladder : geo.getLadders(layer)) {
      if (layer.getLayerNumber() <= c_lastPXDLayer) {  // PXD
        c_nPXDSensors += geo.getLadders(layer).size() * geo.getSensors(ladder).size();
      }
      break;
    }
  }

  c_nSVDSensors = 0;
  for (VxdID layer : geo.getLayers()) {
    for (VxdID ladder : geo.getLadders(layer)) {
      if (layer.getLayerNumber() > c_lastPXDLayer) {  // SVD
        c_nSVDSensors += geo.getLadders(layer).size() * geo.getSensors(ladder).size();
      }
      break;
    }
  }

  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;
  TDirectory* DirTracks = NULL;
  DirTracks = oldDir->mkdir("TracksDQM");
  TDirectory* DirTracksAlignment = NULL;
  DirTracksAlignment = oldDir->mkdir("TracksDQMAlignment");
  DirTracks->cd();

  /** p Value */
  string name = str(format("PValue"));
  string title = str(format("P value of fit"));
  m_PValue = new TH1F(name.c_str(), title.c_str(), 100, 0, 1);
  m_PValue->GetXaxis()->SetTitle("p value");
  m_PValue->GetYaxis()->SetTitle("counts");
  /** Chi2 */
  name = str(format("Chi2"));
  title = str(format("Chi2 of fit"));
  m_Chi2 = new TH1F(name.c_str(), title.c_str(), 200, 0, 150);
  m_Chi2->GetXaxis()->SetTitle("Chi2");
  m_Chi2->GetYaxis()->SetTitle("counts");
  /** NDF */
  name = str(format("NDF"));
  title = str(format("NDF of fit"));
  m_NDF = new TH1F(name.c_str(), title.c_str(), 200, 0, 200);
  m_NDF->GetXaxis()->SetTitle("NDF");
  m_NDF->GetYaxis()->SetTitle("counts");
  /** Chi2 / NDF */
  name = str(format("Chi2NDF"));
  title = str(format("Chi2 div NDF of fit"));
  m_Chi2NDF = new TH1F(name.c_str(), title.c_str(), 200, 0, 10);
  m_Chi2NDF->GetXaxis()->SetTitle("Chi2NDF");
  m_Chi2NDF->GetYaxis()->SetTitle("counts");

  /** Unbiased residuals for PXD u vs v */
  name = str(format("UBResidualsPXD"));
  title = str(format("Unbiased residuals for PXD"));
  m_UBResidualsPXD = new TH2F(name.c_str(), title.c_str(), 200, -200, 200, 200, -200, 200);
  m_UBResidualsPXD->GetXaxis()->SetTitle("u residual [#mum]");
  m_UBResidualsPXD->GetYaxis()->SetTitle("v residual [#mum]");
  m_UBResidualsPXD->GetZaxis()->SetTitle("counts");
  /** Unbiased residuals for SVD u vs v */
  name = str(format("UBResidualsSVD"));
  title = str(format("Unbiased residuals for SVD"));
  m_UBResidualsSVD = new TH2F(name.c_str(), title.c_str(), 200, -200, 200, 200, -200, 200);
  m_UBResidualsSVD->GetXaxis()->SetTitle("u residual [#mum]");
  m_UBResidualsSVD->GetYaxis()->SetTitle("v residual [#mum]");
  m_UBResidualsSVD->GetZaxis()->SetTitle("counts");

  m_TRClusterHitmap = (TH2F**) new TH2F*[c_nVXDLayers];
  m_TRClusterCorrelationsPhi = (TH2F**) new TH2F*[c_nVXDLayers - 1];
  m_TRClusterCorrelationsTheta = (TH2F**) new TH2F*[c_nVXDLayers - 1];
  m_UBResidualsSensor = (TH2F**) new TH2F*[c_nPXDSensors + c_nSVDSensors];

  for (int i = 0; i < c_nVXDLayers; i++) {
    /** Track related clusters - hitmap in IP angle range */
    name = str(format("TRClusterHitmapLayer%1%") % (i + 1));
    title = str(format("Cluster Hitmap for layer %1%") % (i + 1));
    m_TRClusterHitmap[i] = new TH2F(name.c_str(), title.c_str(), 360, -180.0, 180.0, 180, 0.0, 180.0);
    m_TRClusterHitmap[i]->GetXaxis()->SetTitle("Phi angle [deg]");
    m_TRClusterHitmap[i]->GetYaxis()->SetTitle("Theta angle [deg]");
    m_TRClusterHitmap[i]->GetZaxis()->SetTitle("counts");
  }
  for (int i = 0; i < c_nVXDLayers - 1; i++) {
    /** Track related clusters - neighbor corelations in Phi */
    name = str(format("CorrelationsPhiLayers_%1%_%2%") % (i + 1) % (i + 2));
    title = str(format("Correlations in Phi for Layers %1% %2%") % (i + 1) % (i + 2));
    m_TRClusterCorrelationsPhi[i] = new TH2F(name.c_str(), title.c_str(), 360, -180.0, 180.0, 360, -180.0, 180.0);
    title = str(format("angle layer %1% [deg]") % (i + 1));
    m_TRClusterCorrelationsPhi[i]->GetXaxis()->SetTitle(title.c_str());
    title = str(format("angle layer %1% [deg]") % (i + 2));
    m_TRClusterCorrelationsPhi[i]->GetYaxis()->SetTitle(title.c_str());
    m_TRClusterCorrelationsPhi[i]->GetZaxis()->SetTitle("counts");
    /** Track related clusters - neighbor corelations in Theta */
    name = str(format("CorrelationsThetaLayers_%1%_%2%") % (i + 1) % (i + 2));
    title = str(format("Correlations in Theta for Layers %1% %2%") % (i + 1) % (i + 2));
    m_TRClusterCorrelationsTheta[i] = new TH2F(name.c_str(), title.c_str(), 180, 0.0, 180.0, 180, 0.0, 180.0);
    title = str(format("angle layer %1% [deg]") % (i + 1));
    m_TRClusterCorrelationsTheta[i]->GetXaxis()->SetTitle(title.c_str());
    title = str(format("angle layer %1% [deg]") % (i + 2));
    m_TRClusterCorrelationsTheta[i]->GetYaxis()->SetTitle(title.c_str());
    m_TRClusterCorrelationsTheta[i]->GetZaxis()->SetTitle("counts");
  }

  m_MomX = NULL;
  m_MomY = NULL;
  m_MomZ = NULL;
  m_Mom = NULL;
  m_HitsPXD = NULL;
  m_HitsSVD = NULL;
  m_HitsCDC = NULL;
  m_Hits = NULL;
  m_TracksVXD = NULL;
  m_TracksCDC = NULL;
  m_TracksVXDCDC = NULL;
  m_Tracks = NULL;

  int iHitsInPXD = 10;
  int iHitsInSVD = 20;
  int iHitsInCDC = 200;
  int iHits = 200;
  int iTracks = 30;
  int iMomRange = 600;
  float fMomRange = 3.0;
  name = str(format("TrackMomentumX"));
  title = str(format("Track Momentum X"));
  m_MomX = new TH1F(name.c_str(), title.c_str(), 2 * iMomRange, -fMomRange, fMomRange);
  m_MomX->GetXaxis()->SetTitle("Momentum");
  m_MomX->GetYaxis()->SetTitle("counts");
  name = str(format("TrackMomentumY"));
  title = str(format("Track Momentum Y"));
  m_MomY = new TH1F(name.c_str(), title.c_str(), 2 * iMomRange, -fMomRange, fMomRange);
  m_MomY->GetXaxis()->SetTitle("Momentum");
  m_MomY->GetYaxis()->SetTitle("counts");
  name = str(format("TrackMomentumZ"));
  title = str(format("Track Momentum Z"));
  m_MomZ = new TH1F(name.c_str(), title.c_str(), 2 * iMomRange, -fMomRange, fMomRange);
  m_MomZ->GetXaxis()->SetTitle("Momentum");
  m_MomZ->GetYaxis()->SetTitle("counts");
  name = str(format("TrackMomentumPt"));
  title = str(format("Track Momentum pT"));
  m_MomPt = new TH1F(name.c_str(), title.c_str(), 2 * iMomRange, 0.0, fMomRange);
  m_MomPt->GetXaxis()->SetTitle("Momentum");
  m_MomPt->GetYaxis()->SetTitle("counts");
  name = str(format("TrackMomentumMag"));
  title = str(format("Track Momentum Magnitude"));
  m_Mom = new TH1F(name.c_str(), title.c_str(), 2 * iMomRange, 0.0, fMomRange);
  m_Mom->GetXaxis()->SetTitle("Momentum");
  m_Mom->GetYaxis()->SetTitle("counts");

  name = str(format("NoOfHitsInTrack_PXD"));
  title = str(format("No Of Hits In Track - PXD"));
  m_HitsPXD = new TH1F(name.c_str(), title.c_str(), iHitsInPXD, 0, iHitsInPXD);
  m_HitsPXD->GetXaxis()->SetTitle("# hits");
  m_HitsPXD->GetYaxis()->SetTitle("counts");
  name = str(format("NoOfHitsInTrack_SVD"));
  title = str(format("No Of Hits In Track - SVD"));
  m_HitsSVD = new TH1F(name.c_str(), title.c_str(), iHitsInSVD, 0, iHitsInSVD);
  m_HitsSVD->GetXaxis()->SetTitle("# hits");
  m_HitsSVD->GetYaxis()->SetTitle("counts");
  name = str(format("NoOfHitsInTrack_CDC"));
  title = str(format("No Of Hits In Track - CDC"));
  m_HitsCDC = new TH1F(name.c_str(), title.c_str(), iHitsInCDC, 0, iHitsInCDC);
  m_HitsCDC->GetXaxis()->SetTitle("# hits");
  m_HitsCDC->GetYaxis()->SetTitle("counts");
  name = str(format("NoOfHitsInTrack"));
  title = str(format("No Of Hits In Track"));
  m_Hits = new TH1F(name.c_str(), title.c_str(), iHits, 0, iHits);
  m_Hits->GetXaxis()->SetTitle("# hits");
  m_Hits->GetYaxis()->SetTitle("counts");

  name = str(format("NoOfTracksInVXDOnly"));
  title = str(format("No Of Tracks Per Event, Only In VXD"));
  m_TracksVXD = new TH1F(name.c_str(), title.c_str(), iTracks, 0, iTracks);
  m_TracksVXD->GetXaxis()->SetTitle("# tracks");
  m_TracksVXD->GetYaxis()->SetTitle("counts");
  name = str(format("NoOfTracksInCDCOnly"));
  title = str(format("No Of Tracks Per Event, Only In CDC"));
  m_TracksCDC = new TH1F(name.c_str(), title.c_str(), iTracks, 0, iTracks);
  m_TracksCDC->GetXaxis()->SetTitle("# tracks");
  m_TracksCDC->GetYaxis()->SetTitle("counts");
  name = str(format("NoOfTracksInVXDCDC"));
  title = str(format("No Of Tracks Per Event, In VXD+CDC"));
  m_TracksVXDCDC = new TH1F(name.c_str(), title.c_str(), iTracks, 0, iTracks);
  m_TracksVXDCDC->GetXaxis()->SetTitle("# tracks");
  m_TracksVXDCDC->GetYaxis()->SetTitle("counts");
  name = str(format("NoOfTracks"));
  title = str(format("No Of All Tracks Per Event"));
  m_Tracks = new TH1F(name.c_str(), title.c_str(), iTracks, 0, iTracks);
  m_Tracks->GetXaxis()->SetTitle("# tracks");
  m_Tracks->GetYaxis()->SetTitle("counts");

  DirTracksAlignment->cd();
  for (int i = 0; i < c_nPXDSensors + c_nSVDSensors; i++) {
    int iLayer = 0;
    int iLadder = 0;
    int iSensor = 0;
    getIDsFromIndex(i, iLayer, iLadder, iSensor);
    /** Unbiased residuals for PXD u vs v per sensor*/
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    name = str(format("UBResiduals_%1%") % sensorDescr);
    sensorDescr = str(format("Layer %1% Ladder %2% Sensor %3%") % iLayer % iLadder % iSensor);
    title = str(format("PXD Unbiased residuals for sensor %1%") % sensorDescr);
    m_UBResidualsSensor[i] = new TH2F(name.c_str(), title.c_str(), 200, -200, 200, 200, -200, 200);
    m_UBResidualsSensor[i]->GetXaxis()->SetTitle("u residual [#mum]");
    m_UBResidualsSensor[i]->GetYaxis()->SetTitle("v residual [#mum]");
    m_UBResidualsSensor[i]->GetZaxis()->SetTitle("counts");
  }

  oldDir->cd();

}

void TrackDQMModule::beginRun()
{
  if (m_PValue != NULL) m_PValue->Reset();
  if (m_Chi2 != NULL) m_Chi2->Reset();
  if (m_NDF != NULL) m_NDF->Reset();
  if (m_Chi2NDF != NULL) m_Chi2NDF->Reset();
  if (m_UBResidualsPXD != NULL) m_UBResidualsPXD->Reset();
  if (m_UBResidualsSVD != NULL) m_UBResidualsSVD->Reset();

  for (int i = 0; i < c_nVXDLayers; i++) {
    if (m_TRClusterHitmap[i] != NULL) m_TRClusterHitmap[i]->Reset();
  }
  for (int i = 0; i < c_nVXDLayers - 1; i++) {
    if (m_TRClusterCorrelationsPhi[i] != NULL) m_TRClusterCorrelationsPhi[i]->Reset();
    if (m_TRClusterCorrelationsTheta[i] != NULL) m_TRClusterCorrelationsTheta[i]->Reset();
  }
  for (int i = 0; i < c_nPXDSensors + c_nSVDSensors; i++) {
    if (m_UBResidualsSensor[i] != NULL) m_UBResidualsSensor[i]->Reset();
  }
  if (m_MomX != NULL) m_MomX->Reset();
  if (m_MomY != NULL) m_MomY->Reset();
  if (m_MomZ != NULL) m_MomZ->Reset();
  if (m_Mom != NULL) m_Mom->Reset();
  if (m_HitsPXD != NULL) m_HitsPXD->Reset();
  if (m_HitsSVD != NULL) m_HitsSVD->Reset();
  if (m_HitsCDC != NULL) m_HitsCDC->Reset();
  if (m_Hits != NULL) m_Hits->Reset();
  if (m_TracksVXD != NULL) m_TracksVXD->Reset();
  if (m_TracksCDC != NULL) m_TracksCDC->Reset();
  if (m_TracksVXDCDC != NULL) m_TracksVXDCDC->Reset();
  if (m_Tracks != NULL) m_Tracks->Reset();
}


void TrackDQMModule::event()
{
  int iTrack = 0;
  int iTrackVXD = 0;
  int iTrackCDC = 0;
  int iTrackVXDCDC = 0;

  StoreArray<Track> tracks;
  for (const Track& track : tracks) {  // over tracks
    RelationVector<RecoTrack> theRC = DataStore::getRelationsWithObj<RecoTrack>(&track);
    RelationVector<PXDCluster> pxdClustersTrack = DataStore::getRelationsWithObj<PXDCluster>(theRC[0]);
    int nPXD = (int)pxdClustersTrack.size();
    RelationVector<SVDCluster> svdClustersTrack = DataStore::getRelationsWithObj<SVDCluster>(theRC[0]);
    int nSVD = (int)svdClustersTrack.size();
    RelationVector<CDCHit> cdcHitTrack = DataStore::getRelationsWithObj<CDCHit>(theRC[0]);
    int nCDC = (int)cdcHitTrack.size();
    const TrackFitResult* tfr = track.getTrackFitResult(Const::pion);
    if (tfr == nullptr) continue;
    TString message = Form("TrackDQM: track %3i, Mom: %f, %f, %f, Pt: %f, Mag: %f, Hits: PXD %i SVD %i CDC %i Suma %i\n",
                           iTrack,
                           (float)tfr->getMomentum().Px(),
                           (float)tfr->getMomentum().Py(),
                           (float)tfr->getMomentum().Pz(),
                           (float)tfr->getMomentum().Pt(),
                           (float)tfr->getMomentum().Mag(),
                           nPXD, nSVD, nCDC, nPXD + nSVD + nCDC
                          );
    B2DEBUG(230, message.Data());
    iTrack++;
    float Chi2NDF = 0;
    float NDF = 0;
    float pValue = 0;
    if (theRC[0]->wasFitSuccessful()) {
      if (!theRC[0]->getTrackFitStatus())
        continue;
      // add NDF:
      NDF = theRC[0]->getTrackFitStatus()->getNdf();
      m_NDF->Fill(NDF);
      // add Chi2/NDF:
      m_Chi2->Fill(theRC[0]->getTrackFitStatus()->getChi2());
      if (NDF) {
        Chi2NDF = theRC[0]->getTrackFitStatus()->getChi2() / NDF;
        m_Chi2NDF->Fill(Chi2NDF);
      }
      // add p-value:
      pValue = theRC[0]->getTrackFitStatus()->getPVal();
      m_PValue->Fill(pValue);
      // add residuals:
      int iHit = 0;
      int iHitPrew = 0;

      VxdID sensorIDPrew;

      float ResidUPlaneRHUnBias = 0;
      float ResidVPlaneRHUnBias = 0;
      float fPosSPUPrev = 0;
      float fPosSPVPrev = 0;
      float fPosSPU = 0;
      float fPosSPV = 0;
      int iLayerPrev = 0;
      int iLayer = 0;

      int IsSVDU = -1;
      for (auto recoHitInfo : theRC[0]->getRecoHitInformations()) {  // over recohits
        if (!recoHitInfo) {
          B2DEBUG(200, "No genfit::pxd recoHitInfo is missing.");
          continue;
        }
        if (!recoHitInfo->useInFit())
          continue;
        if (!((recoHitInfo->getTrackingDetector() == RecoHitInformation::c_PXD) ||
              (recoHitInfo->getTrackingDetector() == RecoHitInformation::c_SVD)))
          continue;

        auto& genfitTrack = RecoTrackGenfitAccess::getGenfitTrack(*theRC[0]);

        bool biased = false;
        TVectorD resUnBias = genfitTrack.getPointWithMeasurement(iHit)->getFitterInfo()->getResidual(0, biased).getState();
        IsSVDU = -1;
        if (recoHitInfo->getTrackingDetector() == RecoHitInformation::c_PXD) {
          TVector3 rLocal(recoHitInfo->getRelatedTo<PXDCluster>()->getU(), recoHitInfo->getRelatedTo<PXDCluster>()->getV(), 0);
          VxdID sensorID = recoHitInfo->getRelatedTo<PXDCluster>()->getSensorID();
          auto info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
          iLayer = sensorID.getLayerNumber();
          TVector3 ral = info.pointToGlobal(rLocal);
          fPosSPU = ral.Phi() / TMath::Pi() * 180;
          fPosSPV = ral.Theta() / TMath::Pi() * 180;
          ResidUPlaneRHUnBias = resUnBias.GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
          ResidVPlaneRHUnBias = resUnBias.GetMatrixArray()[1] * Unit::convertValueToUnit(1.0, "um");
          if ((iHitPrew < iHit) && (fPosSPUPrev != 0) && (fPosSPVPrev != 0) && ((iLayer - iLayerPrev) == 1)) {
            m_TRClusterCorrelationsPhi[getLayerIndex(iLayerPrev)]->Fill(fPosSPUPrev, fPosSPU);
            m_TRClusterCorrelationsTheta[getLayerIndex(iLayerPrev)]->Fill(fPosSPVPrev, fPosSPV);
            iHitPrew = iHit;
          }
          iLayerPrev = iLayer;
          fPosSPUPrev = fPosSPU;
          fPosSPVPrev = fPosSPV;
          m_UBResidualsPXD->Fill(ResidUPlaneRHUnBias, ResidVPlaneRHUnBias);
          int index = getSensorIndex(iLayer, sensorID.getLadderNumber(), sensorID.getSensorNumber());
          m_UBResidualsSensor[index]->Fill(ResidUPlaneRHUnBias, ResidVPlaneRHUnBias);
          m_TRClusterHitmap[getLayerIndex(iLayer)]->Fill(fPosSPU, fPosSPV);
        }
        if (recoHitInfo->getTrackingDetector() == RecoHitInformation::c_SVD) {
          IsSVDU = recoHitInfo->getRelatedTo<SVDCluster>()->isUCluster();
          VxdID sensorID = recoHitInfo->getRelatedTo<SVDCluster>()->getSensorID();
          auto info = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
          iLayer = sensorID.getLayerNumber();
          if (IsSVDU) {
            TVector3 rLocal(recoHitInfo->getRelatedTo<SVDCluster>()->getPosition(), 0 , 0);
            TVector3 ral = info.pointToGlobal(rLocal);
            fPosSPU = ral.Phi() / TMath::Pi() * 180;
            ResidUPlaneRHUnBias = resUnBias.GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
            if (sensorIDPrew != sensorID) { // other sensor, reset
              ResidVPlaneRHUnBias = 0;
              fPosSPV = 0;
            }
            sensorIDPrew = sensorID;
          } else {
            TVector3 rLocal(0, recoHitInfo->getRelatedTo<SVDCluster>()->getPosition(), 0);
            TVector3 ral = info.pointToGlobal(rLocal);
            fPosSPV = ral.Theta() / TMath::Pi() * 180;
            ResidVPlaneRHUnBias = resUnBias.GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
            if (sensorIDPrew == sensorID) { // evaluate
              if ((iHitPrew < iHit) && (fPosSPUPrev != 0) && (fPosSPVPrev != 0) && ((iLayer - iLayerPrev) == 1)) {
                m_TRClusterCorrelationsPhi[getLayerIndex(iLayerPrev)]->Fill(fPosSPUPrev, fPosSPU);
                m_TRClusterCorrelationsTheta[getLayerIndex(iLayerPrev)]->Fill(fPosSPVPrev, fPosSPV);
                iHitPrew = iHit;
              }
              iLayerPrev = iLayer;
              fPosSPUPrev = fPosSPU;
              fPosSPVPrev = fPosSPV;
              m_UBResidualsSVD->Fill(ResidUPlaneRHUnBias, ResidVPlaneRHUnBias);
              int index = getSensorIndex(iLayer, sensorID.getLadderNumber(), sensorID.getSensorNumber());
              m_UBResidualsSensor[index]->Fill(ResidUPlaneRHUnBias, ResidVPlaneRHUnBias);
              m_TRClusterHitmap[getLayerIndex(iLayer)]->Fill(fPosSPU, fPosSPV);
            }
            if (sensorIDPrew != sensorID) { // other sensor, reset
              ResidUPlaneRHUnBias = 0;
              fPosSPU = 0;
            }
            sensorIDPrew = sensorID;
          }
        }
        iHit++;
      }
    }
    if (((nPXD > 0) || (nSVD > 0)) && (nCDC > 0)) iTrackVXDCDC++;
    if (((nPXD > 0) || (nSVD > 0)) && (nCDC == 0)) iTrackVXD++;
    if (((nPXD == 0) && (nSVD == 0)) && (nCDC > 0)) iTrackCDC++;
    if (m_MomX != NULL) m_MomX->Fill(tfr->getMomentum().Px());
    if (m_MomY != NULL) m_MomY->Fill(tfr->getMomentum().Py());
    if (m_MomZ != NULL) m_MomZ->Fill(tfr->getMomentum().Pz());
    if (m_MomPt != NULL) m_MomPt->Fill(tfr->getMomentum().Pt());
    if (m_Mom != NULL) m_Mom->Fill(tfr->getMomentum().Mag());
    if (m_HitsPXD != NULL) m_HitsPXD->Fill(nPXD);
    if (m_HitsSVD != NULL) m_HitsSVD->Fill(nSVD);
    if (m_HitsCDC != NULL) m_HitsCDC->Fill(nCDC);
    if (m_Hits != NULL) m_Hits->Fill(nPXD + nSVD + nCDC);
  }
  if (m_TracksVXD != NULL) m_TracksVXD->Fill(iTrackVXD);
  if (m_TracksCDC != NULL) m_TracksCDC->Fill(iTrackCDC);
  if (m_TracksVXDCDC != NULL) m_TracksVXDCDC->Fill(iTrackVXDCDC);
  if (m_Tracks != NULL) m_Tracks->Fill(iTrack);
}


void TrackDQMModule::endRun()
{
}


void TrackDQMModule::terminate()
{
}


int TrackDQMModule::getLayerIndex(const int Layer) const
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  for (VxdID layer : geo.getLayers()) {
    if (Layer == layer.getLayerNumber()) {
      return tempcounter;
    }
    tempcounter++;
  }
  return tempcounter;
}

void TrackDQMModule::getLayerIDsFromLayerIndex(const int Index, int& Layer) const
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  for (VxdID layer : geo.getLayers()) {
    if (tempcounter == Index) {
      Layer = layer.getLayerNumber();
      return;
    }
    tempcounter++;
  }
}

int TrackDQMModule::getSensorIndex(const int Layer, const int Ladder, const int Sensor) const
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  for (VxdID layer : geo.getLayers()) {
    // if (layer.getLayerNumber() <= c_lastPXDLayer) continue;  // need SVD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if ((Layer == layer.getLayerNumber()) &&
            (Ladder == ladder.getLadderNumber()) &&
            (Sensor == sensor.getSensorNumber())) {
          return tempcounter;
        }
        tempcounter++;
      }
    }
  }
  return tempcounter;
}

void TrackDQMModule::getIDsFromIndex(const int Index, int& Layer, int& Ladder, int& Sensor) const
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  for (VxdID layer : geo.getLayers()) {
    // if (layer.getLayerNumber() <= c_lastPXDLayer) continue;  // need SVD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if (tempcounter == Index) {
          Layer = layer.getLayerNumber();
          Ladder = ladder.getLadderNumber();
          Sensor = sensor.getSensorNumber();
          return;
        }
        tempcounter++;
      }
    }
  }
}

