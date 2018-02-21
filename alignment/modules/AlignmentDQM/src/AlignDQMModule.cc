/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for Alignment DQM                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/modules/AlignmentDQM/AlignDQMModule.h>

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

#include <TDirectory.h>
#include <TVectorD.h>

using namespace Belle2;
using namespace std;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(AlignDQM)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

AlignDQMModule::AlignDQMModule() : HistoModule()
{
  //Set module properties
  setDescription("DQM of Alignment for off line "
                 "residuals per sensor, layer, "
                 "keep also On-Line DQM from tracking: "
                 "their momentum, "
                 "Number of hits in tracks, "
                 "Number of tracks. "
                );

  setPropertyFlags(c_ParallelProcessingCertified);

}


AlignDQMModule::~AlignDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void AlignDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

}

void AlignDQMModule::defineHisto()
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
  TDirectory* DirAlign = NULL;
  DirAlign = oldDir->mkdir("AlignmentDQM");
  TDirectory* DirAlignSensors = NULL;
  DirAlignSensors = oldDir->mkdir("AlignmentDQMSensors");
  TDirectory* DirAlignSensResids = NULL;
  DirAlignSensResids = DirAlignSensors->mkdir("Residuals2D");
  TDirectory* DirAlignSensResids1D = NULL;
  DirAlignSensResids1D = DirAlignSensors->mkdir("Residuals1D");

  TDirectory* DirAlignSensResMeanUPosUV = NULL;
  DirAlignSensResMeanUPosUV = DirAlignSensors->mkdir("ResidMeanUPositUV");
  TDirectory* DirAlignSensResMeanVPosUV = NULL;
  DirAlignSensResMeanVPosUV = DirAlignSensors->mkdir("ResidMeanVPositUV");
  TDirectory* DirAlignSensResMeanUPosU = NULL;
  DirAlignSensResMeanUPosU = DirAlignSensors->mkdir("ResidMeanUPositU");
  TDirectory* DirAlignSensResMeanVPosU = NULL;
  DirAlignSensResMeanVPosU = DirAlignSensors->mkdir("ResidMeanVPositU");
  TDirectory* DirAlignSensResMeanUPosV = NULL;
  DirAlignSensResMeanUPosV = DirAlignSensors->mkdir("ResidMeanUPositV");
  TDirectory* DirAlignSensResMeanVPosV = NULL;
  DirAlignSensResMeanVPosV = DirAlignSensors->mkdir("ResidMeanVPositV");
  TDirectory* DirAlignSensResUPosU = NULL;
  DirAlignSensResUPosU = DirAlignSensors->mkdir("ResidUPositU");
  TDirectory* DirAlignSensResVPosU = NULL;
  DirAlignSensResVPosU = DirAlignSensors->mkdir("ResidVPositU");
  TDirectory* DirAlignSensResUPosV = NULL;
  DirAlignSensResUPosV = DirAlignSensors->mkdir("ResidUPositV");
  TDirectory* DirAlignSensResVPosV = NULL;
  DirAlignSensResVPosV = DirAlignSensors->mkdir("ResidVPositV");

  TDirectory* DirAlignLayers = NULL;
  DirAlignLayers = oldDir->mkdir("AlignmentDQMLayers");

  TDirectory* DirAlignLayerResMeanUPosUV = NULL;
  DirAlignLayerResMeanUPosUV = DirAlignLayers->mkdir("ResidLayerMeanUPositPhiTheta");
  TDirectory* DirAlignLayerResMeanVPosUV = NULL;
  DirAlignLayerResMeanVPosUV = DirAlignLayers->mkdir("ResidLayerMeanVPositPhiTheta");
  TDirectory* DirAlignLayerResMeanUPosU = NULL;
  DirAlignLayerResMeanUPosU = DirAlignLayers->mkdir("ResidLayerMeanUPositPhi");
  TDirectory* DirAlignLayerResMeanVPosU = NULL;
  DirAlignLayerResMeanVPosU = DirAlignLayers->mkdir("ResidLayerMeanVPositPhi");
  TDirectory* DirAlignLayerResMeanUPosV = NULL;
  DirAlignLayerResMeanUPosV = DirAlignLayers->mkdir("ResidLayerMeanUPositTheta");
  TDirectory* DirAlignLayerResMeanVPosV = NULL;
  DirAlignLayerResMeanVPosV = DirAlignLayers->mkdir("ResidLayerMeanVPositTheta");
  TDirectory* DirAlignLayerResUPosU = NULL;
  DirAlignLayerResUPosU = DirAlignLayers->mkdir("ResidLayerUPositPhi");
  TDirectory* DirAlignLayerResVPosU = NULL;
  DirAlignLayerResVPosU = DirAlignLayers->mkdir("ResidLayerVPositPhi");
  TDirectory* DirAlignLayerResUPosV = NULL;
  DirAlignLayerResUPosV = DirAlignLayers->mkdir("ResidLayerUPositTheta");
  TDirectory* DirAlignLayerResVPosV = NULL;
  DirAlignLayerResVPosV = DirAlignLayers->mkdir("ResidLayerVPositTheta");

  DirAlign->cd();
  // Momentum Phi
  string name = str(format("Alig_MomPhi"));
  string title = str(format("Momentum Phi of fit"));
  m_MomPhi = new TH1F(name.c_str(), title.c_str(), 180, -180, 180);
  m_MomPhi->GetXaxis()->SetTitle("Mom Phi [deg]");
  m_MomPhi->GetYaxis()->SetTitle("counts");
  // Momentum Theta
  name = str(format("Alig_MomTheta"));
  title = str(format("Momentum Theta of fit"));
  m_MomTheta = new TH1F(name.c_str(), title.c_str(), 90, 0, 180);
  m_MomTheta->GetXaxis()->SetTitle("Mom Theta [deg]");
  m_MomTheta->GetYaxis()->SetTitle("counts");
  // Momentum CosTheta
  name = str(format("Alig_MomCosTheta"));
  title = str(format("Cos of Momentum Theta of fit"));
  m_MomCosTheta = new TH1F(name.c_str(), title.c_str(), 100, -1, 1);
  m_MomCosTheta->GetXaxis()->SetTitle("Mom CosTheta");
  m_MomCosTheta->GetYaxis()->SetTitle("counts");

  /** p Value */
  name = str(format("Alig_PValue"));
  title = str(format("P value of fit"));
  m_PValue = new TH1F(name.c_str(), title.c_str(), 100, 0, 1);
  m_PValue->GetXaxis()->SetTitle("p value");
  m_PValue->GetYaxis()->SetTitle("counts");
  /** Chi2 */
  name = str(format("Alig_Chi2"));
  title = str(format("Chi2 of fit"));
  m_Chi2 = new TH1F(name.c_str(), title.c_str(), 200, 0, 150);
  m_Chi2->GetXaxis()->SetTitle("Chi2");
  m_Chi2->GetYaxis()->SetTitle("counts");
  /** NDF */
  name = str(format("Alig_NDF"));
  title = str(format("NDF of fit"));
  m_NDF = new TH1F(name.c_str(), title.c_str(), 200, 0, 200);
  m_NDF->GetXaxis()->SetTitle("NDF");
  m_NDF->GetYaxis()->SetTitle("counts");
  /** Chi2 / NDF */
  name = str(format("Alig_Chi2NDF"));
  title = str(format("Chi2 div NDF of fit"));
  m_Chi2NDF = new TH1F(name.c_str(), title.c_str(), 200, 0, 10);
  m_Chi2NDF->GetXaxis()->SetTitle("Chi2NDF");
  m_Chi2NDF->GetYaxis()->SetTitle("counts");

  /** Unbiased residuals for PXD u vs v */
  name = str(format("Alig_UBResidualsPXD"));
  title = str(format("Unbiased residuals for PXD"));
  m_UBResidualsPXD = new TH2F(name.c_str(), title.c_str(), 200, -ResidualRange, ResidualRange, 200, -ResidualRange, ResidualRange);
  m_UBResidualsPXD->GetXaxis()->SetTitle("u residual [#mum]");
  m_UBResidualsPXD->GetYaxis()->SetTitle("v residual [#mum]");
  m_UBResidualsPXD->GetZaxis()->SetTitle("counts");
  /** Unbiased residuals for SVD u vs v */
  name = str(format("Alig_UBResidualsSVD"));
  title = str(format("Unbiased residuals for SVD"));
  m_UBResidualsSVD = new TH2F(name.c_str(), title.c_str(), 200, -ResidualRange, ResidualRange, 200, -ResidualRange, ResidualRange);
  m_UBResidualsSVD->GetXaxis()->SetTitle("u residual [#mum]");
  m_UBResidualsSVD->GetYaxis()->SetTitle("v residual [#mum]");
  m_UBResidualsSVD->GetZaxis()->SetTitle("counts");
  /** Unbiased residuals for PXD u, v */
  name = str(format("Alig_UBResidualsPXDU"));
  title = str(format("Unbiased residuals in U for PXD"));
  m_UBResidualsPXDU = new TH1F(name.c_str(), title.c_str(), 200, -ResidualRange, ResidualRange);
  m_UBResidualsPXDU->GetXaxis()->SetTitle("residual [#mum]");
  m_UBResidualsPXDU->GetYaxis()->SetTitle("counts");
  name = str(format("Alig_UBResidualsPXDV"));
  title = str(format("Unbiased residuals in V for PXD"));
  m_UBResidualsPXDV = new TH1F(name.c_str(), title.c_str(), 200, -ResidualRange, ResidualRange);
  m_UBResidualsPXDV->GetXaxis()->SetTitle("residual [#mum]");
  m_UBResidualsPXDV->GetYaxis()->SetTitle("counts");
  /** Unbiased residuals for SVD u, v */
  name = str(format("Alig_UBResidualsSVDU"));
  title = str(format("Unbiased residuals in U for SVD"));
  m_UBResidualsSVDU = new TH1F(name.c_str(), title.c_str(), 200, -ResidualRange, ResidualRange);
  m_UBResidualsSVDU->GetXaxis()->SetTitle("residual [#mum]");
  m_UBResidualsSVDU->GetYaxis()->SetTitle("counts");
  name = str(format("Alig_UBResidualsSVDV"));
  title = str(format("Unbiased residuals in V for SVD"));
  m_UBResidualsSVDV = new TH1F(name.c_str(), title.c_str(), 200, -ResidualRange, ResidualRange);
  m_UBResidualsSVDV->GetXaxis()->SetTitle("residual [#mum]");
  m_UBResidualsSVDV->GetYaxis()->SetTitle("counts");

  if (gTools->getNumberOfLayers() == 0) {
    B2WARNING("Missing geometry for VXD, VXD-DQM related are skiped.");
    return;
  }

  m_TRClusterHitmap = (TH2F**) new TH2F*[nVXDLayers];
  m_TRClusterCorrelationsPhi = (TH2F**) new TH2F*[nVXDLayers - 1];
  m_TRClusterCorrelationsTheta = (TH2F**) new TH2F*[nVXDLayers - 1];
  m_UBResidualsSensor = (TH2F**) new TH2F*[nVXDSensors];
  m_UBResidualsSensorU = (TH1F**) new TH2F*[nVXDSensors];
  m_UBResidualsSensorV = (TH1F**) new TH2F*[nVXDSensors];

  for (VxdID layer : geo.getLayers()) {
    int i = layer.getLayerNumber();
    int index = gTools->getLayerIndex(layer.getLayerNumber());
    /** Track related clusters - hitmap in IP angle range */
    name = str(format("Alig_TRClusterHitmapLayer%1%") % i);
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
    name = str(format("Alig_CorrelationsPhiLayers_%1%_%2%") % i % (i + 1));
    title = str(format("Correlations in Phi for Layers %1% %2%") % i % (i + 1));
    m_TRClusterCorrelationsPhi[index] = new TH2F(name.c_str(), title.c_str(), 360, -180.0, 180.0, 360, -180.0, 180.0);
    title = str(format("angle layer %1% [deg]") % i);
    m_TRClusterCorrelationsPhi[index]->GetXaxis()->SetTitle(title.c_str());
    title = str(format("angle layer %1% [deg]") % (i + 1));
    m_TRClusterCorrelationsPhi[index]->GetYaxis()->SetTitle(title.c_str());
    m_TRClusterCorrelationsPhi[index]->GetZaxis()->SetTitle("counts");
    /** Track related clusters - neighbor corelations in Theta */
    name = str(format("Alig_CorrelationsThetaLayers_%1%_%2%") % i % (i + 1));
    title = str(format("Correlations in Theta for Layers %1% %2%") % i % (i + 1));
    m_TRClusterCorrelationsTheta[index] = new TH2F(name.c_str(), title.c_str(), 180, 0.0, 180.0, 180, 0.0, 180.0);
    title = str(format("angle layer %1% [deg]") % i);
    m_TRClusterCorrelationsTheta[index]->GetXaxis()->SetTitle(title.c_str());
    title = str(format("angle layer %1% [deg]") % (i + 1));
    m_TRClusterCorrelationsTheta[index]->GetYaxis()->SetTitle(title.c_str());
    m_TRClusterCorrelationsTheta[index]->GetZaxis()->SetTitle("counts");
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
  name = str(format("Alig_TrackMomentumX"));
  title = str(format("Track Momentum X"));
  m_MomX = new TH1F(name.c_str(), title.c_str(), 2 * iMomRange, -fMomRange, fMomRange);
  m_MomX->GetXaxis()->SetTitle("Momentum");
  m_MomX->GetYaxis()->SetTitle("counts");
  name = str(format("Alig_TrackMomentumY"));
  title = str(format("Track Momentum Y"));
  m_MomY = new TH1F(name.c_str(), title.c_str(), 2 * iMomRange, -fMomRange, fMomRange);
  m_MomY->GetXaxis()->SetTitle("Momentum");
  m_MomY->GetYaxis()->SetTitle("counts");
  name = str(format("Alig_TrackMomentumZ"));
  title = str(format("Track Momentum Z"));
  m_MomZ = new TH1F(name.c_str(), title.c_str(), 2 * iMomRange, -fMomRange, fMomRange);
  m_MomZ->GetXaxis()->SetTitle("Momentum");
  m_MomZ->GetYaxis()->SetTitle("counts");
  name = str(format("Alig_TrackMomentumPt"));
  title = str(format("Track Momentum pT"));
  m_MomPt = new TH1F(name.c_str(), title.c_str(), 2 * iMomRange, 0.0, fMomRange);
  m_MomPt->GetXaxis()->SetTitle("Momentum");
  m_MomPt->GetYaxis()->SetTitle("counts");
  name = str(format("Alig_TrackMomentumMag"));
  title = str(format("Track Momentum Magnitude"));
  m_Mom = new TH1F(name.c_str(), title.c_str(), 2 * iMomRange, 0.0, fMomRange);
  m_Mom->GetXaxis()->SetTitle("Momentum");
  m_Mom->GetYaxis()->SetTitle("counts");

  name = str(format("Alig_NoOfHitsInTrack_PXD"));
  title = str(format("No Of Hits In Track - PXD"));
  m_HitsPXD = new TH1F(name.c_str(), title.c_str(), iHitsInPXD, 0, iHitsInPXD);
  m_HitsPXD->GetXaxis()->SetTitle("# hits");
  m_HitsPXD->GetYaxis()->SetTitle("counts");
  name = str(format("Alig_NoOfHitsInTrack_SVD"));
  title = str(format("No Of Hits In Track - SVD"));
  m_HitsSVD = new TH1F(name.c_str(), title.c_str(), iHitsInSVD, 0, iHitsInSVD);
  m_HitsSVD->GetXaxis()->SetTitle("# hits");
  m_HitsSVD->GetYaxis()->SetTitle("counts");
  name = str(format("Alig_NoOfHitsInTrack_CDC"));
  title = str(format("No Of Hits In Track - CDC"));
  m_HitsCDC = new TH1F(name.c_str(), title.c_str(), iHitsInCDC, 0, iHitsInCDC);
  m_HitsCDC->GetXaxis()->SetTitle("# hits");
  m_HitsCDC->GetYaxis()->SetTitle("counts");
  name = str(format("Alig_NoOfHitsInTrack"));
  title = str(format("No Of Hits In Track"));
  m_Hits = new TH1F(name.c_str(), title.c_str(), iHits, 0, iHits);
  m_Hits->GetXaxis()->SetTitle("# hits");
  m_Hits->GetYaxis()->SetTitle("counts");

  name = str(format("Alig_NoOfTracksInVXDOnly"));
  title = str(format("No Of Tracks Per Event, Only In VXD"));
  m_TracksVXD = new TH1F(name.c_str(), title.c_str(), iTracks, 0, iTracks);
  m_TracksVXD->GetXaxis()->SetTitle("# tracks");
  m_TracksVXD->GetYaxis()->SetTitle("counts");
  name = str(format("Alig_NoOfTracksInCDCOnly"));
  title = str(format("No Of Tracks Per Event, Only In CDC"));
  m_TracksCDC = new TH1F(name.c_str(), title.c_str(), iTracks, 0, iTracks);
  m_TracksCDC->GetXaxis()->SetTitle("# tracks");
  m_TracksCDC->GetYaxis()->SetTitle("counts");
  name = str(format("Alig_NoOfTracksInVXDCDC"));
  title = str(format("No Of Tracks Per Event, In VXD+CDC"));
  m_TracksVXDCDC = new TH1F(name.c_str(), title.c_str(), iTracks, 0, iTracks);
  m_TracksVXDCDC->GetXaxis()->SetTitle("# tracks");
  m_TracksVXDCDC->GetYaxis()->SetTitle("counts");
  name = str(format("Alig_NoOfTracks"));
  title = str(format("No Of All Tracks Per Event"));
  m_Tracks = new TH1F(name.c_str(), title.c_str(), iTracks, 0, iTracks);
  m_Tracks->GetXaxis()->SetTitle("# tracks");
  m_Tracks->GetYaxis()->SetTitle("counts");

  DirAlignSensResids->cd();
  for (int i = 0; i < nVXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    /** Unbiased residuals for PXD u vs v per sensor*/
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    name = str(format("Alig_UBResiduals_%1%") % sensorDescr);
    sensorDescr = str(format("Layer %1% Ladder %2% Sensor %3%") % iLayer % iLadder % iSensor);
    title = str(format("PXD Unbiased residuals for sensor %1%") % sensorDescr);
    m_UBResidualsSensor[i] = new TH2F(name.c_str(), title.c_str(), 200, -ResidualRange, ResidualRange, 200, -ResidualRange,
                                      ResidualRange);
    m_UBResidualsSensor[i]->GetXaxis()->SetTitle("residual U [#mum]");
    m_UBResidualsSensor[i]->GetYaxis()->SetTitle("residual V [#mum]");
    m_UBResidualsSensor[i]->GetZaxis()->SetTitle("counts");
  }
  DirAlignSensResids1D->cd();
  for (int i = 0; i < nVXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    /** Unbiased residuals for PXD u vs v per sensor*/
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    name = str(format("Alig_UBResidualsU_%1%") % sensorDescr);
    sensorDescr = str(format("Layer %1% Ladder %2% Sensor %3%") % iLayer % iLadder % iSensor);
    title = str(format("PXD Unbiased U residuals for sensor %1%") % sensorDescr);
    m_UBResidualsSensorU[i] = new TH1F(name.c_str(), title.c_str(), 200, -ResidualRange, ResidualRange);
    m_UBResidualsSensorU[i]->GetXaxis()->SetTitle("residual [#mum]");
    m_UBResidualsSensorU[i]->GetYaxis()->SetTitle("counts");
    sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    name = str(format("Alig_UBResidualsV_%1%") % sensorDescr);
    sensorDescr = str(format("Layer %1% Ladder %2% Sensor %3%") % iLayer % iLadder % iSensor);
    title = str(format("PXD Unbiased V residuals for sensor %1%") % sensorDescr);
    m_UBResidualsSensorV[i] = new TH1F(name.c_str(), title.c_str(), 200, -ResidualRange, ResidualRange);
    m_UBResidualsSensorV[i]->GetXaxis()->SetTitle("residual [#mum]");
    m_UBResidualsSensorV[i]->GetYaxis()->SetTitle("counts");
  }

  m_ResMeanPosUVSensCounts = (TH2F**) new TH2F*[nVXDSensors];
  m_ResMeanUPosUVSens = (TH2F**) new TH2F*[nVXDSensors];
  m_ResMeanVPosUVSens = (TH2F**) new TH2F*[nVXDSensors];
  m_ResUPosUSens = (TH2F**) new TH2F*[nVXDSensors];
  m_ResUPosVSens = (TH2F**) new TH2F*[nVXDSensors];
  m_ResVPosUSens = (TH2F**) new TH2F*[nVXDSensors];
  m_ResVPosVSens = (TH2F**) new TH2F*[nVXDSensors];
  m_ResMeanUPosUSens = (TH1F**) new TH1F*[nVXDSensors];
  m_ResMeanUPosVSens = (TH1F**) new TH1F*[nVXDSensors];
  m_ResMeanVPosUSens = (TH1F**) new TH1F*[nVXDSensors];
  m_ResMeanVPosVSens = (TH1F**) new TH1F*[nVXDSensors];

  int iSizeBins = 20;
  float fSizeMin = -50;  // in mm
  float fSizeMax = -fSizeMin;
  DirAlignSensResMeanUPosUV->cd();
  for (int i = 0; i < nVXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    name = str(format("ResMeanUPosUVSens_%1%") % sensorDescr);
    sensorDescr = str(format("Layer %1% Ladder %2% Sensor %3%") % iLayer % iLadder % iSensor);
    title = str(format("Residual Mean U in Position UV, %1%") % sensorDescr);
    m_ResMeanUPosUVSens[i] = new TH2F(name.c_str(), title.c_str(), iSizeBins, fSizeMin, fSizeMax, iSizeBins, fSizeMin, fSizeMax);
    m_ResMeanUPosUVSens[i]->GetXaxis()->SetTitle("position U [mm]");
    m_ResMeanUPosUVSens[i]->GetYaxis()->SetTitle("position V [mm]");
    m_ResMeanUPosUVSens[i]->GetZaxis()->SetTitle("residual U [#mum]");
  }
  for (int i = 0; i < nVXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    name = str(format("ResMeanPosUVCountsSens_%1%") % sensorDescr);
    sensorDescr = str(format("Layer %1% Ladder %2% Sensor %3%") % iLayer % iLadder % iSensor);
    title = str(format("Residual Mean Counts in Position UV, %1%") % sensorDescr);
    m_ResMeanPosUVSensCounts[i] = new TH2F(name.c_str(), title.c_str(), iSizeBins, fSizeMin, fSizeMax, iSizeBins, fSizeMin, fSizeMax);
    m_ResMeanPosUVSensCounts[i]->GetXaxis()->SetTitle("position U [mm]");
    m_ResMeanPosUVSensCounts[i]->GetYaxis()->SetTitle("position V [mm]");
    m_ResMeanPosUVSensCounts[i]->GetZaxis()->SetTitle("counts");
  }

  DirAlignSensResMeanVPosUV->cd();
  for (int i = 0; i < nVXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    name = str(format("ResMeanVPosUVSens_%1%") % sensorDescr);
    sensorDescr = str(format("Layer %1% Ladder %2% Sensor %3%") % iLayer % iLadder % iSensor);
    title = str(format("Residual Mean V in Position UV, %1%") % sensorDescr);
    m_ResMeanVPosUVSens[i] = new TH2F(name.c_str(), title.c_str(), iSizeBins, fSizeMin, fSizeMax, iSizeBins, fSizeMin, fSizeMax);
    m_ResMeanVPosUVSens[i]->GetXaxis()->SetTitle("position U [mm]");
    m_ResMeanVPosUVSens[i]->GetYaxis()->SetTitle("position V [mm]");
    m_ResMeanVPosUVSens[i]->GetZaxis()->SetTitle("residual V [#mum]");
  }

  DirAlignSensResMeanUPosU->cd();
  for (int i = 0; i < nVXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    name = str(format("ResMeanUPosUSens_%1%") % sensorDescr);
    sensorDescr = str(format("Layer %1% Ladder %2% Sensor %3%") % iLayer % iLadder % iSensor);
    title = str(format("Residual Mean U in Position U, %1%") % sensorDescr);
    m_ResMeanUPosUSens[i] = new TH1F(name.c_str(), title.c_str(), iSizeBins, fSizeMin, fSizeMax);
    m_ResMeanUPosUSens[i]->GetXaxis()->SetTitle("position U [mm]");
    m_ResMeanUPosUSens[i]->GetYaxis()->SetTitle("residual mean U [#mum]");
  }

  DirAlignSensResMeanVPosU->cd();
  for (int i = 0; i < nVXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    name = str(format("ResMeanVPosUSens_%1%") % sensorDescr);
    sensorDescr = str(format("Layer %1% Ladder %2% Sensor %3%") % iLayer % iLadder % iSensor);
    title = str(format("Residual Mean V in Position U, %1%") % sensorDescr);
    m_ResMeanVPosUSens[i] = new TH1F(name.c_str(), title.c_str(), iSizeBins, fSizeMin, fSizeMax);
    m_ResMeanVPosUSens[i]->GetXaxis()->SetTitle("position U [mm]");
    m_ResMeanVPosUSens[i]->GetYaxis()->SetTitle("residual mean V [#mum]");
  }

  DirAlignSensResMeanUPosV->cd();
  for (int i = 0; i < nVXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    name = str(format("ResMeanUPosVSens_%1%") % sensorDescr);
    sensorDescr = str(format("Layer %1% Ladder %2% Sensor %3%") % iLayer % iLadder % iSensor);
    title = str(format("Residual Mean U in Position V, %1%") % sensorDescr);
    m_ResMeanUPosVSens[i] = new TH1F(name.c_str(), title.c_str(), iSizeBins, fSizeMin, fSizeMax);
    m_ResMeanUPosVSens[i]->GetXaxis()->SetTitle("position V [mm]");
    m_ResMeanUPosVSens[i]->GetYaxis()->SetTitle("residual mean U [#mum]");
  }

  DirAlignSensResMeanVPosV->cd();
  for (int i = 0; i < nVXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    name = str(format("ResMeanVPosVSens_%1%") % sensorDescr);
    sensorDescr = str(format("Layer %1% Ladder %2% Sensor %3%") % iLayer % iLadder % iSensor);
    title = str(format("Residual Mean V in Position V, %1%") % sensorDescr);
    m_ResMeanVPosVSens[i] = new TH1F(name.c_str(), title.c_str(), iSizeBins, fSizeMin, fSizeMax);
    m_ResMeanVPosVSens[i]->GetXaxis()->SetTitle("position V [mm]");
    m_ResMeanVPosVSens[i]->GetYaxis()->SetTitle("residual mean V [#mum]");
  }

  DirAlignSensResUPosU->cd();
  for (int i = 0; i < nVXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    name = str(format("ResUPosUSensor_%1%") % sensorDescr);
    sensorDescr = str(format("Layer %1% Ladder %2% Sensor %3%") % iLayer % iLadder % iSensor);
    title = str(format("Residual U in Position U, %1%") % sensorDescr);
    m_ResUPosUSens[i] = new TH2F(name.c_str(), title.c_str(), iSizeBins, fSizeMin, fSizeMax, 200, -ResidualRange, ResidualRange);
    m_ResUPosUSens[i]->GetXaxis()->SetTitle("position U [mm]");
    m_ResUPosUSens[i]->GetYaxis()->SetTitle("residual U [#mum]");
    m_ResUPosUSens[i]->GetZaxis()->SetTitle("counts");
  }

  DirAlignSensResVPosU->cd();
  for (int i = 0; i < nVXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    name = str(format("ResVPosUSensor_%1%") % sensorDescr);
    sensorDescr = str(format("Layer %1% Ladder %2% Sensor %3%") % iLayer % iLadder % iSensor);
    title = str(format("Residual V in Position U, %1%") % sensorDescr);
    m_ResVPosUSens[i] = new TH2F(name.c_str(), title.c_str(), iSizeBins, fSizeMin, fSizeMax, 200, -ResidualRange, ResidualRange);
    m_ResVPosUSens[i]->GetXaxis()->SetTitle("position U [mm]");
    m_ResVPosUSens[i]->GetYaxis()->SetTitle("residual V [#mum]");
    m_ResVPosUSens[i]->GetZaxis()->SetTitle("counts");
  }

  DirAlignSensResUPosV->cd();
  for (int i = 0; i < nVXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    name = str(format("ResUPosVSensor_%1%") % sensorDescr);
    sensorDescr = str(format("Layer %1% Ladder %2% Sensor %3%") % iLayer % iLadder % iSensor);
    title = str(format("Residual U in Position V, %1%") % sensorDescr);
    m_ResUPosVSens[i] = new TH2F(name.c_str(), title.c_str(), iSizeBins, fSizeMin, fSizeMax, 200, -ResidualRange, ResidualRange);
    m_ResUPosVSens[i]->GetXaxis()->SetTitle("position V [mm]");
    m_ResUPosVSens[i]->GetYaxis()->SetTitle("residual U [#mum]");
    m_ResUPosVSens[i]->GetZaxis()->SetTitle("counts");
  }

  DirAlignSensResVPosV->cd();
  for (int i = 0; i < nVXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    name = str(format("ResVPosVSensor_%1%") % sensorDescr);
    sensorDescr = str(format("Layer %1% Ladder %2% Sensor %3%") % iLayer % iLadder % iSensor);
    title = str(format("Residual V in Position V, %1%") % sensorDescr);
    m_ResVPosVSens[i] = new TH2F(name.c_str(), title.c_str(), iSizeBins, fSizeMin, fSizeMax, 200, -ResidualRange, ResidualRange);
    m_ResVPosVSens[i]->GetXaxis()->SetTitle("position V [mm]");
    m_ResVPosVSens[i]->GetYaxis()->SetTitle("residual V [#mum]");
    m_ResVPosVSens[i]->GetZaxis()->SetTitle("counts");
  }

  m_ResMeanPhiThetaLayerCounts = (TH2F**) new TH2F*[nVXDLayers];
  m_ResMeanUPhiThetaLayer = (TH2F**) new TH2F*[nVXDLayers];
  m_ResMeanVPhiThetaLayer = (TH2F**) new TH2F*[nVXDLayers];
  m_ResUPhiLayer = (TH2F**) new TH2F*[nVXDLayers];
  m_ResVPhiLayer = (TH2F**) new TH2F*[nVXDLayers];
  m_ResUThetaLayer = (TH2F**) new TH2F*[nVXDLayers];
  m_ResVThetaLayer = (TH2F**) new TH2F*[nVXDLayers];
  m_ResMeanUPhiLayer = (TH1F**) new TH1F*[nVXDLayers];
  m_ResMeanVPhiLayer = (TH1F**) new TH1F*[nVXDLayers];
  m_ResMeanUThetaLayer = (TH1F**) new TH1F*[nVXDLayers];
  m_ResMeanVThetaLayer = (TH1F**) new TH1F*[nVXDLayers];

  int iPhiGran = 90;
  int iThetGran = iPhiGran / 2;
  int iYResGran = 200;

  DirAlignLayerResMeanUPosUV->cd();
  for (VxdID layer : geo.getLayers()) {
    int iLayer = layer.getLayerNumber();
    int iLay = gTools->getLayerIndex(layer.getLayerNumber());
    name = str(format("ResMeanUPhiThetaLayer_%1%") % iLayer);
    title = str(format("Residuals Mean U in Phi Theta, Layer %1%") % iLayer);
    m_ResMeanUPhiThetaLayer[iLay] = new TH2F(name.c_str(), title.c_str(), iPhiGran, -180, 180, iThetGran, 0, 180);
    m_ResMeanUPhiThetaLayer[iLay]->GetXaxis()->SetTitle("Phi [deg]");
    m_ResMeanUPhiThetaLayer[iLay]->GetYaxis()->SetTitle("Theta [deg]");
    m_ResMeanUPhiThetaLayer[iLay]->GetZaxis()->SetTitle("residual [#mum]");
  }
  for (VxdID layer : geo.getLayers()) {
    int iLayer = layer.getLayerNumber();
    int iLay = gTools->getLayerIndex(layer.getLayerNumber());
    name = str(format("ResCounterPhiThetaLayer_%1%") % iLayer);
    title = str(format("Residuals counter in Phi Theta, Layer %1%") % iLayer);
    m_ResMeanPhiThetaLayerCounts[iLay] = new TH2F(name.c_str(), title.c_str(), iPhiGran, -180, 180, iThetGran, 0, 180);
    m_ResMeanPhiThetaLayerCounts[iLay]->GetXaxis()->SetTitle("Phi [deg]");
    m_ResMeanPhiThetaLayerCounts[iLay]->GetYaxis()->SetTitle("Theta [deg]");
    m_ResMeanPhiThetaLayerCounts[iLay]->GetZaxis()->SetTitle("counts");
  }

  DirAlignLayerResMeanVPosUV->cd();
  for (VxdID layer : geo.getLayers()) {
    int iLayer = layer.getLayerNumber();
    int iLay = gTools->getLayerIndex(layer.getLayerNumber());
    name = str(format("ResMeanVPhiThetaLayer_%1%") % iLayer);
    title = str(format("Residuals Mean V in Phi Theta, Layer %1%") % iLayer);
    m_ResMeanVPhiThetaLayer[iLay] = new TH2F(name.c_str(), title.c_str(), iPhiGran, -180, 180, iThetGran, 0, 180);
    m_ResMeanVPhiThetaLayer[iLay]->GetXaxis()->SetTitle("Phi [deg]");
    m_ResMeanVPhiThetaLayer[iLay]->GetYaxis()->SetTitle("Theta [deg]");
    m_ResMeanVPhiThetaLayer[iLay]->GetZaxis()->SetTitle("residual [#mum]");
  }

  DirAlignLayerResMeanUPosU->cd();
  for (VxdID layer : geo.getLayers()) {
    int iLayer = layer.getLayerNumber();
    int iLay = gTools->getLayerIndex(layer.getLayerNumber());
    name = str(format("ResMeanUPhiLayer_%1%") % iLayer);
    title = str(format("Residuals Mean U in Phi, Layer %1%") % iLayer);
    m_ResMeanUPhiLayer[iLay] = new TH1F(name.c_str(), title.c_str(), iPhiGran, -180, 180);
    m_ResMeanUPhiLayer[iLay]->GetXaxis()->SetTitle("Phi [deg]");
    m_ResMeanUPhiLayer[iLay]->GetYaxis()->SetTitle("residual [#mum]");
  }

  DirAlignLayerResMeanVPosU->cd();
  for (VxdID layer : geo.getLayers()) {
    int iLayer = layer.getLayerNumber();
    int iLay = gTools->getLayerIndex(layer.getLayerNumber());
    name = str(format("ResMeanVPhiLayer_%1%") % iLayer);
    title = str(format("Residuals Mean V in Phi, Layer %1%") % iLayer);
    m_ResMeanVPhiLayer[iLay] = new TH1F(name.c_str(), title.c_str(), iPhiGran, -180, 180);
    m_ResMeanVPhiLayer[iLay]->GetXaxis()->SetTitle("Phi [deg]");
    m_ResMeanVPhiLayer[iLay]->GetYaxis()->SetTitle("residual [#mum]");
  }

  DirAlignLayerResMeanUPosV->cd();
  for (VxdID layer : geo.getLayers()) {
    int iLayer = layer.getLayerNumber();
    int iLay = gTools->getLayerIndex(layer.getLayerNumber());
    name = str(format("ResMeanUThetaLayer_%1%") % iLayer);
    title = str(format("Residuals Mean U in Theta, Layer %1%") % iLayer);
    m_ResMeanUThetaLayer[iLay] = new TH1F(name.c_str(), title.c_str(), iThetGran, 0, 180);
    m_ResMeanUThetaLayer[iLay]->GetXaxis()->SetTitle("Theta [deg]");
    m_ResMeanUThetaLayer[iLay]->GetYaxis()->SetTitle("residual [#mum]");
  }

  DirAlignLayerResMeanVPosV->cd();
  for (VxdID layer : geo.getLayers()) {
    int iLayer = layer.getLayerNumber();
    int iLay = gTools->getLayerIndex(layer.getLayerNumber());
    name = str(format("ResMeanVThetaLayer_%1%") % iLayer);
    title = str(format("Residuals Mean V in Theta, Layer %1%") % iLayer);
    m_ResMeanVThetaLayer[iLay] = new TH1F(name.c_str(), title.c_str(), iThetGran, 0, 180);
    m_ResMeanVThetaLayer[iLay]->GetXaxis()->SetTitle("Theta [deg]");
    m_ResMeanVThetaLayer[iLay]->GetYaxis()->SetTitle("residual [#mum]");
  }

  DirAlignLayerResUPosU->cd();
  for (VxdID layer : geo.getLayers()) {
    int iLayer = layer.getLayerNumber();
    int iLay = gTools->getLayerIndex(layer.getLayerNumber());
    name = str(format("ResUPhiLayer_%1%") % iLayer);
    title = str(format("Residuals U in Phi, Layer %1%") % iLayer);
    m_ResUPhiLayer[iLay] = new TH2F(name.c_str(), title.c_str(), iPhiGran, -180, 180, iYResGran, -ResidualRange, ResidualRange);
    m_ResUPhiLayer[iLay]->GetXaxis()->SetTitle("Phi [deg]");
    m_ResUPhiLayer[iLay]->GetYaxis()->SetTitle("residual [#mum]");
    m_ResUPhiLayer[iLay]->GetZaxis()->SetTitle("counts");
  }

  DirAlignLayerResVPosU->cd();
  for (VxdID layer : geo.getLayers()) {
    int iLayer = layer.getLayerNumber();
    int iLay = gTools->getLayerIndex(layer.getLayerNumber());
    name = str(format("ResVPhiLayer_%1%") % iLayer);
    title = str(format("Residuals V in Phi, Layer %1%") % iLayer);
    m_ResVPhiLayer[iLay] = new TH2F(name.c_str(), title.c_str(), iPhiGran, -180, 180, iYResGran, -ResidualRange, ResidualRange);
    m_ResVPhiLayer[iLay]->GetXaxis()->SetTitle("Phi [deg]");
    m_ResVPhiLayer[iLay]->GetYaxis()->SetTitle("residual [#mum]");
    m_ResVPhiLayer[iLay]->GetZaxis()->SetTitle("counts");
  }

  DirAlignLayerResUPosV->cd();
  for (VxdID layer : geo.getLayers()) {
    int iLayer = layer.getLayerNumber();
    int iLay = gTools->getLayerIndex(layer.getLayerNumber());
    name = str(format("ResUThetaLayer_%1%") % iLayer);
    title = str(format("Residuals U in Theta, Layer %1%") % iLayer);
    m_ResUThetaLayer[iLay] = new TH2F(name.c_str(), title.c_str(), iThetGran, 0, 180, iYResGran, -ResidualRange, ResidualRange);
    m_ResUThetaLayer[iLay]->GetXaxis()->SetTitle("Theta [deg]");
    m_ResUThetaLayer[iLay]->GetYaxis()->SetTitle("residual [#mum]");
    m_ResUThetaLayer[iLay]->GetZaxis()->SetTitle("counts");
  }

  DirAlignLayerResVPosV->cd();
  for (VxdID layer : geo.getLayers()) {
    int iLayer = layer.getLayerNumber();
    int iLay = gTools->getLayerIndex(layer.getLayerNumber());
    name = str(format("ResVThetaLayer_%1%") % iLayer);
    title = str(format("Residuals V in Theta, Layer %1%") % iLayer);
    m_ResVThetaLayer[iLay] = new TH2F(name.c_str(), title.c_str(), iThetGran, 0, 180, iYResGran, -ResidualRange, ResidualRange);
    m_ResVThetaLayer[iLay]->GetXaxis()->SetTitle("Theta [deg]");
    m_ResVThetaLayer[iLay]->GetYaxis()->SetTitle("residual [#mum]");
    m_ResVThetaLayer[iLay]->GetZaxis()->SetTitle("counts");
  }

  oldDir->cd();

}

void AlignDQMModule::beginRun()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  if (m_MomPhi != NULL) m_MomPhi->Reset();
  if (m_MomTheta != NULL) m_MomTheta->Reset();
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


void AlignDQMModule::event()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  int iTrack = 0;
  int iTrackVXD = 0;
  int iTrackCDC = 0;
  int iTrackVXDCDC = 0;

  try {

    StoreArray<Track> tracks;
    for (const Track& track : tracks) {  // over tracks
      RelationVector<RecoTrack> theRC = DataStore::getRelationsWithObj<RecoTrack>(&track);
      RelationVector<PXDCluster> pxdClustersTrack = DataStore::getRelationsWithObj<PXDCluster>(theRC[0]);
      int nPXD = (int)pxdClustersTrack.size();
      RelationVector<SVDCluster> svdClustersTrack = DataStore::getRelationsWithObj<SVDCluster>(theRC[0]);
      int nSVD = (int)svdClustersTrack.size();
      RelationVector<CDCHit> cdcHitTrack = DataStore::getRelationsWithObj<CDCHit>(theRC[0]);
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
      TString message = Form("AlignDQM: track %3i, Mom: %f, %f, %f, Pt: %f, Mag: %f, Hits: PXD %i SVD %i CDC %i Suma %i\n",
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

      float Phi = 90;
      if (fabs(tfr->getMomentum().Px()) > 0.00000001) {
        Phi = atan2(tfr->getMomentum().Py(), tfr->getMomentum().Px()) * TMath::RadToDeg();
      }
      float pxy = sqrt(tfr->getMomentum().Px() * tfr->getMomentum().Px() + tfr->getMomentum().Py() * tfr->getMomentum().Py());
      float Theta = 90;
      if (fabs(tfr->getMomentum().Pz()) > 0.00000001) {
        Theta = atan2(pxy, tfr->getMomentum().Pz()) * TMath::RadToDeg();
      }
      m_MomPhi->Fill(Phi);
      m_MomTheta->Fill(Theta);
      m_MomCosTheta->Fill(cos(Theta - 90.0));

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
        float posU = 0;
        float posV = 0;
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
            posU = recoHitInfo->getRelatedTo<PXDCluster>()->getU();
            posV = recoHitInfo->getRelatedTo<PXDCluster>()->getV();
            TVector3 rLocal(posU, posV, 0);
            VxdID sensorID = recoHitInfo->getRelatedTo<PXDCluster>()->getSensorID();
            auto info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
            iLayer = sensorID.getLayerNumber();
            TVector3 ral = info.pointToGlobal(rLocal);
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
            int indexLayer = gTools->getLayerIndex(sensorID.getLayerNumber());
            m_UBResidualsSensor[index]->Fill(ResidUPlaneRHUnBias, ResidVPlaneRHUnBias);
            m_UBResidualsSensorU[index]->Fill(ResidUPlaneRHUnBias);
            m_UBResidualsSensorV[index]->Fill(ResidVPlaneRHUnBias);
            m_TRClusterHitmap[indexLayer]->Fill(fPosSPU, fPosSPV);

            posU *= Unit::convertValueToUnit(1.0, "mm");
            posV *= Unit::convertValueToUnit(1.0, "mm");

            m_ResMeanPosUVSensCounts[index]->Fill(posU, posV);
            m_ResMeanUPosUVSens[index]->Fill(posU, posV, ResidUPlaneRHUnBias);
            m_ResMeanVPosUVSens[index]->Fill(posU, posV, ResidVPlaneRHUnBias);
            m_ResUPosUSens[index]->Fill(posU, ResidUPlaneRHUnBias);
            m_ResUPosVSens[index]->Fill(posV, ResidUPlaneRHUnBias);
            m_ResVPosUSens[index]->Fill(posU, ResidVPlaneRHUnBias);
            m_ResVPosVSens[index]->Fill(posV, ResidVPlaneRHUnBias);

            m_ResMeanPhiThetaLayerCounts[indexLayer]->Fill(fPosSPU, fPosSPV);
            m_ResMeanUPhiThetaLayer[indexLayer]->Fill(fPosSPU, fPosSPV, ResidUPlaneRHUnBias);
            m_ResMeanVPhiThetaLayer[indexLayer]->Fill(fPosSPU, fPosSPV, ResidVPlaneRHUnBias);
            m_ResUPhiLayer[indexLayer]->Fill(fPosSPU, ResidUPlaneRHUnBias);
            m_ResVPhiLayer[indexLayer]->Fill(fPosSPU, ResidVPlaneRHUnBias);
            m_ResUThetaLayer[indexLayer]->Fill(fPosSPV, ResidUPlaneRHUnBias);
            m_ResVThetaLayer[indexLayer]->Fill(fPosSPV, ResidVPlaneRHUnBias);

          }
          if (recoHitInfo->getTrackingDetector() == RecoHitInformation::c_SVD) {
            IsSVDU = recoHitInfo->getRelatedTo<SVDCluster>()->isUCluster();
            VxdID sensorID = recoHitInfo->getRelatedTo<SVDCluster>()->getSensorID();
            auto info = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
            iLayer = sensorID.getLayerNumber();
            if (IsSVDU) {
              posU = recoHitInfo->getRelatedTo<SVDCluster>()->getPosition();
              TVector3 rLocal(posU, 0, 0);
              TVector3 ral = info.pointToGlobal(rLocal);
              fPosSPU = ral.Phi() / TMath::Pi() * 180;
              ResidUPlaneRHUnBias = resUnBias.GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
              if (sensorIDPrew != sensorID) { // other sensor, reset
                ResidVPlaneRHUnBias = 0;
                fPosSPV = 0;
              }
              sensorIDPrew = sensorID;
            } else {
              posV = recoHitInfo->getRelatedTo<SVDCluster>()->getPosition();
              TVector3 rLocal(0, posV, 0);
              TVector3 ral = info.pointToGlobal(rLocal);
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
                int indexLayer = gTools->getLayerIndex(sensorID.getLayerNumber());
                m_UBResidualsSensor[index]->Fill(ResidUPlaneRHUnBias, ResidVPlaneRHUnBias);
                m_UBResidualsSensorU[index]->Fill(ResidUPlaneRHUnBias);
                m_UBResidualsSensorV[index]->Fill(ResidVPlaneRHUnBias);
                m_TRClusterHitmap[indexLayer]->Fill(fPosSPU, fPosSPV);

                posU *= Unit::convertValueToUnit(1.0, "mm");
                posV *= Unit::convertValueToUnit(1.0, "mm");

                m_ResMeanPosUVSensCounts[index]->Fill(posU, posV);
                m_ResMeanUPosUVSens[index]->Fill(posU, posV, ResidUPlaneRHUnBias);
                m_ResMeanVPosUVSens[index]->Fill(posU, posV, ResidVPlaneRHUnBias);
                m_ResUPosUSens[index]->Fill(posU, ResidUPlaneRHUnBias);
                m_ResUPosVSens[index]->Fill(posV, ResidUPlaneRHUnBias);
                m_ResVPosUSens[index]->Fill(posU, ResidVPlaneRHUnBias);
                m_ResVPosVSens[index]->Fill(posV, ResidVPlaneRHUnBias);

                m_ResMeanPhiThetaLayerCounts[indexLayer]->Fill(fPosSPU, fPosSPV);
                m_ResMeanUPhiThetaLayer[indexLayer]->Fill(fPosSPU, fPosSPV, ResidUPlaneRHUnBias);
                m_ResMeanVPhiThetaLayer[indexLayer]->Fill(fPosSPU, fPosSPV, ResidVPlaneRHUnBias);
                m_ResUPhiLayer[indexLayer]->Fill(fPosSPU, ResidUPlaneRHUnBias);
                m_ResVPhiLayer[indexLayer]->Fill(fPosSPU, ResidVPlaneRHUnBias);
                m_ResUThetaLayer[indexLayer]->Fill(fPosSPV, ResidUPlaneRHUnBias);
                m_ResVThetaLayer[indexLayer]->Fill(fPosSPV, ResidVPlaneRHUnBias);

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

  } catch (...) {
    B2DEBUG(70, "Some problem in Alignment DQM module!");
  }
}


void AlignDQMModule::endRun()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  for (VxdID layer : geo.getLayers()) {
    int iLay = gTools->getLayerIndex(layer.getLayerNumber());
    m_ResMeanUPhiThetaLayer[iLay]->Divide(m_ResMeanPhiThetaLayerCounts[iLay]);
    m_ResMeanVPhiThetaLayer[iLay]->Divide(m_ResMeanPhiThetaLayerCounts[iLay]);
    for (int i = 0; i < m_ResUPhiLayer[iLay]->GetNbinsX(); i++) {
      float SumResU = 0;
      float SumResV = 0;
      float CountsU = 0;
      float CountsV = 0;
      for (int j = 0; j < m_ResUPhiLayer[iLay]->GetNbinsY(); j++) {
        SumResU += m_ResUPhiLayer[iLay]->GetBinContent(i + 1, j + 1) * m_ResUPhiLayer[iLay]->GetYaxis()->GetBinCenter(j + 1);
        SumResV += m_ResVPhiLayer[iLay]->GetBinContent(i + 1, j + 1) * m_ResVPhiLayer[iLay]->GetYaxis()->GetBinCenter(j + 1);
        CountsU += m_ResUPhiLayer[iLay]->GetBinContent(i + 1, j + 1);
        CountsV += m_ResVPhiLayer[iLay]->GetBinContent(i + 1, j + 1);
      }
      float valU = 0;
      float valV = 0;
      if (CountsU != 0) {
        valU = SumResU / CountsU;
      }
      if (CountsV != 0) {
        valV = SumResV / CountsV;
      }
      m_ResMeanUPhiLayer[iLay]->SetBinContent(i + 1, valU);
      m_ResMeanVPhiLayer[iLay]->SetBinContent(i + 1, valV);
    }

    for (int i = 0; i < m_ResUThetaLayer[iLay]->GetNbinsX(); i++) {
      float SumResU = 0;
      float SumResV = 0;
      float CountsU = 0;
      float CountsV = 0;
      for (int j = 0; j < m_ResUThetaLayer[iLay]->GetNbinsY(); j++) {
        SumResU += m_ResUThetaLayer[iLay]->GetBinContent(i + 1, j + 1) * m_ResUThetaLayer[iLay]->GetYaxis()->GetBinCenter(j + 1);
        SumResV += m_ResVThetaLayer[iLay]->GetBinContent(i + 1, j + 1) * m_ResVThetaLayer[iLay]->GetYaxis()->GetBinCenter(j + 1);
        CountsU += m_ResUThetaLayer[iLay]->GetBinContent(i + 1, j + 1);
        CountsV += m_ResVThetaLayer[iLay]->GetBinContent(i + 1, j + 1);
      }
      float valU = 0;
      float valV = 0;
      if (CountsU != 0) {
        valU = SumResU / CountsU;
      }
      if (CountsV != 0) {
        valV = SumResV / CountsV;
      }
      m_ResMeanUThetaLayer[iLay]->SetBinContent(i + 1, valU);
      m_ResMeanVThetaLayer[iLay]->SetBinContent(i + 1, valV);
    }
  }

  for (int iSen = 0; iSen < gTools->getNumberOfSensors(); iSen++) {
    m_ResMeanUPosUVSens[iSen]->Divide(m_ResMeanPosUVSensCounts[iSen]);
    m_ResMeanVPosUVSens[iSen]->Divide(m_ResMeanPosUVSensCounts[iSen]);
    for (int i = 0; i < m_ResUPosUSens[iSen]->GetNbinsX(); i++) {
      float SumResU = 0;
      float SumResV = 0;
      float CountsU = 0;
      float CountsV = 0;
      for (int j = 0; j < m_ResUPosUSens[iSen]->GetNbinsY(); j++) {
        SumResU += m_ResUPosUSens[iSen]->GetBinContent(i + 1, j + 1) * m_ResUPosUSens[iSen]->GetYaxis()->GetBinCenter(j + 1);
        SumResV += m_ResVPosUSens[iSen]->GetBinContent(i + 1, j + 1) * m_ResVPosUSens[iSen]->GetYaxis()->GetBinCenter(j + 1);
        CountsU += m_ResUPosUSens[iSen]->GetBinContent(i + 1, j + 1);
        CountsV += m_ResVPosUSens[iSen]->GetBinContent(i + 1, j + 1);
      }
      float valU = 0;
      float valV = 0;
      if (CountsU != 0) {
        valU = SumResU / CountsU;
      }
      if (CountsV != 0) {
        valV = SumResV / CountsV;
      }
      m_ResMeanUPosUSens[iSen]->SetBinContent(i + 1, valU);
      m_ResMeanVPosUSens[iSen]->SetBinContent(i + 1, valV);
    }

    for (int i = 0; i < m_ResUPosVSens[iSen]->GetNbinsX(); i++) {
      float SumResU = 0;
      float SumResV = 0;
      float CountsU = 0;
      float CountsV = 0;
      for (int j = 0; j < m_ResUPosVSens[iSen]->GetNbinsY(); j++) {
        SumResU += m_ResUPosVSens[iSen]->GetBinContent(i + 1, j + 1) * m_ResUPosVSens[iSen]->GetYaxis()->GetBinCenter(j + 1);
        SumResV += m_ResVPosVSens[iSen]->GetBinContent(i + 1, j + 1) * m_ResVPosVSens[iSen]->GetYaxis()->GetBinCenter(j + 1);
        CountsU += m_ResUPosVSens[iSen]->GetBinContent(i + 1, j + 1);
        CountsV += m_ResVPosVSens[iSen]->GetBinContent(i + 1, j + 1);
      }
      float valU = 0;
      float valV = 0;
      if (CountsU != 0) {
        valU = SumResU / CountsU;
      }
      if (CountsV != 0) {
        valV = SumResV / CountsV;
      }
      m_ResMeanUPosVSens[iSen]->SetBinContent(i + 1, valU);
      m_ResMeanVPosVSens[iSen]->SetBinContent(i + 1, valV);
    }
  }

}
