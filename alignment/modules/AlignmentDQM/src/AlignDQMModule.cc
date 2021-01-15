/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys, Jachym Bartik                               *
 *                                                                        *
 * Prepared for Alignment DQM                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/modules/AlignmentDQM/AlignDQMModule.h>
#include <alignment/modules/AlignmentDQM/AlignDQMEventProcessor.h>
#include <tracking/dqmUtils/HistogramFactory.h>

#include <TDirectory.h>

using namespace Belle2;
using namespace Belle2::HistogramFactory;
using namespace std;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(AlignDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

AlignDQMModule::AlignDQMModule() : DQMHistoModuleBase()
{
  setDescription("DQM of Alignment for off line "
                 "residuals per sensor, layer, "
                 "keep also On-Line DQM from tracking: "
                 "their momentum, "
                 "Number of hits in tracks, "
                 "Number of tracks. "
                );
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void AlignDQMModule::defineHisto()
{
  DQMHistoModuleBase::defineHisto();

  if (VXD::GeoCache::getInstance().getGeoTools()->getNumberOfLayers() == 0)
    B2WARNING("Missing geometry for VXD.");

  TDirectory* originalDirectory = gDirectory;
  TDirectory* alignmentDirectory = originalDirectory->mkdir("AlignmentDQM");
  TDirectory* sensorsDirectory = originalDirectory->mkdir("AlignmentDQMSensors");
  TDirectory* layersDirectory = originalDirectory->mkdir("AlignmentDQMLayers");

  alignmentDirectory->cd();
  DefineTracks();
  DefineHits();
  DefineMomentumAngles();
  DefineMomentumCoordinates();
  DefineHelixParametersAndCorrelations();
  DefineTrackFitStatus();
  DefineTRClusters();
  DefineUBResidualsVXD();
  DefineHalfShellsVXD();

  sensorsDirectory->cd();
  DefineSensors();

  layersDirectory->cd();
  DefineLayers();

  originalDirectory->cd();

  for (auto change : m_histogramParameterChanges)
    ProcessHistogramParameterChange(get<0>(change), get<1>(change), get<2>(change));
}

void AlignDQMModule::event()
{
  DQMHistoModuleBase::event();
  if (!histogramsDefined)
    return;

  AlignDQMEventProcessor eventProcessor = AlignDQMEventProcessor(this, m_recoTracksStoreArrayName, m_tracksStoreArrayName);

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

void AlignDQMModule::DefineHelixParametersAndCorrelations()
{
  TDirectory* originalDirectory = gDirectory;

  TDirectory* helixParameters = originalDirectory->mkdir("HelixPars");
  TDirectory* helixCorrelations = originalDirectory->mkdir("HelixCorrelations");

  int iZ0Range = 100;
  double fZ0Range = 10.0;     // Half range in cm
  int iD0Range = 100;
  double fD0Range = 1.0;      // Half range in cm
  int iMomRangeBig = 600;
  int iMomRangeSmall = 60;
  double fMomRange = 6.0;
  int iPhiRange = 180;
  double fPhiRange = 180.0;   // Half range in deg
  int iLambdaRange = 100;
  double fLambdaRange = 4.0;
  int iOmegaRange = 100;
  double fOmegaRange = 0.1;

  auto phi = Axis(iPhiRange, -fPhiRange, fPhiRange, "#phi [deg]");
  auto D0 = Axis(iD0Range, -fD0Range, fD0Range, "d0 [cm]");
  auto Z0 = Axis(iZ0Range, -fZ0Range, fZ0Range, "z0 [cm]");
  auto tanLambda = Axis(iLambdaRange, -fLambdaRange, fLambdaRange, "Tan Lambda");
  auto omega = Axis(iOmegaRange, -fOmegaRange, fOmegaRange, "Omega");
  auto momentumBig = Axis(2 * iMomRangeBig, 0.0, fMomRange, "Momentum");
  auto momentumSmall = Axis(2 * iMomRangeSmall, 0.0, fMomRange, "Momentum");

  auto factory = Factory(this);

  helixParameters->cd();

  factory.yTitleDefault("Arb. Units");

  m_Z0 =        factory.xAxis(Z0).CreateTH1F("Z0", "z0 - the z coordinate of the perigee (beam spot position)");
  m_D0 =        factory.xAxis(D0).CreateTH1F("D0", "d0 - the signed distance to the IP in the r-phi plane");
  m_Phi =       factory.xAxis(phi).CreateTH1F("Phi",
                                              "Phi - angle of the transverse momentum in the r-phi plane, with CDF naming convention");
  m_Omega =     factory.xAxis(omega).CreateTH1F("Omega",
                                                "Omega - the curvature of the track. It's sign is defined by the charge of the particle");
  m_TanLambda = factory.xAxis(tanLambda).CreateTH1F("TanLambda", "TanLambda - the slope of the track in the r-z plane");
  m_MomPt =     factory.xAxis(momentumBig).yTitle("counts").CreateTH1F("TrackMomentumPt", "Track Momentum pT");

  helixCorrelations->cd();

  factory.zTitleDefault("Arb. Units");

  m_PhiD0 =           factory.xAxis(phi).yAxis(D0).CreateTH2F("PhiD0",
                                                              "Phi - angle of the transverse momentum in the r-phi plane vs. d0 - signed distance to the IP in r-phi");
  m_PhiZ0 =           factory.xAxis(phi).yAxis(Z0).CreateTH2F("PhiZ0",
                                                              "Phi - angle of the transverse momentum in the r-phi plane vs. z0 of the perigee (to see primary vertex shifts along R or z)");
  m_PhiMomPt =        factory.xAxis(phi).yAxis(momentumSmall).CreateTH2F("PhiMomPt",
                      "Phi - angle of the transverse momentum in the r-phi plane vs. Track momentum Pt");
  m_PhiOmega =        factory.xAxis(phi).yAxis(omega).CreateTH2F("PhiOmega",
                      "Phi - angle of the transverse momentum in the r-phi plane vs. Omega - the curvature of the track");
  m_PhiTanLambda =    factory.xAxis(phi).yAxis(tanLambda).CreateTH2F("PhiTanLambda",
                      "dPhi - angle of the transverse momentum in the r-phi plane vs. TanLambda - the slope of the track in the r-z plane");
  m_D0Z0 =            factory.xAxis(D0).yAxis(Z0).CreateTH2F("D0Z0",
                                                             "d0 - signed distance to the IP in r-phi vs. z0 of the perigee (to see primary vertex shifts along R or z)");
  m_D0MomPt =         factory.xAxis(D0).yAxis(momentumSmall).CreateTH2F("D0MomPt",
                      "d0 - signed distance to the IP in r-phi vs. Track momentum Pt");
  m_D0Omega =         factory.xAxis(D0).yAxis(omega).CreateTH2F("D0Omega",
                      "d0 - signed distance to the IP in r-phi vs. Omega - the curvature of the track");
  m_D0TanLambda =     factory.xAxis(D0).yAxis(tanLambda).CreateTH2F("D0TanLambda",
                      "d0 - signed distance to the IP in r-phi vs. TanLambda - the slope of the track in the r-z plane");
  m_Z0MomPt =         factory.xAxis(Z0).yAxis(momentumSmall).CreateTH2F("Z0MomPt",
                      "z0 - the z0 coordinate of the perigee vs. Track momentum Pt");
  m_Z0Omega =         factory.xAxis(Z0).yAxis(omega).CreateTH2F("Z0Omega",
                      "z0 - the z0 coordinate of the perigee vs. Omega - the curvature of the track");
  m_Z0TanLambda =     factory.xAxis(Z0).yAxis(tanLambda).CreateTH2F("Z0TanLambda",
                      "z0 - the z0 coordinate of the perigee vs. TanLambda - the slope of the track in the r-z plane");
  m_MomPtOmega =      factory.xAxis(momentumSmall).yAxis(omega).CreateTH2F("MomPtOmega",
                      "Track momentum Pt vs. Omega - the curvature of the track");
  m_MomPtTanLambda =  factory.xAxis(momentumSmall).yAxis(tanLambda).CreateTH2F("MomPtTanLambda",
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

  auto positionU = Axis(iSizeBins, fSizeMin, fSizeMax, "position U [mm]");
  auto positionV = Axis(positionU).title("position V [mm]");
  auto residualU = Axis(200, -residualRange, residualRange, "residual U [#mum]");
  auto residualV = Axis(residualU).title("residual V [#mum]");

  auto factory = Factory(this);

  factory.xAxisDefault(positionU).yAxisDefault(positionV);

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

  factory.xAxisDefault(positionV);

  resMeanUPosV->cd();
  m_ResMeanUPosVSens =        factory.yTitle("residual mean U [#mum]").CreateSensorsTH1F(format("ResMeanUPosVSens_%1%"),
                              format("Residual Mean U in Position V, %1%"));
  resMeanVPosV->cd();
  m_ResMeanVPosVSens =        factory.yTitle("residual mean V [#mum]").CreateSensorsTH1F(format("ResMeanVPosVSens_%1%"),
                              format("Residual Mean V in Position V, %1%"));

  factory.xAxisDefault(positionU).zTitleDefault("counts");

  resUPosU->cd();
  m_ResUPosUSens =            factory.yAxis(residualU).CreateSensorsTH2F(format("ResUPosUSensor_%1%"),
                              format("Residual U in Position U, %1%"));
  resVPosU->cd();
  m_ResVPosUSens =            factory.yAxis(residualV).CreateSensorsTH2F(format("ResVPosUSensor_%1%"),
                              format("Residual V in Position U, %1%"));

  factory.xAxisDefault(positionV);

  resUPosV->cd();
  m_ResUPosVSens =            factory.yAxis(residualU).CreateSensorsTH2F(format("ResUPosVSensor_%1%"),
                              format("Residual U in Position V, %1%"));
  resVPosV->cd();
  m_ResVPosVSens =            factory.yAxis(residualV).CreateSensorsTH2F(format("ResVPosVSensor_%1%"),
                              format("Residual V in Position V, %1%"));

  resids2D->cd();
  m_UBResidualsSensor =       factory.xAxis(residualU).yAxis(residualV).CreateSensorsTH2F(format("UBResiduals_%1%"),
                              format("PXD Unbiased residuals for sensor %1%"));

  factory.yTitleDefault("counts");

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

  auto phi = Axis(iPhiGran, -180, 180, "Phi [deg]");
  auto theta = Axis(iThetGran, 0, 180, "Theta [deg]");
  auto residual = Axis(iYResGran, -residualRange, residualRange, "residual [#mum]");

  auto factory = Factory(this);
  factory.xAxisDefault(phi).yAxisDefault(theta).zTitleDefault("counts");

  resMeanUPosUV->cd();
  m_ResMeanUPhiThetaLayer =       factory.zTitle("residual [#mum]").CreateLayersTH2F(format("ResMeanUPhiThetaLayer_%1%"),
                                  format("Residuals Mean U in Phi Theta, Layer %1%"));
  resMeanVPosUV->cd();
  m_ResMeanVPhiThetaLayer =       factory.zTitle("residual [#mum]").CreateLayersTH2F(format("ResMeanVPhiThetaLayer_%1%"),
                                  format("Residuals Mean V in Phi Theta, Layer %1%"));
  resMeanPosUVCounts->cd();
  m_ResMeanPhiThetaLayerCounts =  factory.CreateLayersTH2F(format("ResCounterPhiThetaLayer_%1%"),
                                                           format("Residuals counter in Phi Theta, Layer %1%"));

  factory.yAxisDefault(residual);

  resUPosU->cd();
  m_ResUPhiLayer =                factory.CreateLayersTH2F(format("ResUPhiLayer_%1%"), format("Residuals U in Phi, Layer %1%"));
  resVPosU->cd();
  m_ResVPhiLayer =                factory.CreateLayersTH2F(format("ResVPhiLayer_%1%"), format("Residuals V in Phi, Layer %1%"));

  factory.xAxisDefault(theta);

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

  factory.xAxisDefault(phi).yTitleDefault("residual [#mum]");

  resMeanUPosU->cd();
  m_ResMeanUPhiLayer =            factory.CreateLayersTH1F(format("ResMeanUPhiLayer_%1%"),
                                                           format("Residuals Mean U in Phi, Layer %1%"));
  resMeanVPosU->cd();
  m_ResMeanVPhiLayer =            factory.CreateLayersTH1F(format("ResMeanVPhiLayer_%1%"),
                                                           format("Residuals Mean V in Phi, Layer %1%"));

  originalDirectory->cd();
}

void AlignDQMModule::FillHelixParametersAndCorrelations(const TrackFitResult* tfr)
{
  DQMHistoModuleBase::FillHelixParametersAndCorrelations(tfr);

  m_PhiZ0->Fill(tfr->getPhi0() / Unit::deg, tfr->getZ0());
  m_PhiMomPt->Fill(tfr->getPhi0() / Unit::deg, tfr->getMomentum().Pt());
  m_PhiOmega->Fill(tfr->getPhi0() / Unit::deg, tfr->getOmega());
  m_PhiTanLambda->Fill(tfr->getPhi0() / Unit::deg, tfr->getTanLambda());
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

void AlignDQMModule::FillPositionSensors(TVector3 residual_um, TVector3 position, int sensorIndex)
{
  float positionU_mm = position.x() / Unit::mm;
  float positionV_mm = position.y() / Unit::mm;

  m_ResMeanPosUVSensCounts[sensorIndex]->Fill(positionU_mm, positionV_mm);
  m_ResMeanUPosUVSens[sensorIndex]->Fill(positionU_mm, positionV_mm, residual_um.x());
  m_ResMeanVPosUVSens[sensorIndex]->Fill(positionU_mm, positionV_mm, residual_um.y());
  m_ResUPosUSens[sensorIndex]->Fill(positionU_mm, residual_um.x());
  m_ResUPosVSens[sensorIndex]->Fill(positionV_mm, residual_um.x());
  m_ResVPosUSens[sensorIndex]->Fill(positionU_mm, residual_um.y());
  m_ResVPosVSens[sensorIndex]->Fill(positionV_mm, residual_um.y());
}

void AlignDQMModule::FillLayers(TVector3 residual_um, float phi_deg, float theta_deg, int layerIndex)
{
  m_ResMeanPhiThetaLayerCounts[layerIndex]->Fill(phi_deg, theta_deg);
  m_ResMeanUPhiThetaLayer[layerIndex]->Fill(phi_deg, theta_deg, residual_um.x());
  m_ResMeanVPhiThetaLayer[layerIndex]->Fill(phi_deg, theta_deg, residual_um.y());
  m_ResUPhiLayer[layerIndex]->Fill(phi_deg, residual_um.x());
  m_ResVPhiLayer[layerIndex]->Fill(phi_deg, residual_um.y());
  m_ResUThetaLayer[layerIndex]->Fill(theta_deg, residual_um.x());
  m_ResVThetaLayer[layerIndex]->Fill(theta_deg, residual_um.y());
}

TH1F* AlignDQMModule::Create(string name, string title, int nbinsx, double xlow, double xup, string xTitle, string yTitle)
{
  return DQMHistoModuleBase::Create("Alig_" + name, title, nbinsx, xlow, xup, xTitle, yTitle);
}

TH2F* AlignDQMModule::Create(string name, string title, int nbinsx, double xlow, double xup,  int nbinsy, double ylow, double yup,
                             string xTitle, string yTitle, string zTitle)
{
  return DQMHistoModuleBase::Create("Alig_" + name, title, nbinsx, xlow, xup,  nbinsy, ylow, yup, xTitle, yTitle, zTitle);
}
