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
#include <alignment/modules/AlignmentDQM/AlignmentEventProcessor.h>
#include <tracking/dqmUtils/THFFactory.h>

#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <vxd/geometry/GeoTools.h>
#include <vxd/geometry/SensorInfoBase.h>

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

AlignDQMModule::AlignDQMModule() : BaseDQMHistogramModule()
{
  //Set module properties
  setDescription("DQM of Alignment for off line "
                 "residuals per sensor, layer, "
                 "keep also On-Line DQM from tracking: "
                 "their momentum, "
                 "Number of hits in tracks, "
                 "Number of tracks. "
                );
  //setPropertyFlags(c_ParallelProcessingCertified);

  //addParam("TracksStoreArrayName", m_TracksStoreArrayName, "StoreArray name where the merged Tracks are written.", m_TracksStoreArrayName);
  //addParam("RecoTracksStoreArrayName", m_RecoTracksStoreArrayName, "StoreArray name where the merged RecoTracks are written.", m_RecoTracksStoreArrayName);
}


AlignDQMModule::~AlignDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void AlignDQMModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_RecoTracksStoreArrayName);
  recoTracks.isOptional();

  StoreArray<Track> tracks(m_TracksStoreArrayName);
  tracks.isOptional();

  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
}

void AlignDQMModule::defineHisto()
{
  if (VXD::GeoCache::getInstance().getGeoTools()->getNumberOfLayers() == 0)
    B2WARNING("Missing geometry for VXD.");

  TDirectory* originalDirectory = gDirectory;
  TDirectory* alignmentDirectory = originalDirectory->mkdir("AlignmentDQM");
  TDirectory* sensorsDirectory = originalDirectory->mkdir("AlignmentDQMSensors");
  TDirectory* layersDirectory = originalDirectory->mkdir("AlignmentDQMLayers");

  alignmentDirectory->cd();
  DefineHelixParameters();
  DefineGeneral();
  DefineUBResiduals();
  DefineHalfShells();
  DefineClusters();
  DefineMomentum();
  DefineHits();
  DefineTracks();

  sensorsDirectory->cd();
  DefineSensors();

  layersDirectory->cd();
  DefineLayers();

  originalDirectory->cd();

  for (auto change : m_HistogramParameterChanges)
    ProcessHistogramParameterChange(get<0>(change), get<1>(change), get<2>(change));
}

void AlignDQMModule::event()
{

  AlignmentEventProcessor eventProcessor = AlignmentEventProcessor(this, m_RecoTracksStoreArrayName, m_TracksStoreArrayName);

  eventProcessor.Run();
}

void AlignDQMModule::endRun()
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  auto gTools = geo.getGeoTools();

  for (VxdID layer : geo.getLayers()) {
    int layerIndex = gTools->getLayerIndex(layer.getLayerNumber());
    m_ResMeanUPhiThetaLayer[layerIndex]->Divide(m_ResMeanPhiThetaLayerCounts[layerIndex]);
    m_ResMeanVPhiThetaLayer[layerIndex]->Divide(m_ResMeanPhiThetaLayerCounts[layerIndex]);

    ComputeMean(m_ResMeanUPhiLayer[layerIndex], m_ResUPhiLayer[layerIndex]);
    ComputeMean(m_ResMeanVPhiLayer[layerIndex], m_ResVPhiLayer[layerIndex]);

    ComputeMean(m_ResMeanUThetaLayer[layerIndex], m_ResUThetaLayer[layerIndex]);
    ComputeMean(m_ResMeanVThetaLayer[layerIndex], m_ResVThetaLayer[layerIndex]);
  }

  for (int sensorIndex = 0; sensorIndex < gTools->getNumberOfSensors(); sensorIndex++) {
    m_ResMeanUPosUVSens[sensorIndex]->Divide(m_ResMeanPosUVSensCounts[sensorIndex]);
    m_ResMeanVPosUVSens[sensorIndex]->Divide(m_ResMeanPosUVSensCounts[sensorIndex]);

    ComputeMean(m_ResMeanUPosUSens[sensorIndex], m_ResUPosUSens[sensorIndex]);
    ComputeMean(m_ResMeanVPosUSens[sensorIndex], m_ResVPosUSens[sensorIndex]);

    ComputeMean(m_ResMeanUPosVSens[sensorIndex], m_ResUPosVSens[sensorIndex]);
    ComputeMean(m_ResMeanVPosVSens[sensorIndex], m_ResVPosVSens[sensorIndex]);
  }
}

void AlignDQMModule::DefineHelixParameters()
{
  TDirectory* originalDirectory = gDirectory;

  TDirectory* helixParameters = originalDirectory->mkdir("HelixPars");
  TDirectory* helixCorrelations = originalDirectory->mkdir("HelixCorrelations");

  double fZ0Range = 10.0;     // Half range in cm
  double fD0Range = 1.0;      // Half range in cm
  int iMomRange = 60;
  double fMomRange = 3.0;
  int iPhiRange = 180;
  double fPhiRange = 180.0;   // Half range in deg
  double lambdaRange = 4.0;
  double omegaRange = 0.1;

  auto phi = THFAxis(iPhiRange, -fPhiRange, fPhiRange, "#phi [deg]");
  auto D0 = THFAxis(100, -fD0Range, fD0Range, "d0 [cm]");
  auto Z0 = THFAxis(100, -fZ0Range, fZ0Range, "z0 [cm]");
  auto tanLambda = THFAxis(100, -lambdaRange, lambdaRange, "Tan Lambda");
  auto omega = THFAxis(100, -omegaRange, omegaRange, "Omega");
  auto momentum = THFAxis(2 * iMomRange, 0.0, fMomRange, "Momentum");

  auto factory = THFFactory(this);

  helixParameters->cd();

  factory.yTitleSet("Arb. Units");

  m_Z0 =        factory.xAxis(Z0).CreateTH1F("Z0", "z0 - the z coordinate of the perigee (beam spot position)");
  m_D0 =        factory.xAxis(D0).CreateTH1F("D0", "d0 - the signed distance to the IP in the r-phi plane");
  m_Phi =       factory.xAxis(phi).CreateTH1F("Phi",
                                              "Phi - angle of the transverse momentum in the r-phi plane, with CDF naming convention");
  m_Omega =     factory.xAxis(omega).CreateTH1F("Omega",
                                                "Omega - the curvature of the track. It's sign is defined by the charge of the particle");
  m_TanLambda = factory.xAxis(tanLambda).CreateTH1F("TanLambda", "TanLambda - the slope of the track in the r-z plane");
  m_MomPt =     factory.xAxis(momentum).yTitle("counts").CreateTH1F("TrackMomentumPt", "Track Momentum pT");

  helixCorrelations->cd();

  factory.zTitleSet("Arb. Units");

  m_PhiD0 =           factory.xAxis(phi).yAxis(D0).CreateTH2F("PhiD0",
                                                              "Phi - angle of the transverse momentum in the r-phi plane vs. d0 - signed distance to the IP in r-phi");
  m_PhiZ0 =           factory.xAxis(phi).yAxis(Z0).CreateTH2F("PhiZ0",
                                                              "Phi - angle of the transverse momentum in the r-phi plane vs. z0 of the perigee (to see primary vertex shifts along R or z)");
  m_PhiMomPt =        factory.xAxis(phi).yAxis(momentum).CreateTH2F("PhiMomPt",
                      "Phi - angle of the transverse momentum in the r-phi plane vs. Track momentum Pt");
  m_PhiOmega =        factory.xAxis(phi).yAxis(omega).CreateTH2F("PhiOmega",
                      "Phi - angle of the transverse momentum in the r-phi plane vs. Omega - the curvature of the track");
  m_PhiTanLambda =    factory.xAxis(phi).yAxis(tanLambda).CreateTH2F("PhiTanLambda",
                      "dPhi - angle of the transverse momentum in the r-phi plane vs. TanLambda - the slope of the track in the r-z plane");
  m_D0Z0 =            factory.xAxis(D0).yAxis(Z0).CreateTH2F("D0Z0",
                                                             "d0 - signed distance to the IP in r-phi vs. z0 of the perigee (to see primary vertex shifts along R or z)");
  m_D0MomPt =         factory.xAxis(D0).yAxis(momentum).CreateTH2F("D0MomPt",
                      "d0 - signed distance to the IP in r-phi vs. Track momentum Pt");
  m_D0Omega =         factory.xAxis(D0).yAxis(omega).CreateTH2F("D0Omega",
                      "d0 - signed distance to the IP in r-phi vs. Omega - the curvature of the track");
  m_D0TanLambda =     factory.xAxis(D0).yAxis(tanLambda).CreateTH2F("D0TanLambda",
                      "d0 - signed distance to the IP in r-phi vs. TanLambda - the slope of the track in the r-z plane");
  m_Z0MomPt =         factory.xAxis(Z0).yAxis(momentum).CreateTH2F("Z0MomPt",
                      "z0 - the z0 coordinate of the perigee vs. Track momentum Pt");
  m_Z0Omega =         factory.xAxis(Z0).yAxis(omega).CreateTH2F("Z0Omega",
                      "z0 - the z0 coordinate of the perigee vs. Omega - the curvature of the track");
  m_Z0TanLambda =     factory.xAxis(Z0).yAxis(tanLambda).CreateTH2F("Z0TanLambda",
                      "z0 - the z0 coordinate of the perigee vs. TanLambda - the slope of the track in the r-z plane");
  m_MomPtOmega =      factory.xAxis(momentum).yAxis(omega).CreateTH2F("MomPtOmega",
                      "Track momentum Pt vs. Omega - the curvature of the track");
  m_MomPtTanLambda =  factory.xAxis(momentum).yAxis(tanLambda).CreateTH2F("MomPtTanLambda",
                      "Track momentum Pt vs. TanLambda - the slope of the track in the r-z plane");
  m_OmegaTanLambda =  factory.xAxis(omega).yAxis(tanLambda).CreateTH2F("OmegaTanLambda",
                      "Omega - the curvature of the track vs. TanLambda - the slope of the track in the r-z plane");

  originalDirectory->cd();
}

void AlignDQMModule::DefineSensors()
{
  TDirectory* originalDirectory = gDirectory;

  TDirectory* resMeanUPosUV = originalDirectory->mkdir("ResidMeanUPositUV");
  TDirectory* resMeanVPosUV = originalDirectory->mkdir("ResidMeanVPositUV");
  TDirectory* resMeanPosUVCounts = originalDirectory->mkdir("ResidMeanPositUVCounts");
  TDirectory* resMeanUPosU = originalDirectory->mkdir("ResidMeanUPositU");
  TDirectory* resMeanVPosU = originalDirectory->mkdir("ResidMeanVPositU");
  TDirectory* resMeanUPosV = originalDirectory->mkdir("ResidMeanUPositV");
  TDirectory* resMeanVPosV = originalDirectory->mkdir("ResidMeanVPositV");
  TDirectory* resUPosU = originalDirectory->mkdir("ResidUPositU");
  TDirectory* resVPosU = originalDirectory->mkdir("ResidVPositU");
  TDirectory* resUPosV = originalDirectory->mkdir("ResidUPositV");
  TDirectory* resVPosV = originalDirectory->mkdir("ResidVPositV");
  TDirectory* resids2D = originalDirectory->mkdir("Residuals2D");
  TDirectory* resids1D = originalDirectory->mkdir("Residuals1D");

  int iSizeBins = 20;
  double fSizeMin = -50;  // in mm
  double fSizeMax = -fSizeMin;
  double residualRange = 400;  // in um

  auto posU = THFAxis(iSizeBins, fSizeMin, fSizeMax, "position U [mm]");
  auto posV = THFAxis(posU).title("position V [mm]");
  auto residualU = THFAxis(200, -residualRange, residualRange, "residual U [#mum]");
  auto residualV = THFAxis(residualU).title("residual V [#mum]");

  THFFactory factory = THFFactory(this);

  factory.xAxisSet(posU).yAxisSet(posV);

  resMeanUPosUV->cd();
  m_ResMeanUPosUVSens =       factory.zTitle("residual U [#mum]").CreateSensorsTH2F(format("ResMeanUPosUVSens_%1%"),
                              format("Residual Mean U in Position UV, %1%"));
  resMeanVPosUV->cd();
  m_ResMeanVPosUVSens =       factory.zTitle("residual V [#mum]").CreateSensorsTH2F(format("ResMeanVPosUVSens_%1%"),
                              format("Residual Mean V in Position UV, %1%"));
  resMeanPosUVCounts->cd();
  m_ResMeanPosUVSensCounts =  factory.zTitle("counts").CreateSensorsTH2F(format("ResMeanPosUVCountsSens_%1%"),
                              format("Residual Mean Counts in Position UV, %1%"));
  resMeanUPosU->cd();
  m_ResMeanUPosUSens =        factory.yTitle("residual mean U [#mum]").CreateSensorsTH1F(format("ResMeanUPosUSens_%1%"),
                              format("Residual Mean U in Position U, %1%"));
  resMeanVPosU->cd();
  m_ResMeanVPosUSens =        factory.yTitle("residual mean V [#mum]").CreateSensorsTH1F(format("ResMeanVPosUSens_%1%"),
                              format("Residual Mean V in Position U, %1%"));

  factory.xAxisSet(posV);

  resMeanUPosV->cd();
  m_ResMeanUPosVSens =        factory.yTitle("residual mean U [#mum]").CreateSensorsTH1F(format("ResMeanUPosVSens_%1%"),
                              format("Residual Mean U in Position V, %1%"));
  resMeanVPosV->cd();
  m_ResMeanVPosVSens =        factory.yTitle("residual mean V [#mum]").CreateSensorsTH1F(format("ResMeanVPosVSens_%1%"),
                              format("Residual Mean V in Position V, %1%"));

  factory.xAxisSet(posU).zTitleSet("counts");

  resUPosU->cd();
  m_ResUPosUSens =            factory.yAxis(residualU).CreateSensorsTH2F(format("ResUPosUSensor_%1%"),
                              format("Residual U in Position U, %1%"));
  resVPosU->cd();
  m_ResVPosUSens =            factory.yAxis(residualV).CreateSensorsTH2F(format("ResVPosUSensor_%1%"),
                              format("Residual V in Position U, %1%"));

  factory.xAxisSet(posV);

  resUPosV->cd();
  m_ResUPosVSens =            factory.yAxis(residualU).CreateSensorsTH2F(format("ResUPosVSensor_%1%"),
                              format("Residual U in Position V, %1%"));
  resVPosV->cd();
  m_ResVPosVSens =            factory.yAxis(residualV).CreateSensorsTH2F(format("ResVPosVSensor_%1%"),
                              format("Residual V in Position V, %1%"));

  resids2D->cd();
  m_UBResidualsSensor =       factory.xAxis(residualU).yAxis(residualV).CreateSensorsTH2F(format("UBResiduals_%1%"),
                              format("PXD Unbiased residuals for sensor %1%"));

  factory.yTitleSet("counts");

  resids1D->cd();
  m_UBResidualsSensorU =      factory.xAxis(residualU).CreateSensorsTH1F(format("UBResidualsU_%1%"),
                              format("PXD Unbiased U residuals for sensor %1%"));
  m_UBResidualsSensorV =      factory.xAxis(residualV).CreateSensorsTH1F(format("UBResidualsV_%1%"),
                              format("PXD Unbiased V residuals for sensor %1%"));

  originalDirectory->cd();
}

void AlignDQMModule::DefineLayers()
{
  TDirectory* originalDirectory = gDirectory;

  TDirectory* resMeanUPosUV = originalDirectory->mkdir("ResidLayerMeanUPositPhiTheta");
  TDirectory* resMeanVPosUV = originalDirectory->mkdir("ResidLayerMeanVPositPhiTheta");
  TDirectory* resMeanPosUVCounts = originalDirectory->mkdir("ResidLayerMeanPositPhiThetaCounts");
  TDirectory* resMeanUPosU = originalDirectory->mkdir("ResidLayerMeanUPositPhi");
  TDirectory* resMeanVPosU = originalDirectory->mkdir("ResidLayerMeanVPositPhi");
  TDirectory* resMeanUPosV = originalDirectory->mkdir("ResidLayerMeanUPositTheta");
  TDirectory* resMeanVPosV = originalDirectory->mkdir("ResidLayerMeanVPositTheta");
  TDirectory* resUPosU = originalDirectory->mkdir("ResidLayerUPositPhi");
  TDirectory* resVPosU = originalDirectory->mkdir("ResidLayerVPositPhi");
  TDirectory* resUPosV = originalDirectory->mkdir("ResidLayerUPositTheta");
  TDirectory* resVPosV = originalDirectory->mkdir("ResidLayerVPositTheta");

  int iPhiGran = 90;
  int iThetGran = iPhiGran / 2;
  int iYResGran = 200;
  double residualRange = 400;  // in um

  auto phi = THFAxis(iPhiGran, -180, 180, "Phi [deg]");
  auto theta = THFAxis(iThetGran, 0, 180, "Theta [deg]");
  auto residual = THFAxis(iYResGran, -residualRange, residualRange, "residual [#mum]");

  THFFactory factory = THFFactory(this);
  factory.xAxisSet(phi).yAxisSet(theta).zTitleSet("counts");

  resMeanUPosUV->cd();
  m_ResMeanUPhiThetaLayer =       factory.zTitle("residual [#mum]").CreateLayersTH2F(format("ResMeanUPhiThetaLayer_%1%"),
                                  format("Residuals Mean U in Phi Theta, Layer %1%"));
  resMeanVPosUV->cd();
  m_ResMeanVPhiThetaLayer =       factory.zTitle("residual [#mum]").CreateLayersTH2F(format("ResMeanVPhiThetaLayer_%1%"),
                                  format("Residuals Mean V in Phi Theta, Layer %1%"));
  resMeanPosUVCounts->cd();
  m_ResMeanPhiThetaLayerCounts =  factory.CreateLayersTH2F(format("ResCounterPhiThetaLayer_%1%"),
                                                           format("Residuals counter in Phi Theta, Layer %1%"));

  factory.yAxisSet(residual);

  resUPosU->cd();
  m_ResUPhiLayer =                factory.CreateLayersTH2F(format("ResUPhiLayer_%1%"), format("Residuals U in Phi, Layer %1%"));
  resVPosU->cd();
  m_ResVPhiLayer =                factory.CreateLayersTH2F(format("ResVPhiLayer_%1%"), format("Residuals V in Phi, Layer %1%"));

  factory.xAxisSet(theta);

  resUPosV->cd();
  m_ResUThetaLayer =              factory.CreateLayersTH2F(format("ResUThetaLayer_%1%"), format("Residuals U in Theta, Layer %1%"));
  resVPosV->cd();
  m_ResVThetaLayer =              factory.CreateLayersTH2F(format("ResVThetaLayer_%1%"), format("Residuals V in Theta, Layer %1%"));
  resMeanUPosV->cd();
  m_ResMeanUThetaLayer =          factory.CreateLayersTH1F(format("ResMeanUThetaLayer_%1%"),
                                                           format("Residuals Mean U in Theta, Layer %1%"));
  resMeanVPosV->cd();
  m_ResMeanVThetaLayer =          factory.CreateLayersTH1F(format("ResMeanVThetaLayer_%1%"),
                                                           format("Residuals Mean V in Theta, Layer %1%"));

  factory.xAxisSet(phi).yTitleSet("residual [#mum]");

  resMeanUPosU->cd();
  m_ResMeanUPhiLayer =            factory.CreateLayersTH1F(format("ResMeanUPhiLayer_%1%"),
                                                           format("Residuals Mean U in Phi, Layer %1%"));
  resMeanVPosU->cd();
  m_ResMeanVPhiLayer =            factory.CreateLayersTH1F(format("ResMeanVPhiLayer_%1%"),
                                                           format("Residuals Mean V in Phi, Layer %1%"));

  originalDirectory->cd();
}

void AlignDQMModule::FillTrackFitResult(const TrackFitResult* tfr)
{
  BaseDQMHistogramModule::FillTrackFitResult(tfr);

  m_PhiZ0->Fill(tfr->getPhi0() * Unit::convertValueToUnit(1.0, "deg"), tfr->getZ0());
  m_PhiMomPt->Fill(tfr->getPhi0() * Unit::convertValueToUnit(1.0, "deg"), tfr->getMomentum().Pt());
  m_PhiOmega->Fill(tfr->getPhi0() * Unit::convertValueToUnit(1.0, "deg"), tfr->getOmega());
  m_PhiTanLambda->Fill(tfr->getPhi0() * Unit::convertValueToUnit(1.0, "deg"), tfr->getTanLambda());
  m_D0MomPt->Fill(tfr->getD0(), tfr->getMomentum().Pt());
  m_D0Omega->Fill(tfr->getD0(), tfr->getOmega());
  m_D0TanLambda->Fill(tfr->getD0(), tfr->getTanLambda());
  m_Z0MomPt->Fill(tfr->getZ0(), tfr->getMomentum().Pt());
  m_Z0Omega->Fill(tfr->getZ0(), tfr->getOmega());
  m_Z0TanLambda->Fill(tfr->getZ0(), tfr->getTanLambda());
  m_MomPtOmega->Fill(tfr->getMomentum().Pt(), tfr->getOmega());
  m_MomPtTanLambda->Fill(tfr->getMomentum().Pt(), tfr->getTanLambda());
  m_OmegaTanLambda->Fill(tfr->getOmega(), tfr->getTanLambda());
}

void AlignDQMModule::FillSensorIndex(float ResidUPlaneRHUnBias, float ResidVPlaneRHUnBias, float posU, float posV, int sensorIndex)
{
  float posU_mm = posU * Unit::convertValueToUnit(1.0, "mm");
  float posV_mm = posV * Unit::convertValueToUnit(1.0, "mm");

  m_ResMeanPosUVSensCounts[sensorIndex]->Fill(posU_mm, posV_mm);
  m_ResMeanUPosUVSens[sensorIndex]->Fill(posU_mm, posV_mm, ResidUPlaneRHUnBias);
  m_ResMeanVPosUVSens[sensorIndex]->Fill(posU_mm, posV_mm, ResidVPlaneRHUnBias);
  m_ResUPosUSens[sensorIndex]->Fill(posU_mm, ResidUPlaneRHUnBias);
  m_ResUPosVSens[sensorIndex]->Fill(posV_mm, ResidUPlaneRHUnBias);
  m_ResVPosUSens[sensorIndex]->Fill(posU_mm, ResidVPlaneRHUnBias);
  m_ResVPosVSens[sensorIndex]->Fill(posV_mm, ResidVPlaneRHUnBias);
}

void AlignDQMModule::FillLayers(float ResidUPlaneRHUnBias, float ResidVPlaneRHUnBias, float fPosSPU, float fPosSPV, int layerIndex)
{
  m_ResMeanPhiThetaLayerCounts[layerIndex]->Fill(fPosSPU, fPosSPV);
  m_ResMeanUPhiThetaLayer[layerIndex]->Fill(fPosSPU, fPosSPV, ResidUPlaneRHUnBias);
  m_ResMeanVPhiThetaLayer[layerIndex]->Fill(fPosSPU, fPosSPV, ResidVPlaneRHUnBias);
  m_ResUPhiLayer[layerIndex]->Fill(fPosSPU, ResidUPlaneRHUnBias);
  m_ResVPhiLayer[layerIndex]->Fill(fPosSPU, ResidVPlaneRHUnBias);
  m_ResUThetaLayer[layerIndex]->Fill(fPosSPV, ResidUPlaneRHUnBias);
  m_ResVThetaLayer[layerIndex]->Fill(fPosSPV, ResidVPlaneRHUnBias);
}