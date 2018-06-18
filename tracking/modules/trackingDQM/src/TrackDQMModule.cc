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

#include <tracking/modules/trackingDQM/TrackDQMModule.h>

#include <genfit/MeasurementOnPlane.h>
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

#include <vxd/geometry/GeoTools.h>

//#include <framework/database/DBObjPtr.h>

#include <algorithm>
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
  StoreArray<RecoTrack> recoTracks(m_RecoTracksStoreArrayName);
  if (!recoTracks.isOptional()) {
    B2WARNING("Missing recoTracks array, Track-DQM is skipped.");
    return;
  }
  StoreArray<Track> Tracks(m_TracksStoreArrayName);
  if (!Tracks.isOptional()) {
    B2WARNING("Missing Tracks array, Track-DQM is skipped.");
    return;
  }

  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

}

void TrackDQMModule::defineHisto()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfLayers() == 0) {
    B2WARNING("Missing geometry for VXD.");
  }

  // basic constants presets:
  int nVXDLayers = gTools->getNumberOfLayers();
  int nVXDSensors = gTools->getNumberOfSensors();
  float ResidualRange = 400;  // in um
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;
  TDirectory* DirTracks = NULL;
  DirTracks = oldDir->mkdir("TracksDQM");
  TDirectory* DirTracksAlignment = NULL;
  DirTracksAlignment = oldDir->mkdir("TracksDQMAlignment");
  DirTracks->cd();
  // Momentum Phi
  string name = str(format("MomPhi"));
  string title = str(format("Momentum Phi of fit"));
  m_MomPhi = new TH1F(name.c_str(), title.c_str(), 180, -180, 180);
  m_MomPhi->GetXaxis()->SetTitle("Mom Phi [deg]");
  m_MomPhi->GetYaxis()->SetTitle("counts");
  // Momentum CosTheta
  name = str(format("MomCosTheta"));
  title = str(format("Cos of Momentum Theta of fit"));
  m_MomCosTheta = new TH1F(name.c_str(), title.c_str(), 100, -1, 1);
  m_MomCosTheta->GetXaxis()->SetTitle("Mom CosTheta");
  m_MomCosTheta->GetYaxis()->SetTitle("counts");

  /** p Value */
  name = str(format("PValue"));
  title = str(format("P value of fit"));
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
  m_UBResidualsPXD = new TH2F(name.c_str(), title.c_str(), 200, -ResidualRange, ResidualRange, 200, -ResidualRange, ResidualRange);
  m_UBResidualsPXD->GetXaxis()->SetTitle("u residual [#mum]");
  m_UBResidualsPXD->GetYaxis()->SetTitle("v residual [#mum]");
  m_UBResidualsPXD->GetZaxis()->SetTitle("counts");
  /** Unbiased residuals for SVD u vs v */
  name = str(format("UBResidualsSVD"));
  title = str(format("Unbiased residuals for SVD"));
  m_UBResidualsSVD = new TH2F(name.c_str(), title.c_str(), 200, -ResidualRange, ResidualRange, 200, -ResidualRange, ResidualRange);
  m_UBResidualsSVD->GetXaxis()->SetTitle("u residual [#mum]");
  m_UBResidualsSVD->GetYaxis()->SetTitle("v residual [#mum]");
  m_UBResidualsSVD->GetZaxis()->SetTitle("counts");
  /** Unbiased residuals for PXD u, v */
  name = str(format("UBResidualsPXDU"));
  title = str(format("Unbiased residuals in U for PXD"));
  m_UBResidualsPXDU = new TH1F(name.c_str(), title.c_str(), 200, -ResidualRange, ResidualRange);
  m_UBResidualsPXDU->GetXaxis()->SetTitle("residual [#mum]");
  m_UBResidualsPXDU->GetYaxis()->SetTitle("counts");
  name = str(format("UBResidualsPXDV"));
  title = str(format("Unbiased residuals in V for PXD"));
  m_UBResidualsPXDV = new TH1F(name.c_str(), title.c_str(), 200, -ResidualRange, ResidualRange);
  m_UBResidualsPXDV->GetXaxis()->SetTitle("residual [#mum]");
  m_UBResidualsPXDV->GetYaxis()->SetTitle("counts");
  /** Unbiased residuals for SVD u, v */
  name = str(format("UBResidualsSVDU"));
  title = str(format("Unbiased residuals in U for SVD"));
  m_UBResidualsSVDU = new TH1F(name.c_str(), title.c_str(), 200, -ResidualRange, ResidualRange);
  m_UBResidualsSVDU->GetXaxis()->SetTitle("residual [#mum]");
  m_UBResidualsSVDU->GetYaxis()->SetTitle("counts");
  name = str(format("UBResidualsSVDV"));
  title = str(format("Unbiased residuals in V for SVD"));
  m_UBResidualsSVDV = new TH1F(name.c_str(), title.c_str(), 200, -ResidualRange, ResidualRange);
  m_UBResidualsSVDV->GetXaxis()->SetTitle("residual [#mum]");
  m_UBResidualsSVDV->GetYaxis()->SetTitle("counts");

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
  float fMomRange = 6.0;
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
  name = str(format("TrackZ0"));
  title = str(format("z0 - the z coordinate of the perigee (beam spot position)"));
  m_Z0 = new TH1F(name.c_str(), title.c_str(), 200, -10.0, 10.0);
  m_Z0->GetXaxis()->SetTitle("z0 [cm]");
  m_Z0->GetYaxis()->SetTitle("Arb. Units");
  name = str(format("TrackD0"));
  title = str(format("d0 - the signed distance to the IP in the r-phi plane"));
  m_D0 = new TH1F(name.c_str(), title.c_str(), 200, -1.0, 1.0);
  m_D0->GetXaxis()->SetTitle("d0 [cm]");
  m_D0->GetYaxis()->SetTitle("Arb. Units");
  name = str(format("TrackD0Phi"));
  title = str(format("d0 vs Phi - the signed distance to the IP in the r-phi plane"));
  m_D0Phi = new TH2F(name.c_str(), title.c_str(), 72, -180.0, 180.0, 80, -0.4, 0.4);
  m_D0Phi->GetXaxis()->SetTitle("#phi0 [deg]");
  m_D0Phi->GetYaxis()->SetTitle("d0 [cm]");
  m_D0Phi->GetZaxis()->SetTitle("Arb. Units");
  name = str(format("TrackD0Z0"));
  title = str(
            format("z0 vs d0 - signed distance to the IP in r-phi vs. z0 of the perigee (to see primary vertex shifts along R or z)"));
  m_D0Z0 = new TH2F(name.c_str(), title.c_str(), 200, -10.0, 10.0, 80, -0.4, 0.4);
  m_D0Z0->GetXaxis()->SetTitle("z0 [cm]");
  m_D0Z0->GetYaxis()->SetTitle("d0 [cm]");
  m_D0Z0->GetZaxis()->SetTitle("Arb. Units");

  name = str(format("TrackPhi"));
  title = str(format("Phi - angle of the transverse momentum in the r-phi plane, with CDF naming convention"));
  m_Phi = new TH1F(name.c_str(), title.c_str(), 72, -180.0, 180.0);
  m_Phi->GetXaxis()->SetTitle("#phi [deg]");
  m_Phi->GetYaxis()->SetTitle("Arb. Units");
  name = str(format("TrackTanLambda"));
  title = str(format("TanLambda - the slope of the track in the r-z plane"));
  m_TanLambda = new TH1F(name.c_str(), title.c_str(), 400, -4.0, 4.0);
  m_TanLambda->GetXaxis()->SetTitle("Tan Lambda");
  m_TanLambda->GetYaxis()->SetTitle("Arb. Units");
  name = str(format("TrackOmega"));
  title = str(format("Omega - the curvature of the track. It's sign is defined by the charge of the particle"));
  m_Omega = new TH1F(name.c_str(), title.c_str(), 400, -0.1, 0.1);
  m_Omega->GetXaxis()->SetTitle("Omega");
  m_Omega->GetYaxis()->SetTitle("Arb. Units");

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

  if (gTools->getNumberOfLayers() == 0) {
    B2WARNING("Missing geometry for VXD, VXD-DQM related are skiped.");
    return;
  }

  m_TRClusterHitmap = (TH2F**) new TH2F*[nVXDLayers];
  m_TRClusterCorrelationsPhi = (TH2F**) new TH2F*[nVXDLayers - 1];
  m_TRClusterCorrelationsTheta = (TH2F**) new TH2F*[nVXDLayers - 1];
  m_UBResidualsSensor = (TH2F**) new TH2F*[nVXDSensors];
  m_UBResidualsSensorU = (TH1F**) new TH1F*[nVXDSensors];
  m_UBResidualsSensorV = (TH1F**) new TH1F*[nVXDSensors];

  for (VxdID layer : geo.getLayers()) {
    int i = layer.getLayerNumber();
    int index = gTools->getLayerIndex(layer.getLayerNumber());
    /** Track related clusters - hitmap in IP angle range */
    name = str(format("TRClusterHitmapLayer%1%") % i);
    title = str(format("Cluster Hitmap for layer %1%") % i);
    m_TRClusterHitmap[index] = new TH2F(name.c_str(), title.c_str(), 360, -180.0, 180.0, 180, 0.0, 180.0);
    m_TRClusterHitmap[index]->GetXaxis()->SetTitle("Phi angle [deg]");
    m_TRClusterHitmap[index]->GetYaxis()->SetTitle("Theta angle [deg]");
    m_TRClusterHitmap[index]->GetZaxis()->SetTitle("counts");
  }
  for (VxdID layer : geo.getLayers()) {
    int i = layer.getLayerNumber();
    if (i == gTools->getLastLayer()) continue;
    int index = gTools->getLayerIndex(layer.getLayerNumber());
    /** Track related clusters - neighbor corelations in Phi */
    name = str(format("CorrelationsPhiLayers_%1%_%2%") % i % (i + 1));
    title = str(format("Correlations in Phi for Layers %1% %2%") % i % (i + 1));
    m_TRClusterCorrelationsPhi[index] = new TH2F(name.c_str(), title.c_str(), 360, -180.0, 180.0, 360, -180.0, 180.0);
    title = str(format("angle layer %1% [deg]") % i);
    m_TRClusterCorrelationsPhi[index]->GetXaxis()->SetTitle(title.c_str());
    title = str(format("angle layer %1% [deg]") % (i + 1));
    m_TRClusterCorrelationsPhi[index]->GetYaxis()->SetTitle(title.c_str());
    m_TRClusterCorrelationsPhi[index]->GetZaxis()->SetTitle("counts");
    /** Track related clusters - neighbor corelations in Theta */
    name = str(format("CorrelationsThetaLayers_%1%_%2%") % i % (i + 1));
    title = str(format("Correlations in Theta for Layers %1% %2%") % i % (i + 1));
    m_TRClusterCorrelationsTheta[index] = new TH2F(name.c_str(), title.c_str(), 180, 0.0, 180.0, 180, 0.0, 180.0);
    title = str(format("angle layer %1% [deg]") % i);
    m_TRClusterCorrelationsTheta[index]->GetXaxis()->SetTitle(title.c_str());
    title = str(format("angle layer %1% [deg]") % (i + 1));
    m_TRClusterCorrelationsTheta[index]->GetYaxis()->SetTitle(title.c_str());
    m_TRClusterCorrelationsTheta[index]->GetZaxis()->SetTitle("counts");
  }

  DirTracksAlignment->cd();
  for (int i = 0; i < nVXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    /** Unbiased residuals for PXD u vs v per sensor*/
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    name = str(format("UBResidualsU_%1%") % sensorDescr);
    sensorDescr = str(format("Layer %1% Ladder %2% Sensor %3%") % iLayer % iLadder % iSensor);
    title = str(format("PXD Unbiased U residuals for sensor %1%") % sensorDescr);
    m_UBResidualsSensorU[i] = new TH1F(name.c_str(), title.c_str(), 200, -ResidualRange, ResidualRange);
    m_UBResidualsSensorU[i]->GetXaxis()->SetTitle("residual [#mum]");
    m_UBResidualsSensorU[i]->GetYaxis()->SetTitle("counts");
    sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    name = str(format("UBResidualsV_%1%") % sensorDescr);
    sensorDescr = str(format("Layer %1% Ladder %2% Sensor %3%") % iLayer % iLadder % iSensor);
    title = str(format("PXD Unbiased V residuals for sensor %1%") % sensorDescr);
    m_UBResidualsSensorV[i] = new TH1F(name.c_str(), title.c_str(), 200, -ResidualRange, ResidualRange);
    m_UBResidualsSensorV[i]->GetXaxis()->SetTitle("residual [#mum]");
    m_UBResidualsSensorV[i]->GetYaxis()->SetTitle("counts");
    sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    name = str(format("UBResiduals_%1%") % sensorDescr);
    sensorDescr = str(format("Layer %1% Ladder %2% Sensor %3%") % iLayer % iLadder % iSensor);
    title = str(format("PXD Unbiased residuals for sensor %1%") % sensorDescr);
    m_UBResidualsSensor[i] = new TH2F(name.c_str(), title.c_str(), 200, -ResidualRange, ResidualRange, 200, -ResidualRange,
                                      ResidualRange);
    m_UBResidualsSensor[i]->GetXaxis()->SetTitle("residual U [#mum]");
    m_UBResidualsSensor[i]->GetYaxis()->SetTitle("residual V [#mum]");
    m_UBResidualsSensor[i]->GetZaxis()->SetTitle("counts");
  }

  oldDir->cd();

}

void TrackDQMModule::beginRun()
{
  StoreArray<RecoTrack> recoTracks(m_RecoTracksStoreArrayName);
  if (!recoTracks.isOptional())  return;
  StoreArray<Track> Tracks(m_TracksStoreArrayName);
  if (!Tracks.isOptional()) return;

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  if (m_MomPhi != NULL) m_MomPhi->Reset();
  if (m_MomCosTheta != NULL) m_MomCosTheta->Reset();
  if (m_PValue != NULL) m_PValue->Reset();
  if (m_Chi2 != NULL) m_Chi2->Reset();
  if (m_NDF != NULL) m_NDF->Reset();
  if (m_Chi2NDF != NULL) m_Chi2NDF->Reset();
  if (m_UBResidualsPXD != NULL) m_UBResidualsPXD->Reset();
  if (m_UBResidualsSVD != NULL) m_UBResidualsSVD->Reset();
  if (m_UBResidualsPXDU != NULL) m_UBResidualsPXDU->Reset();
  if (m_UBResidualsSVDU != NULL) m_UBResidualsSVDU->Reset();
  if (m_UBResidualsPXDV != NULL) m_UBResidualsPXDV->Reset();
  if (m_UBResidualsSVDV != NULL) m_UBResidualsSVDV->Reset();

  if (m_MomX != NULL) m_MomX->Reset();
  if (m_MomY != NULL) m_MomY->Reset();
  if (m_MomZ != NULL) m_MomZ->Reset();
  if (m_Mom != NULL) m_Mom->Reset();
  if (m_D0 != NULL) m_D0->Reset();
  if (m_D0Phi != NULL) m_D0Phi->Reset();
  if (m_D0Z0 != NULL) m_D0Z0->Reset();

  if (m_Z0 != NULL) m_Z0->Reset();
  if (m_Phi != NULL) m_Phi->Reset();
  if (m_TanLambda != NULL) m_TanLambda->Reset();
  if (m_Omega != NULL) m_Omega->Reset();
  if (m_HitsPXD != NULL) m_HitsPXD->Reset();
  if (m_HitsSVD != NULL) m_HitsSVD->Reset();
  if (m_HitsCDC != NULL) m_HitsCDC->Reset();
  if (m_Hits != NULL) m_Hits->Reset();
  if (m_TracksVXD != NULL) m_TracksVXD->Reset();
  if (m_TracksCDC != NULL) m_TracksCDC->Reset();
  if (m_TracksVXDCDC != NULL) m_TracksVXDCDC->Reset();
  if (m_Tracks != NULL) m_Tracks->Reset();

  if (gTools->getNumberOfLayers() == 0) return;

  for (VxdID layer : geo.getLayers()) {
    int i = gTools->getLayerIndex(layer.getLayerNumber());
    if (m_TRClusterHitmap[i] != NULL) m_TRClusterHitmap[i]->Reset();
  }

  for (VxdID layer : geo.getLayers()) {
    int i = layer.getLayerNumber();
    if (i == gTools->getLastLayer()) continue;
    i = gTools->getLayerIndex(i);
    if (m_TRClusterCorrelationsPhi[i] != NULL) m_TRClusterCorrelationsPhi[i]->Reset();
    if (m_TRClusterCorrelationsTheta[i] != NULL) m_TRClusterCorrelationsTheta[i]->Reset();
  }
  for (int i = 0; i < gTools->getNumberOfSensors(); i++) {
    if (m_UBResidualsSensor[i] != NULL) m_UBResidualsSensor[i]->Reset();
    if (m_UBResidualsSensorU[i] != NULL) m_UBResidualsSensorU[i]->Reset();
    if (m_UBResidualsSensorV[i] != NULL) m_UBResidualsSensorV[i]->Reset();
  }
}


void TrackDQMModule::event()
{
  StoreArray<RecoTrack> recoTracks(m_RecoTracksStoreArrayName);
  if (!recoTracks.isOptional() || !recoTracks.getEntries())  return;
  StoreArray<Track> tracks(m_TracksStoreArrayName);
  if (!tracks.isOptional() || !tracks.getEntries()) return;

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  try {
    int iTrack = 0;
    int iTrackVXD = 0;
    int iTrackCDC = 0;
    int iTrackVXDCDC = 0;

    for (const Track& track : tracks) {  // over tracks
      RelationVector<RecoTrack> recoTrack = track.getRelationsTo<RecoTrack>(m_RecoTracksStoreArrayName);
      if (!recoTrack.size()) continue;
      RelationVector<PXDCluster> pxdClustersTrack = DataStore::getRelationsWithObj<PXDCluster>(recoTrack[0]);
      int nPXD = (int)pxdClustersTrack.size();
      RelationVector<SVDCluster> svdClustersTrack = DataStore::getRelationsWithObj<SVDCluster>(recoTrack[0]);
      int nSVD = (int)svdClustersTrack.size();
      RelationVector<CDCHit> cdcHitTrack = DataStore::getRelationsWithObj<CDCHit>(recoTrack[0]);
      int nCDC = (int)cdcHitTrack.size();
      const TrackFitResult* tfr = track.getTrackFitResultWithClosestMass(Const::pion);
      /*
      const auto& resmap = track.getTrackFitResults();
      auto hypot = max_element(
      resmap.begin(),
      resmap.end(),
      [](const pair<Const::ChargedStable, const TrackFitResult*>& x1, const pair<Const::ChargedStable, const TrackFitResult*>& x2)->bool
      {return x1.second->getPValue() < x2.second->getPValue();}
      );
      const TrackFitResult* tfr = hypot->second;
      */
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

      float Phi = 90;  // in deg
      if (fabs(tfr->getMomentum().Px()) > 0.00000001) {
        Phi = atan2(tfr->getMomentum().Py(), tfr->getMomentum().Px()) * TMath::RadToDeg();
      }
      float pxy = sqrt(tfr->getMomentum().Px() * tfr->getMomentum().Px() + tfr->getMomentum().Py() * tfr->getMomentum().Py());
      float Theta = TMath::Pi() / 2.0;  // in rad
      if (fabs(tfr->getMomentum().Pz()) > 0.00000001) {
        Theta = atan2(pxy, tfr->getMomentum().Pz());
      }
      m_MomPhi->Fill(Phi);
//      m_MomCosTheta->Fill(cos(Theta - (TMath::Pi() / 2.0)));
      m_MomCosTheta->Fill(cos(Theta));

      float Chi2NDF = 0;
      float NDF = 0;
      float pValue = 0;
      if (recoTrack[0]->wasFitSuccessful()) {
        if (!recoTrack[0]->getTrackFitStatus())
          continue;

        // add NDF:
        NDF = recoTrack[0]->getTrackFitStatus()->getNdf();
        m_NDF->Fill(NDF);
        // add Chi2/NDF:
        m_Chi2->Fill(recoTrack[0]->getTrackFitStatus()->getChi2());
        if (NDF) {
          Chi2NDF = recoTrack[0]->getTrackFitStatus()->getChi2() / NDF;
          m_Chi2NDF->Fill(Chi2NDF);
        }
        // add p-value:
        pValue = recoTrack[0]->getTrackFitStatus()->getPVal();
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
        for (auto recoHitInfo : recoTrack[0]->getRecoHitInformations()) {  // over recohits
          if (!recoHitInfo) {
            B2DEBUG(200, "No genfit::pxd recoHitInfo is missing.");
            continue;
          }
          if (!recoHitInfo->useInFit())
            continue;
          if (!((recoHitInfo->getTrackingDetector() == RecoHitInformation::c_PXD) ||
                (recoHitInfo->getTrackingDetector() == RecoHitInformation::c_SVD)))
            continue;

          auto& genfitTrack = RecoTrackGenfitAccess::getGenfitTrack(*recoTrack[0]);

          bool biased = false;
          if (!genfitTrack.getPointWithMeasurement(iHit)->getFitterInfo()) continue;
          TVectorD resUnBias = genfitTrack.getPointWithMeasurement(iHit)->getFitterInfo()->getResidual(0, biased).getState();
          IsSVDU = -1;
          if (recoHitInfo->getTrackingDetector() == RecoHitInformation::c_PXD) {
            TVector3 rLocal(recoHitInfo->getRelatedTo<PXDCluster>()->getU(), recoHitInfo->getRelatedTo<PXDCluster>()->getV(), 0);
            VxdID sensorID = recoHitInfo->getRelatedTo<PXDCluster>()->getSensorID();
            auto info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
            iLayer = sensorID.getLayerNumber();
            TVector3 ral = info.pointToGlobal(rLocal, true);
            fPosSPU = ral.Phi() / TMath::Pi() * 180;
            fPosSPV = ral.Theta() / TMath::Pi() * 180;
            ResidUPlaneRHUnBias = resUnBias.GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
            ResidVPlaneRHUnBias = resUnBias.GetMatrixArray()[1] * Unit::convertValueToUnit(1.0, "um");
            if ((iHitPrew < iHit) && (fPosSPUPrev != 0) && (fPosSPVPrev != 0) && ((iLayer - iLayerPrev) == 1)) {
              int index = gTools->getLayerIndex(sensorID.getLayerNumber()) - gTools->getFirstLayer();
              m_TRClusterCorrelationsPhi[index]->Fill(fPosSPUPrev, fPosSPU);
              m_TRClusterCorrelationsTheta[index]->Fill(fPosSPVPrev, fPosSPV);
              iHitPrew = iHit;
            }
            iLayerPrev = iLayer;
            fPosSPUPrev = fPosSPU;
            fPosSPVPrev = fPosSPV;
            m_UBResidualsPXD->Fill(ResidUPlaneRHUnBias, ResidVPlaneRHUnBias);
            m_UBResidualsPXDU->Fill(ResidUPlaneRHUnBias);
            m_UBResidualsPXDV->Fill(ResidVPlaneRHUnBias);
            int index = gTools->getSensorIndex(sensorID);
            m_UBResidualsSensor[index]->Fill(ResidUPlaneRHUnBias, ResidVPlaneRHUnBias);
            m_UBResidualsSensorU[index]->Fill(ResidUPlaneRHUnBias);
            m_UBResidualsSensorV[index]->Fill(ResidVPlaneRHUnBias);
            m_TRClusterHitmap[gTools->getLayerIndex(sensorID.getLayerNumber())]->Fill(fPosSPU, fPosSPV);
          }
          if (recoHitInfo->getTrackingDetector() == RecoHitInformation::c_SVD) {
            IsSVDU = recoHitInfo->getRelatedTo<SVDCluster>()->isUCluster();
            VxdID sensorID = recoHitInfo->getRelatedTo<SVDCluster>()->getSensorID();
            auto info = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
            iLayer = sensorID.getLayerNumber();
            if (IsSVDU) {
              TVector3 rLocal(recoHitInfo->getRelatedTo<SVDCluster>()->getPosition(), 0 , 0);
              TVector3 ral = info.pointToGlobal(rLocal, true);
              fPosSPU = ral.Phi() / TMath::Pi() * 180;
              ResidUPlaneRHUnBias = resUnBias.GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
              if (sensorIDPrew != sensorID) { // other sensor, reset
                ResidVPlaneRHUnBias = 0;
                fPosSPV = 0;
              }
              sensorIDPrew = sensorID;
            } else {
              TVector3 rLocal(0, recoHitInfo->getRelatedTo<SVDCluster>()->getPosition(), 0);
              TVector3 ral = info.pointToGlobal(rLocal, true);
              fPosSPV = ral.Theta() / TMath::Pi() * 180;
              ResidVPlaneRHUnBias = resUnBias.GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
              if (sensorIDPrew == sensorID) { // evaluate
                if ((iHitPrew < iHit) && (fPosSPUPrev != 0) && (fPosSPVPrev != 0) && ((iLayer - iLayerPrev) == 1)) {
                  int index = gTools->getLayerIndex(sensorID.getLayerNumber()) - gTools->getFirstLayer();
                  m_TRClusterCorrelationsPhi[index]->Fill(fPosSPUPrev, fPosSPU);
                  m_TRClusterCorrelationsTheta[index]->Fill(fPosSPVPrev, fPosSPV);
                  iHitPrew = iHit;
                }
                iLayerPrev = iLayer;
                fPosSPUPrev = fPosSPU;
                fPosSPVPrev = fPosSPV;
                m_UBResidualsSVD->Fill(ResidUPlaneRHUnBias, ResidVPlaneRHUnBias);
                m_UBResidualsSVDU->Fill(ResidUPlaneRHUnBias);
                m_UBResidualsSVDV->Fill(ResidVPlaneRHUnBias);
                int index = gTools->getSensorIndex(sensorID);
                m_UBResidualsSensor[index]->Fill(ResidUPlaneRHUnBias, ResidVPlaneRHUnBias);
                m_UBResidualsSensorU[index]->Fill(ResidUPlaneRHUnBias);
                m_UBResidualsSensorV[index]->Fill(ResidVPlaneRHUnBias);
                m_TRClusterHitmap[gTools->getLayerIndex(sensorID.getLayerNumber())]->Fill(fPosSPU, fPosSPV);
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
      if (m_D0 != NULL) m_D0->Fill(tfr->getD0());
      if (m_D0Phi != NULL) m_D0Phi->Fill(tfr->getPhi0() * Unit::convertValueToUnit(1.0, "deg"), tfr->getD0());
      if (m_Z0 != NULL) m_Z0->Fill(tfr->getZ0());
      if (m_D0Z0 != NULL) m_D0Z0->Fill(tfr->getZ0(), tfr->getD0());

      if (m_Phi != NULL) m_Phi->Fill(tfr->getPhi() * Unit::convertValueToUnit(1.0, "deg"));
      if (m_TanLambda != NULL) m_TanLambda->Fill(tfr->getTanLambda());
      if (m_Omega != NULL) m_Omega->Fill(tfr->getOmega());

      if (m_HitsPXD != NULL) m_HitsPXD->Fill(nPXD);
      if (m_HitsSVD != NULL) m_HitsSVD->Fill(nSVD);
      if (m_HitsCDC != NULL) m_HitsCDC->Fill(nCDC);
      if (m_Hits != NULL) m_Hits->Fill(nPXD + nSVD + nCDC);
    }
    if (m_TracksVXD != NULL) m_TracksVXD->Fill(iTrackVXD);
    if (m_TracksCDC != NULL) m_TracksCDC->Fill(iTrackCDC);
    if (m_TracksVXDCDC != NULL) m_TracksVXDCDC->Fill(iTrackVXDCDC);
    if (m_Tracks != NULL) m_Tracks->Fill(iTrack);
  } catch (...) {
    B2DEBUG(70, "Some problem in Track DQM module!");
  }
}
