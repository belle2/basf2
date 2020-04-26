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

#include <tracking/dqmUtils/DQMHistoModuleBase.h>
#include <tracking/dqmUtils/THFFactory.h>

#include <framework/datastore/StoreArray.h>
#include <vxd/geometry/GeoTools.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <TDirectory.h>

using namespace Belle2;
using namespace std;
using boost::format;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistoModuleBase::DQMHistoModuleBase() : HistoModule()
{
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("tracksStoreArrayName", m_tracksStoreArrayName, "StoreArray name where the merged Tracks are written.",
           m_tracksStoreArrayName);
  addParam("recoTracksStoreArrayName", m_recoTracksStoreArrayName, "StoreArray name where the merged RecoTracks are written.",
           m_recoTracksStoreArrayName);
  addParam("histogramParameterChanges", m_histogramParameterChanges, "Changes of default parameters of histograms.",
           m_histogramParameterChanges);
}

DQMHistoModuleBase::~DQMHistoModuleBase()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void DQMHistoModuleBase::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_recoTracksStoreArrayName);
  if (!recoTracks.isOptional()) {
    B2WARNING("Missing recoTracks array, " + getName() + " is skipped.");
    return;
  }

  StoreArray<Track> Tracks(m_tracksStoreArrayName);
  if (!Tracks.isOptional()) {
    B2WARNING("Missing Tracks array, " + getName() + " is skipped.");
    return;
  }

  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
}

void DQMHistoModuleBase::defineHisto()
{
  histogramsDefined = true;
}

void DQMHistoModuleBase::beginRun()
{
  StoreArray<RecoTrack> recoTracks(m_recoTracksStoreArrayName);
  if (!recoTracks.isOptional()) {
    B2DEBUG(22, "Missing recoTracks array in beginRun() for " + getName());
    return;
  }
  StoreArray<Track> tracks(m_tracksStoreArrayName);
  if (!tracks.isOptional()) {
    B2DEBUG(22, "Missing recoTracks array in beginRun() for " + getName());
    return;
  }

  for (TH1* histogram : m_histograms)
    histogram->Reset();
}


void DQMHistoModuleBase::event()
{
  if (!histogramsDefined) {
    B2ERROR("Histograms not defined in " + this->getName() + " module, event processing is skipped!");
    return;
  }
}

TH1F* DQMHistoModuleBase::Create(const char* name, const char* title, int nbinsx, double xlow, double xup, const char* xTitle,
                                 const char* yTitle)
{
  TH1F* histogram = new TH1F(name, title, nbinsx, xlow, xup);
  histogram->GetXaxis()->SetTitle(xTitle);
  histogram->GetYaxis()->SetTitle(yTitle);

  m_histograms.push_back(histogram);

  return histogram;
}

TH2F* DQMHistoModuleBase::Create(const char* name, const char* title, int nbinsx, double xlow, double xup, int nbinsy,
                                 double ylow, double yup, const char* xTitle, const char* yTitle, const char* zTitle)
{
  TH2F* histogram = new TH2F(name, title, nbinsx, xlow, xup, nbinsy, ylow, yup);
  histogram->GetXaxis()->SetTitle(xTitle);
  histogram->GetYaxis()->SetTitle(yTitle);
  histogram->GetZaxis()->SetTitle(zTitle);

  m_histograms.push_back(histogram);

  return histogram;
}

string DQMHistoModuleBase::SensorNameDescription(VxdID sensorID)
{
  return str(format("%1%_%2%_%3%") % sensorID.getLayerNumber() % sensorID.getLadderNumber() % sensorID.getSensorNumber());
}

string DQMHistoModuleBase::SensorTitleDescription(VxdID sensorID)
{
  return str(format("Layer %1% Ladder %2% Sensor %3%") % sensorID.getLayerNumber() % sensorID.getLadderNumber() %
             sensorID.getSensorNumber());
}

TH1F** DQMHistoModuleBase::CreateLayers(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow,
                                        double xup, const char* xTitle, const char* yTitle)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  auto gTools = geo.getGeoTools();

  TH1F** output = new TH1F*[gTools->getNumberOfLayers()];

  for (VxdID layer : geo.getLayers()) {
    int layerNumber = layer.getLayerNumber();
    int layerIndex = gTools->getLayerIndex(layerNumber);
    string name = str(nameTemplate % layerNumber);
    string title = str(titleTemplate % layerNumber);
    output[layerIndex] = Create(name.c_str(), title.c_str(), nbinsx, xlow, xup, xTitle, yTitle);
  }

  return output;
}

TH2F** DQMHistoModuleBase::CreateLayers(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow,
                                        double xup, int nbinsy, double ylow, double yup, const char* xTitle, const char* yTitle, const char* zTitle)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  auto gTools = geo.getGeoTools();

  TH2F** output = new TH2F*[gTools->getNumberOfLayers()];

  for (VxdID layer : geo.getLayers()) {
    int layerNumber = layer.getLayerNumber();
    int layerIndex = gTools->getLayerIndex(layerNumber);
    string name = str(nameTemplate % layerNumber);
    string title = str(titleTemplate % layerNumber);
    output[layerIndex] = Create(name.c_str(), title.c_str(), nbinsx, xlow, xup, nbinsy, ylow, yup, xTitle, yTitle, zTitle);
  }

  return output;
}

TH1F** DQMHistoModuleBase::CreateSensors(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow,
                                         double xup, const char* xTitle, const char* yTitle)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  auto gTools = geo.getGeoTools();
  int nVXDSensors = gTools->getNumberOfSensors();

  TH1F** output = new TH1F*[nVXDSensors];

  for (int sensorIndex = 0; sensorIndex < nVXDSensors; sensorIndex++) {
    VxdID sensorID = gTools->getSensorIDFromIndex(sensorIndex);
    string name = str(nameTemplate % SensorNameDescription(sensorID));
    string title = str(titleTemplate % SensorTitleDescription(sensorID));
    output[sensorIndex] = Create(name.c_str(), title.c_str(), nbinsx, xlow, xup, xTitle, yTitle);
  }

  return output;
}

TH2F** DQMHistoModuleBase::CreateSensors(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow,
                                         double xup, int nbinsy, double ylow, double yup, const char* xTitle, const char* yTitle, const char* zTitle)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  auto gTools = geo.getGeoTools();
  int nVXDSensors = gTools->getNumberOfSensors();

  TH2F** output = new TH2F*[nVXDSensors];

  for (int sensorIndex = 0; sensorIndex < nVXDSensors; sensorIndex++) {
    VxdID sensorID = gTools->getSensorIDFromIndex(sensorIndex);
    string name = str(nameTemplate % SensorNameDescription(sensorID));
    string title = str(titleTemplate % SensorTitleDescription(sensorID));
    output[sensorIndex] = Create(name.c_str(), title.c_str(), nbinsx, xlow, xup, nbinsy, ylow, yup, xTitle, yTitle, zTitle);
  }

  return output;
}

void DQMHistoModuleBase::DefineGeneral()
{
  m_MomPhi = Create("MomPhi", "Momentum Phi of fit", 180, TMath::Pi(), TMath::Pi(), "Mom Phi", "counts");
  m_MomTheta = Create("MomTheta", "Momentum Theta of fit", 90, 0, TMath::Pi(), "Mom Theta", "counts");
  m_MomCosTheta = Create("MomCosTheta", "Cos of Momentum Theta of fit", 100, -1, 1, "Mom CosTheta", "counts");
  m_PValue = Create("PValue", "P value of fit", 100, 0, 1, "p value", "counts");
  m_Chi2 = Create("Chi2", "Chi2 of fit", 200, 0, 150, "Chi2", "counts");
  m_NDF = Create("NDF", "NDF of fit", 200, 0, 200, "NDF", "counts");
  m_Chi2NDF = Create("Chi2NDF", "Chi2 div NDF of fit", 200, 0, 10, "Chi2NDF", "counts");
}

void DQMHistoModuleBase::DefineUBResiduals()
{
  double residualRange = 400;  // in um

  auto residualU = THFAxis(200, -residualRange, residualRange, "residual U [#mum]");
  auto residualV = THFAxis(residualU).title("residual V [#mum]");

  auto factory = THFFactory(this);
  factory.xAxisSet(residualU).yAxisSet(residualV).zTitleSet("counts");

  m_UBResidualsPXD = factory.CreateTH2F("UBResidualsPXD", "Unbiased residuals for PXD");
  m_UBResidualsSVD = factory.CreateTH2F("UBResidualsSVD", "Unbiased residuals for SVD");

  factory.xAxisSet(residualU).yTitleSet("counts");

  m_UBResidualsPXDU = factory.CreateTH1F("UBResidualsPXDU", "Unbiased residuals in U for PXD");
  m_UBResidualsSVDU = factory.CreateTH1F("UBResidualsSVDU", "Unbiased residuals in U for SVD");

  factory.xAxisSet(residualV);

  m_UBResidualsPXDV = factory.CreateTH1F("UBResidualsPXDV", "Unbiased residuals in V for PXD");
  m_UBResidualsSVDV = factory.CreateTH1F("UBResidualsSVDV", "Unbiased residuals in V for SVD");
}

void DQMHistoModuleBase::DefineHelixParameters()
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

  m_PhiD0 = factory.xAxis(phi).yAxis(D0).CreateTH2F("PhiD0", "d0 vs Phi - the signed distance to the IP in the r-phi plane");
  m_D0Z0 =  factory.xAxis(D0).yAxis(Z0).CreateTH2F("D0Z0",
                                                   "z0 vs d0 - signed distance to the IP in r-phi vs. z0 of the perigee (to see primary vertex shifts along R or z)");

  originalDirectory->cd();
}

void DQMHistoModuleBase::DefineMomentum()
{
  int iMomRange = 60;
  double fMomRange = 3.0;

  auto momentum = THFAxis(2 * iMomRange, -fMomRange, fMomRange, "Momentum");
  auto factory = THFFactory(this).xAxisSet(momentum).yTitleSet("counts");

  m_MomX = factory.CreateTH1F("TrackMomentumX", "Track Momentum X");
  m_MomY = factory.CreateTH1F("TrackMomentumY", "Track Momentum Y");
  m_MomZ = factory.CreateTH1F("TrackMomentumZ", "Track Momentum Z");
  m_Mom = factory.xlow(.0).CreateTH1F("TrackMomentumMag", "Track Momentum Magnitude");
}

void DQMHistoModuleBase::DefineHits()
{
  int iHitsInPXD = 10;
  int iHitsInSVD = 20;
  int iHitsInCDC = 200;
  int iHits = 200;

  auto factory = THFFactory(this).xlowSet(0).xTitleSet("# hits").yTitleSet("counts");

  m_HitsPXD = factory.nbinsx(iHitsInPXD).xup(iHitsInPXD).CreateTH1F("NoOfHitsInTrack_PXD", "No Of Hits In Track - PXD");
  m_HitsSVD = factory.nbinsx(iHitsInSVD).xup(iHitsInSVD).CreateTH1F("NoOfHitsInTrack_SVD", "No Of Hits In Track - SVD");
  m_HitsCDC = factory.nbinsx(iHitsInCDC).xup(iHitsInCDC).CreateTH1F("NoOfHitsInTrack_CDC", "No Of Hits In Track - CDC");
  m_Hits = factory.nbinsx(iHits).xup(iHits).CreateTH1F("NoOfHitsInTrack", "No Of Hits In Track");
}

void DQMHistoModuleBase::DefineTracks()
{
  int iTracks = 30;

  auto tracks = THFAxis(iTracks, 0, iTracks, "# tracks");
  auto factory = THFFactory(this).xAxisSet(tracks).yTitleSet("counts");

  m_TracksVXD = factory.CreateTH1F("NoOfTracksInVXDOnly", "No Of Tracks Per Event, Only In VXD");
  m_TracksCDC = factory.CreateTH1F("NoOfTracksInCDCOnly", "No Of Tracks Per Event, Only In CDC");
  m_TracksVXDCDC = factory.CreateTH1F("NoOfTracksInVXDCDC", "No Of Tracks Per Event, In VXD+CDC");
  m_Tracks = factory.CreateTH1F("NoOfTracks", "No Of All Tracks Per Event");
}

void DQMHistoModuleBase::DefineHalfShells()
{
  TDirectory* originalDirectory = gDirectory;
  TDirectory* halfShells = originalDirectory->mkdir("HalfShells");
  halfShells->cd();

  double residualRange = 400;  // in um
  auto residual = THFAxis(200, -residualRange, residualRange, "residual [#mum]");
  auto factory = THFFactory(this).xAxisSet(residual).yTitleSet("counts");

  m_UBResidualsPXDX_Ying = factory.CreateTH1F("UBResidualsPXDX_Ying", "Unbiased residuals in X for PXD for Ying");
  m_UBResidualsPXDX_Yang = factory.CreateTH1F("UBResidualsPXDX_Yang", "Unbiased residuals in X for PXD for Yang");
  m_UBResidualsSVDX_Pat = factory.CreateTH1F("UBResidualsSVDX_Pat", "Unbiased residuals in X for SVD for Pat");
  m_UBResidualsSVDX_Mat = factory.CreateTH1F("UBResidualsSVDX_Mat", "Unbiased residuals in X for SVD for Mat");

  m_UBResidualsPXDY_Ying = factory.CreateTH1F("UBResidualsPXDY_Ying", "Unbiased residuals in Y for PXD for Ying");
  m_UBResidualsPXDY_Yang = factory.CreateTH1F("UBResidualsPXDY_Yang", "Unbiased residuals in Y for PXD for Yang");
  m_UBResidualsSVDY_Pat = factory.CreateTH1F("UBResidualsSVDY_Pat", "Unbiased residuals in Y for SVD for Pat");
  m_UBResidualsSVDY_Mat = factory.CreateTH1F("UBResidualsSVDY_Mat", "Unbiased residuals in Y for SVD for Mat");

  m_UBResidualsPXDZ_Ying = factory.CreateTH1F("UBResidualsPXDZ_Ying", "Unbiased residuals in Z for PXD for Ying");
  m_UBResidualsPXDZ_Yang = factory.CreateTH1F("UBResidualsPXDZ_Yang", "Unbiased residuals in Z for PXD for Yang");
  m_UBResidualsSVDZ_Pat = factory.CreateTH1F("UBResidualsSVDZ_Pat", "Unbiased residuals in Z for SVD for Pat");
  m_UBResidualsSVDZ_Mat = factory.CreateTH1F("UBResidualsSVDZ_Mat", "Unbiased residuals in Z for SVD for Mat");

  originalDirectory->cd();
}

void DQMHistoModuleBase::DefineClusters()
{
  double range = 180; // in um
  int nbins = 360;

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();

  int nVXDLayers = gTools->getNumberOfLayers();
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  m_TRClusterCorrelationsPhi = new TH2F*[nVXDLayers - 1];
  m_TRClusterCorrelationsTheta = new TH2F*[nVXDLayers - 1];

  /** Track related clusters - hitmap in IP angle range */
  m_TRClusterHitmap = CreateLayers(format("TRClusterHitmapLayer%1%"), format("Cluster Hitmap for layer %1%"), nbins, -range, range,
                                   nbins / 2, .0, range, "Phi angle [deg]", "Theta angle [deg]", "counts");

  for (VxdID layer : geo.getLayers()) {
    int layerNumber = layer.getLayerNumber();
    if (layerNumber == gTools->getLastLayer())
      continue;

    int layerIndex = gTools->getLayerIndex(layerNumber);

    /** Track related clusters - neighbor corelations in Phi */
    string name = str(format("CorrelationsPhiLayers_%1%_%2%") % layerNumber % (layerNumber + 1));
    string title = str(format("Correlations in Phi for Layers %1% %2%") % layerNumber % (layerNumber + 1));
    string xTitle = str(format("angle layer %1% [deg]") % layerNumber);
    string yTitle = str(format("angle layer %1% [deg]") % (layerNumber + 1));
    m_TRClusterCorrelationsPhi[layerIndex] = Create(name.c_str(), title.c_str(), nbins, -range, range, nbins, -range, range,
                                                    xTitle.c_str(), yTitle.c_str(), "counts");

    /** Track related clusters - neighbor corelations in Theta */
    name = str(format("CorrelationsThetaLayers_%1%_%2%") % layerNumber % (layerNumber + 1));
    title = str(format("Correlations in Theta for Layers %1% %2%") % layerNumber % (layerNumber + 1));
    m_TRClusterCorrelationsTheta[layerIndex] = Create(name.c_str(), title.c_str(), nbins / 2, .0, range, nbins / 2, .0, range,
                                                      xTitle.c_str(), yTitle.c_str(), "counts");
  }
}

void DQMHistoModuleBase::DefineSensors()
{
  TDirectory* originalDirectory = gDirectory;

  TDirectory* resids2D = originalDirectory->mkdir("Residuals2D");
  TDirectory* resids1D = originalDirectory->mkdir("Residuals1D");

  double residualRange = 400;  // in um

  auto residualU = THFAxis(200, -residualRange, residualRange, "residual U [#mum]");
  auto residualV = THFAxis(residualU).title("residual V [#mum]");

  auto factory = THFFactory(this);

  resids2D->cd();
  m_UBResidualsSensor = factory.xAxis(residualU).yAxis(residualV).zTitle("counts").CreateSensorsTH2F(format("UBResiduals_%1%"),
                        format("PXD Unbiased residuals for sensor %1%"));

  factory.yTitleSet("counts");

  resids1D->cd();
  m_UBResidualsSensorU = factory.xAxis(residualU).CreateSensorsTH1F(format("UBResidualsU_%1%"),
                         format("PXD Unbiased U residuals for sensor %1%"));
  m_UBResidualsSensorV = factory.xAxis(residualV).CreateSensorsTH1F(format("UBResidualsV_%1%"),
                         format("PXD Unbiased V residuals for sensor %1%"));

  originalDirectory->cd();
}

void DQMHistoModuleBase::FillTracks(int iTrack, int iTrackVXD, int iTrackCDC, int iTrackVXDCDC)
{
  m_Tracks->Fill(iTrack);
  m_TracksVXD->Fill(iTrackVXD);
  m_TracksCDC->Fill(iTrackCDC);
  m_TracksVXDCDC->Fill(iTrackVXDCDC);
}

void DQMHistoModuleBase::FillHits(int nPXD, int nSVD, int nCDC)
{
  m_HitsPXD->Fill(nPXD);
  m_HitsSVD->Fill(nSVD);
  m_HitsCDC->Fill(nCDC);
  m_Hits->Fill(nPXD + nSVD + nCDC);
}

void DQMHistoModuleBase::FillMomentum(const TrackFitResult* tfr)
{
  float px = tfr->getMomentum().Px();
  float py = tfr->getMomentum().Py();
  float pz = tfr->getMomentum().Pz();

  float Phi = atan2(py, px);
  float pxy = sqrt(px * px + py * py);
  float Theta = atan2(pxy, pz);

  m_MomPhi->Fill(Phi);
  m_MomTheta->Fill(Theta);
  m_MomCosTheta->Fill(cos(Theta));
}

void DQMHistoModuleBase::FillTrackFitResult(const TrackFitResult* tfr)
{
  m_MomX->Fill(tfr->getMomentum().Px());
  m_MomY->Fill(tfr->getMomentum().Py());
  m_MomZ->Fill(tfr->getMomentum().Pz());
  m_MomPt->Fill(tfr->getMomentum().Pt());
  m_Mom->Fill(tfr->getMomentum().Mag());

  m_D0->Fill(tfr->getD0());
  m_PhiD0->Fill(tfr->getPhi0() * Unit::convertValueToUnit(1.0, "deg"), tfr->getD0());
  m_Z0->Fill(tfr->getZ0());
  m_D0Z0->Fill(tfr->getZ0(), tfr->getD0());

  m_Phi->Fill(tfr->getPhi() * Unit::convertValueToUnit(1.0, "deg"));
  m_Omega->Fill(tfr->getOmega());
  m_TanLambda->Fill(tfr->getTanLambda());
}

void DQMHistoModuleBase::FillTrackFitStatus(const genfit::FitStatus* tfs)
{
  float NDF = tfs->getNdf();
  m_NDF->Fill(NDF);

  m_Chi2->Fill(tfs->getChi2());
  if (NDF) {
    float Chi2NDF = tfs->getChi2() / NDF;
    m_Chi2NDF->Fill(Chi2NDF);
  }

  m_PValue->Fill(tfs->getPVal());
}

void DQMHistoModuleBase::FillCorrelations(float fPosSPU, float fPosSPUPrev, float fPosSPV, float fPosSPVPrev,
                                          int correlationIndex)
{
  m_TRClusterCorrelationsPhi[correlationIndex]->Fill(fPosSPUPrev, fPosSPU);
  m_TRClusterCorrelationsTheta[correlationIndex]->Fill(fPosSPVPrev, fPosSPV);
}

void DQMHistoModuleBase::FillUBResidualsPXD(float residUPlaneRHUnBias, float residVPlaneRHUnBias)
{
  m_UBResidualsPXD->Fill(residUPlaneRHUnBias, residVPlaneRHUnBias);
  m_UBResidualsPXDU->Fill(residUPlaneRHUnBias);
  m_UBResidualsPXDV->Fill(residVPlaneRHUnBias);
}

void DQMHistoModuleBase::FillUBResidualsSVD(float residUPlaneRHUnBias, float residVPlaneRHUnBias)
{
  m_UBResidualsSVD->Fill(residUPlaneRHUnBias, residVPlaneRHUnBias);
  m_UBResidualsSVDU->Fill(residUPlaneRHUnBias);
  m_UBResidualsSVDV->Fill(residVPlaneRHUnBias);
}

void DQMHistoModuleBase::FillPXDHalfShells(float residUPlaneRHUnBias, float residVPlaneRHUnBias,
                                           const VXD::SensorInfoBase* sensorInfo, bool isNotYang)
{
  TVector3 localResidual(residUPlaneRHUnBias, residVPlaneRHUnBias, 0);
  auto globalResidual = sensorInfo->vectorToGlobal(localResidual, true);

  if (isNotYang) {
    m_UBResidualsPXDX_Ying->Fill(globalResidual.x());
    m_UBResidualsPXDY_Ying->Fill(globalResidual.y());
    m_UBResidualsPXDZ_Ying->Fill(globalResidual.z());
  } else {
    m_UBResidualsPXDX_Yang->Fill(globalResidual.x());
    m_UBResidualsPXDY_Yang->Fill(globalResidual.y());
    m_UBResidualsPXDZ_Yang->Fill(globalResidual.z());
  }
}

void DQMHistoModuleBase::FillSVDHalfShells(float residUPlaneRHUnBias, float residVPlaneRHUnBias,
                                           const VXD::SensorInfoBase* sensorInfo, bool isNotMat)
{
  TVector3 localResidual(residUPlaneRHUnBias, residVPlaneRHUnBias, 0);
  auto globalResidual = sensorInfo->vectorToGlobal(localResidual, true);

  if (isNotMat) {
    m_UBResidualsSVDX_Pat->Fill(globalResidual.x());
    m_UBResidualsSVDY_Pat->Fill(globalResidual.y());
    m_UBResidualsSVDZ_Pat->Fill(globalResidual.z());
  } else {
    m_UBResidualsSVDX_Mat->Fill(globalResidual.x());
    m_UBResidualsSVDY_Mat->Fill(globalResidual.y());
    m_UBResidualsSVDZ_Mat->Fill(globalResidual.z());
  }
}

void DQMHistoModuleBase::FillUBResidualsSensor(float residUPlaneRHUnBias, float residVPlaneRHUnBias, int sensorIndex)
{
  m_UBResidualsSensor[sensorIndex]->Fill(residUPlaneRHUnBias, residVPlaneRHUnBias);
  m_UBResidualsSensorU[sensorIndex]->Fill(residUPlaneRHUnBias);
  m_UBResidualsSensorV[sensorIndex]->Fill(residVPlaneRHUnBias);
}

void DQMHistoModuleBase::FillTRClusterHitmap(float fPosSPU, float fPosSPV, int layerIndex)
{
  m_TRClusterHitmap[layerIndex]->Fill(fPosSPU, fPosSPV);
}

void DQMHistoModuleBase::ComputeMean(TH1F* output, TH2F* input, bool onX)
{
  output->Reset();
  int nbinsi = onX ? input->GetNbinsX() : input->GetNbinsY();
  int nbinsy = onX ? input->GetNbinsY() : input->GetNbinsX();
  TAxis* axis = onX ? input->GetYaxis() : input->GetXaxis();

  for (int i = 1; i <= nbinsi; i++) {
    float sum = 0;
    float count = 0;

    for (int j = 1; j <= nbinsy; j++) {
      float value = onX ? input->GetBinContent(i, j) : input->GetBinContent(j, i);
      sum += value * axis->GetBinCenter(j);
      count += value;
    }

    output->SetBinContent(i, count != 0 ? sum / count : 0);
  }
}

void DQMHistoModuleBase::ProcessHistogramParameterChange(string name, string parameter, string value)
{
  TH1* histogram;
  bool found = false;

  for (auto adept : m_histograms)
    if (adept->GetName() == name) {
      found = true;
      histogram = adept;
      break;
    }

  if (!found) {
    B2WARNING("Histogram " + name + " not found, parameter change is skipped in " + getName() + ".");
    return;
  }

  try {
    EditHistogramParameter(histogram, parameter, value);
  } catch (const invalid_argument& e) {
    B2WARNING("Value " + value + " of parameter " + parameter + " for histogram " + histogram->GetName() +
              " could not be parsed, parameter change is skipped in " + getName() + ".");
  } catch (const out_of_range& e) {
    B2WARNING("Value " + value + " of parameter " + parameter + " for histogram " + histogram->GetName() +
              " is out of range, parameter change is skipped in " + getName() + ".");
  }
}

void DQMHistoModuleBase::EditHistogramParameter(TH1* histogram, string parameter, string value)
{
  if (parameter == "title") {
    histogram->SetTitle(value.c_str());
    return;
  }
  if (parameter == "nbinsx") {
    auto axis = histogram->GetXaxis();
    axis->Set(stoi(value), axis->GetXmin(), axis->GetXmax());
    return;
  }
  if (parameter == "xlow") {
    auto axis = histogram->GetXaxis();
    axis->Set(axis->GetNbins(), stod(value), axis->GetXmax());
    return;
  }
  if (parameter == "xup") {
    auto axis = histogram->GetXaxis();
    axis->Set(axis->GetNbins(), axis->GetXmin(), stod(value));
    return;
  }
  if (parameter == "xTitle") {
    histogram->GetXaxis()->SetTitle(value.c_str());
    return;
  }
  if (parameter == "yTitle") {
    histogram->GetYaxis()->SetTitle(value.c_str());
    return;
  }

  if (dynamic_cast<TH2F*>(histogram) == nullptr) {
    B2WARNING("Parameter " + parameter + " not found in histogram " + histogram->GetName() + ", parameter change is skipped in " +
              getName() + ".");
    return;
  }

  if (parameter == "nbinsy") {
    auto axis = histogram->GetYaxis();
    axis->Set(stoi(value), axis->GetXmin(), axis->GetXmax());
    return;
  }
  if (parameter == "ylow") {
    auto axis = histogram->GetYaxis();
    axis->Set(axis->GetNbins(), stod(value), axis->GetXmax());
    return;
  }
  if (parameter == "yup") {
    auto axis = histogram->GetYaxis();
    axis->Set(axis->GetNbins(), axis->GetXmin(), stod(value));
    return;
  }
  if (parameter == "zTitle") {
    histogram->GetZaxis()->SetTitle(value.c_str());
    return;
  }

  B2WARNING("Parameter " + parameter + " not found in histogram " + histogram->GetName() + ", parameter change is skipped in " +
            getName() + ".");
}