/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/dqmUtils/DQMHistoModuleBase.h>
#include <tracking/dqmUtils/HistogramFactory.h>

#include <framework/datastore/StoreArray.h>
#include <vxd/geometry/GeoTools.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <TDirectory.h>

using namespace Belle2;
using namespace Belle2::HistogramFactory;
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

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

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

TH1F* DQMHistoModuleBase::Create(std::string name, std::string title, int nbinsx, double xlow, double xup, std::string xTitle,
                                 std::string yTitle)
{
  TH1F* histogram = new TH1F(name.c_str(), title.c_str(), nbinsx, xlow, xup);
  histogram->GetXaxis()->SetTitle(xTitle.c_str());
  histogram->GetYaxis()->SetTitle(yTitle.c_str());

  m_histograms.push_back(histogram);

  return histogram;
}

TH2F* DQMHistoModuleBase::Create(std::string name, std::string title, int nbinsx, double xlow, double xup, int nbinsy, double ylow,
                                 double yup, std::string xTitle, std::string yTitle, std::string zTitle)
{
  TH2F* histogram = new TH2F(name.c_str(), title.c_str(), nbinsx, xlow, xup, nbinsy, ylow, yup);
  histogram->GetXaxis()->SetTitle(xTitle.c_str());
  histogram->GetYaxis()->SetTitle(yTitle.c_str());
  histogram->GetZaxis()->SetTitle(zTitle.c_str());

  m_histograms.push_back(histogram);

  return histogram;
}

std::string DQMHistoModuleBase::SensorNameDescription(VxdID sensorID)
{
  return str(format("%1%_%2%_%3%") % sensorID.getLayerNumber() % sensorID.getLadderNumber() % sensorID.getSensorNumber());
}

std::string DQMHistoModuleBase::SensorTitleDescription(VxdID sensorID)
{
  return str(format("Layer %1% Ladder %2% Sensor %3%") % sensorID.getLayerNumber() % sensorID.getLadderNumber() %
             sensorID.getSensorNumber());
}

TH1F** DQMHistoModuleBase::CreateLayers(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow,
                                        double xup, std::string xTitle, std::string yTitle)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  auto gTools = geo.getGeoTools();

  TH1F** output = new TH1F*[gTools->getNumberOfLayers()];

  for (VxdID layer : geo.getLayers()) {
    int layerNumber = layer.getLayerNumber();
    int layerIndex = gTools->getLayerIndex(layerNumber);
    std::string name = str(nameTemplate % layerNumber);
    std::string title = str(titleTemplate % layerNumber);
    output[layerIndex] = Create(name, title, nbinsx, xlow, xup, xTitle, yTitle);
  }

  return output;
}

TH2F** DQMHistoModuleBase::CreateLayers(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow,
                                        double xup, int nbinsy, double ylow, double yup, std::string xTitle, std::string yTitle, std::string zTitle)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  auto gTools = geo.getGeoTools();

  TH2F** output = new TH2F*[gTools->getNumberOfLayers()];

  for (VxdID layer : geo.getLayers()) {
    int layerNumber = layer.getLayerNumber();
    int layerIndex = gTools->getLayerIndex(layerNumber);
    std::string name = str(nameTemplate % layerNumber);
    std::string title = str(titleTemplate % layerNumber);
    output[layerIndex] = Create(name, title, nbinsx, xlow, xup, nbinsy, ylow, yup, xTitle, yTitle, zTitle);
  }

  return output;
}

TH1F** DQMHistoModuleBase::CreateSensors(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow,
                                         double xup, std::string xTitle, std::string yTitle)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  auto gTools = geo.getGeoTools();
  int nVXDSensors = gTools->getNumberOfSensors();

  TH1F** output = new TH1F*[nVXDSensors];

  for (int sensorIndex = 0; sensorIndex < nVXDSensors; sensorIndex++) {
    VxdID sensorID = gTools->getSensorIDFromIndex(sensorIndex);
    std::string name = str(nameTemplate % SensorNameDescription(sensorID));
    std::string title = str(titleTemplate % SensorTitleDescription(sensorID));
    output[sensorIndex] = Create(name, title, nbinsx, xlow, xup, xTitle, yTitle);
  }

  return output;
}

TH2F** DQMHistoModuleBase::CreateSensors(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow,
                                         double xup, int nbinsy, double ylow, double yup, std::string xTitle, std::string yTitle, std::string zTitle)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  auto gTools = geo.getGeoTools();
  int nVXDSensors = gTools->getNumberOfSensors();

  TH2F** output = new TH2F*[nVXDSensors];

  for (int sensorIndex = 0; sensorIndex < nVXDSensors; sensorIndex++) {
    VxdID sensorID = gTools->getSensorIDFromIndex(sensorIndex);
    std::string name = str(nameTemplate % SensorNameDescription(sensorID));
    std::string title = str(titleTemplate % SensorTitleDescription(sensorID));
    output[sensorIndex] = Create(name, title, nbinsx, xlow, xup, nbinsy, ylow, yup, xTitle, yTitle, zTitle);
  }

  return output;
}

void DQMHistoModuleBase::DefineMomentumAngles()
{
  m_MomPhi = Create("MomPhi", "Track Azimuthal Angle", 180, -180, 180, "Mom Phi", "counts");
  m_MomTheta = Create("MomTheta", "Track Polar Angle", 90, 0, 180, "Mom Theta", "counts");
  m_MomCosTheta = Create("MomCosTheta", "Cosine of the Track Polar Angle", 100, -1, 1, "Mom CosTheta", "counts");
}

void DQMHistoModuleBase::DefineTrackFitStatus()
{
  m_PValue = Create("PValue", "Track Fit P value", 100, 0, 1, "p-value", "counts");
  m_Chi2 = Create("Chi2", "Track Fit Chi2", 200, 0, 150, "Chi2", "counts");
  m_NDF = Create("NDF", "Track Fit NDF", 200, 0, 200, "NDF", "counts");
  m_Chi2NDF = Create("Chi2NDF", "Track Fit Chi2/NDF", 200, 0, 10, "Chi2/NDF", "counts");
}

void DQMHistoModuleBase::DefineUBResidualsVXD()
{
  double residualRange = 400;  // in um

  auto residualU = Axis(200, -residualRange, residualRange, "residual U [#mum]");
  auto residualV = Axis(residualU).title("residual V [#mum]");

  auto factory = Factory(this);
  factory.xAxisDefault(residualU).yAxisDefault(residualV).zTitleDefault("counts");

  if (! m_hltDQM)
    m_UBResidualsPXD = factory.CreateTH2F("UBResidualsPXD", "PXD Unbiased Residuals");
  m_UBResidualsSVD = factory.CreateTH2F("UBResidualsSVD", "SVD Unbiased Residuals");

  factory.xAxisDefault(residualU).yTitleDefault("counts");

  if (! m_hltDQM)
    m_UBResidualsPXDU = factory.CreateTH1F("UBResidualsPXDU", "PXD Unbiased residuals in U");
  m_UBResidualsSVDU = factory.CreateTH1F("UBResidualsSVDU", "SVD Unbiased residuals in U");

  factory.xAxisDefault(residualV);

  if (! m_hltDQM)
    m_UBResidualsPXDV = factory.CreateTH1F("UBResidualsPXDV", "PXD Unbiased residuals in V");
  m_UBResidualsSVDV = factory.CreateTH1F("UBResidualsSVDV", "SVD Unbiased residuals in V");
}

void DQMHistoModuleBase::DefineHelixParametersAndCorrelations()
{

  int iZ0Range = 200;
  double fZ0Range = 10.0;     // Half range in cm
  int iD0Range = 200;
  double fD0Range = 2.0;      // Half range in cm
  int iPhiRange = 72;
  double fPhiRange = 180.0;   // Half range in deg
  int iLambdaRange = 400;
  double fLambdaRange = 4.0;
  int iOmegaRange = 400;
  double fOmegaRange = 0.1;

  auto phi = Axis(iPhiRange, -fPhiRange, fPhiRange, "#phi [deg]");
  auto D0 = Axis(iD0Range, -fD0Range, fD0Range, "d0 [cm]");
  auto D0_2d = Axis(50, -0.5, 0.5, "d0 [cm]");
  auto Z0 = Axis(iZ0Range, -fZ0Range, fZ0Range, "z0 [cm]");
  auto Z0_2d = Axis(100, -2, 2, "z0 [cm]");
  auto tanLambda = Axis(iLambdaRange, -fLambdaRange, fLambdaRange, "Tan Lambda");
  auto omega = Axis(iOmegaRange, -fOmegaRange, fOmegaRange, "Omega");

  auto factory = Factory(this);

  factory.yTitleDefault("Arb. Units");

  m_Z0 =        factory.xAxis(Z0).CreateTH1F("HelixZ0", "z0, the z coordinate of the perigee");
  m_D0 =        factory.xAxis(D0).CreateTH1F("HelixD0", "d0, the signed distance of the perigee in the r-phi plane");
  m_Phi =       factory.xAxis(phi).CreateTH1F("HelixPhi",
                                              "Phi0, momentum azymuthal angle at the perigee");
  m_Omega =     factory.xAxis(omega).CreateTH1F("HelixOmega",
                                                "Omega, the curvature of the track, sign defined by the charge of the particle");
  m_TanLambda = factory.xAxis(tanLambda).CreateTH1F("HelixTanLambda", "TanLambda, the slope of the track in the r-z plane");


  factory.zTitleDefault("Arb. Units");

  m_PhiD0 = factory.xAxis(phi).yAxis(D0_2d).CreateTH2F("Helix2dPhiD0",
                                                       "d0 vs Phi0, the signed distance of the perigee in the r-phi plane vs. momentum azymuthal angle at the perigee");
  m_D0Z0 =  factory.xAxis(Z0_2d).yAxis(D0_2d).CreateTH2F("Helix2dD0Z0",
                                                         "d0 vs z0, the signed distance of the perigee in r-phi vs. z0 of the perigee");

}

void DQMHistoModuleBase::DefineMomentumCoordinates()
{
  int iMomRange = 100;
  double fMomRange = 6.0;

  auto momentum = Axis(2 * iMomRange, -fMomRange, fMomRange, "Momentum [GeV/c]");
  auto pt_momentum = Axis(iMomRange, 0, fMomRange, "Momentum [GeV/c]");
  auto factory = Factory(this).xAxisDefault(momentum).yTitleDefault("counts");

  m_MomX = factory.CreateTH1F("TrackMomentumX", "Track Momentum X");
  m_MomY = factory.CreateTH1F("TrackMomentumY", "Track Momentum Y");
  m_MomZ = factory.CreateTH1F("TrackMomentumZ", "Track Momentum Z");
  m_MomPt = factory.xAxis(pt_momentum).yTitle("counts").CreateTH1F("TrackMomentumPt", "Track Momentum pT");
  m_Mom = factory.xlow(.0).CreateTH1F("TrackMomentumMag", "Track Momentum Magnitude");
}

void DQMHistoModuleBase::DefineHits()
{
  int iHitsInSVD = 20;
  int iHitsInCDC = 200;
  int iHits = 200;

  auto factory = Factory(this).xlowDefault(0).xTitleDefault("# hits").yTitleDefault("counts");

  if (! m_hltDQM) {
    int iHitsInPXD = 10;
    m_HitsPXD = factory.nbinsx(iHitsInPXD).xup(iHitsInPXD).CreateTH1F("NoOfHitsInTrack_PXD", "Number of PXD Hits per Track");
  }
  m_HitsSVD = factory.nbinsx(iHitsInSVD).xup(iHitsInSVD).CreateTH1F("NoOfHitsInTrack_SVD", "Number of SVD Hits per Track");
  m_HitsCDC = factory.nbinsx(iHitsInCDC).xup(iHitsInCDC).CreateTH1F("NoOfHitsInTrack_CDC", "Number of CDC Hits per Track");
  m_Hits = factory.nbinsx(iHits).xup(iHits).CreateTH1F("NoOfHitsInTrack", "Number of Hits per Track");
}

void DQMHistoModuleBase::DefineTracks()
{
  int iTracks = 30;

  auto tracks = Axis(iTracks, 0, iTracks, "# tracks");
  auto factory = Factory(this).xAxisDefault(tracks).yTitleDefault("counts");

  m_TracksVXD = factory.CreateTH1F("NoOfTracksInVXDOnly", "Number of VXD-Only Tracks per Event");
  m_TracksCDC = factory.CreateTH1F("NoOfTracksInCDCOnly", "Number of CDC-Only Tracks per Event");
  m_TracksVXDCDC = factory.CreateTH1F("NoOfTracksInVXDCDC", "Number of VXD+CDC Tracks per Event");
  m_Tracks = factory.CreateTH1F("NoOfTracks", "Number of Tracks per Event");
}

void DQMHistoModuleBase::DefineHalfShellsVXD()
{

  double residualRange = 400;  // in um
  auto residual = Axis(200, -residualRange, residualRange, "residual [#mum]");
  auto factory = Factory(this).xAxisDefault(residual).yTitleDefault("counts");

  if (! m_hltDQM) {
    m_UBResidualsPXDX_Yin = factory.CreateTH1F("UBResidualsPXDX_Yin", "PXD-Yin Unbiased Residuals in X");
    m_UBResidualsPXDX_Yang = factory.CreateTH1F("UBResidualsPXDX_Yang", "PXD-Yang Unbiased Residuals in X");
  }
  m_UBResidualsSVDX_Pat = factory.CreateTH1F("UBResidualsSVDX_Pat", "SVD-Pat Unbiased Residuals in X");
  m_UBResidualsSVDX_Mat = factory.CreateTH1F("UBResidualsSVDX_Mat", "SVD-Mat Unbiased Residuals in X");

  if (! m_hltDQM) {
    m_UBResidualsPXDY_Yin = factory.CreateTH1F("UBResidualsPXDY_Yin", "PXD-Yin Unbiased Residuals in Y");
    m_UBResidualsPXDY_Yang = factory.CreateTH1F("UBResidualsPXDY_Yang", "PXD-Yang Unbiased Residuals in Y");
  }
  m_UBResidualsSVDY_Pat = factory.CreateTH1F("UBResidualsSVDY_Pat", "SVD-Pat Unbiased Residuals in Y");
  m_UBResidualsSVDY_Mat = factory.CreateTH1F("UBResidualsSVDY_Mat", "SVD-Mat Unbiased Residuals in Y");

  if (! m_hltDQM) {
    m_UBResidualsPXDZ_Yin = factory.CreateTH1F("UBResidualsPXDZ_Yin", "PXD-Yin Unbiased Residuals in Z");
    m_UBResidualsPXDZ_Yang = factory.CreateTH1F("UBResidualsPXDZ_Yang", "PXD-Yang Unbiased Residuals in Z");
  }
  m_UBResidualsSVDZ_Pat = factory.CreateTH1F("UBResidualsSVDZ_Pat", "SVD-Pat Unbiased Residuals in Z");
  m_UBResidualsSVDZ_Mat = factory.CreateTH1F("UBResidualsSVDZ_Mat", "SVD-Mat Unbiased Residuals in Z");

}

void DQMHistoModuleBase::DefineTRClusters()
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
    std::string name = str(format("CorrelationsPhiLayers_%1%_%2%") % layerNumber % (layerNumber + 1));
    std::string title = str(format("Correlations in Phi for Layers %1% %2%") % layerNumber % (layerNumber + 1));
    std::string xTitle = str(format("angle layer %1% [deg]") % layerNumber);
    std::string yTitle = str(format("angle layer %1% [deg]") % (layerNumber + 1));
    m_TRClusterCorrelationsPhi[layerIndex] = Create(name, title, nbins, -range, range, nbins, -range, range,
                                                    xTitle, yTitle, "counts");

    /** Track related clusters - neighbor corelations in Theta */
    name = str(format("CorrelationsThetaLayers_%1%_%2%") % layerNumber % (layerNumber + 1));
    title = str(format("Correlations in Theta for Layers %1% %2%") % layerNumber % (layerNumber + 1));
    m_TRClusterCorrelationsTheta[layerIndex] = Create(name, title, nbins / 2, .0, range, nbins / 2, .0, range,
                                                      xTitle, yTitle, "counts");
  }
}

void DQMHistoModuleBase::Define1DSensors()
{

  double residualRange = 400;  // in um

  auto residualU = Axis(200, -residualRange, residualRange, "residual U [#mum]");
  auto residualV = Axis(residualU).title("residual V [#mum]");

  auto factory = Factory(this);

  factory.yTitleDefault("counts");

  m_UBResidualsSensorU = factory.xAxis(residualU).CreateSensorsTH1F(format("UBResidualsU_%1%"),
                         format("VXD Unbiased U Residuals for sensor %1%"));
  m_UBResidualsSensorV = factory.xAxis(residualV).CreateSensorsTH1F(format("UBResidualsV_%1%"),
                         format("VXD Unbiased V Residuals for sensor %1%"));

}

void DQMHistoModuleBase::Define2DSensors()
{

  double residualRange = 400;  // in um

  auto residualU = Axis(200, -residualRange, residualRange, "residual U [#mum]");
  auto residualV = Axis(residualU).title("residual V [#mum]");

  auto factory = Factory(this);

  m_UBResidualsSensor = factory.xAxis(residualU).yAxis(residualV).zTitle("counts").CreateSensorsTH2F(format("UBResiduals_%1%"),
                        format("VXD Unbiased Residuals for sensor %1%"));
}

void DQMHistoModuleBase::FillTrackIndexes(int iTrack, int iTrackVXD, int iTrackCDC, int iTrackVXDCDC)
{
  m_Tracks->Fill(iTrack);
  m_TracksVXD->Fill(iTrackVXD);
  m_TracksCDC->Fill(iTrackCDC);
  m_TracksVXDCDC->Fill(iTrackVXDCDC);
}

void DQMHistoModuleBase::FillHitNumbers(int nPXD, int nSVD, int nCDC)
{
  if (! m_hltDQM)
    m_HitsPXD->Fill(nPXD);
  else nPXD = 0;
  m_HitsSVD->Fill(nSVD);
  m_HitsCDC->Fill(nCDC);
  m_Hits->Fill(nPXD + nSVD + nCDC);
}

void DQMHistoModuleBase::FillMomentumAngles(const TrackFitResult* tfr)
{
  const ROOT::Math::XYZVector mom = tfr->getMomentum();

  // don't fill NAN or INF
  if (checkVariableForNANOrINF(mom.X()) or checkVariableForNANOrINF(mom.Y()) or checkVariableForNANOrINF(mom.Z()) or
      checkVariableForNANOrINF(mom.Phi()) or checkVariableForNANOrINF(mom.Theta())) {
    return;
  }

  float Phi = mom.Phi();
  float Theta = mom.Theta();

  m_MomPhi->Fill(Phi / Unit::deg);
  m_MomTheta->Fill(Theta / Unit::deg);
  m_MomCosTheta->Fill(cos(Theta));
}

void DQMHistoModuleBase::FillMomentumCoordinates(const TrackFitResult* tfr)
{
  const ROOT::Math::XYZVector& mom = tfr->getMomentum();

  // don't fill NAN or INF, Mag is NAN/INF if any component is, no need to check
  if (checkVariableForNANOrINF(mom.X()) or checkVariableForNANOrINF(mom.Y()) or
      checkVariableForNANOrINF(mom.Z()) or checkVariableForNANOrINF(mom.Rho())) {
    return;
  }

  m_MomX->Fill(mom.X());
  m_MomY->Fill(mom.Y());
  m_MomZ->Fill(mom.Z());
  m_Mom->Fill(mom.R());
  m_MomPt->Fill(mom.Rho());
}

void DQMHistoModuleBase::FillHelixParametersAndCorrelations(const TrackFitResult* tfr)
{
  // don't fill NAN or INF
  if (checkVariableForNANOrINF(tfr->getD0()) or checkVariableForNANOrINF(tfr->getZ0()) or checkVariableForNANOrINF(tfr->getPhi()) or
      checkVariableForNANOrINF(tfr->getOmega()) or checkVariableForNANOrINF(tfr->getTanLambda())) {
    return;
  }

  m_D0->Fill(tfr->getD0());
  m_Z0->Fill(tfr->getZ0());
  m_Phi->Fill(tfr->getPhi() / Unit::deg);
  m_Omega->Fill(tfr->getOmega());
  m_TanLambda->Fill(tfr->getTanLambda());

  m_PhiD0->Fill(tfr->getPhi0() / Unit::deg, tfr->getD0());
  m_D0Z0->Fill(tfr->getZ0(), tfr->getD0());
}

void DQMHistoModuleBase::FillTrackFitStatus(const genfit::FitStatus* tfs)
{
  // don't fill NAN or INF
  if (checkVariableForNANOrINF(tfs->getChi2()) or checkVariableForNANOrINF(tfs->getNdf())) {
    return;
  }

  float NDF = tfs->getNdf();
  m_NDF->Fill(NDF);
  float chi2 = tfs->getChi2();
  m_Chi2->Fill(chi2);
  if (NDF > 0) {
    float Chi2NDF = chi2 / NDF;
    m_Chi2NDF->Fill(Chi2NDF);
  }

  m_PValue->Fill(tfs->getPVal());
}

void DQMHistoModuleBase::FillTRClusterCorrelations(float phi_deg, float phiPrev_deg, float theta_deg, float thetaPrev_deg,
                                                   int correlationIndex)
{
  m_TRClusterCorrelationsPhi[correlationIndex]->Fill(phiPrev_deg, phi_deg);
  m_TRClusterCorrelationsTheta[correlationIndex]->Fill(thetaPrev_deg, theta_deg);
}

void DQMHistoModuleBase::FillUBResidualsPXD(const B2Vector3D& residual_um)
{
  if (checkVariableForNANOrINF(residual_um.X()) or checkVariableForNANOrINF(residual_um.Y())) {
    return;
  }
  m_UBResidualsPXD->Fill(residual_um.x(), residual_um.y());
  m_UBResidualsPXDU->Fill(residual_um.x());
  m_UBResidualsPXDV->Fill(residual_um.y());
}

void DQMHistoModuleBase::FillUBResidualsSVD(const B2Vector3D& residual_um)
{
  if (checkVariableForNANOrINF(residual_um.X()) or checkVariableForNANOrINF(residual_um.Y())) {
    return;
  }
  m_UBResidualsSVD->Fill(residual_um.x(), residual_um.y());
  m_UBResidualsSVDU->Fill(residual_um.x());
  m_UBResidualsSVDV->Fill(residual_um.y());
}

void DQMHistoModuleBase::FillHalfShellsPXD(const B2Vector3D& globalResidual_um, bool isNotYang)
{
  if (checkVariableForNANOrINF(globalResidual_um.X()) or
      checkVariableForNANOrINF(globalResidual_um.Y()) or
      checkVariableForNANOrINF(globalResidual_um.Z())) {
    return;
  }
  if (isNotYang) {
    m_UBResidualsPXDX_Yin->Fill(globalResidual_um.x());
    m_UBResidualsPXDY_Yin->Fill(globalResidual_um.y());
    m_UBResidualsPXDZ_Yin->Fill(globalResidual_um.z());
  } else {
    m_UBResidualsPXDX_Yang->Fill(globalResidual_um.x());
    m_UBResidualsPXDY_Yang->Fill(globalResidual_um.y());
    m_UBResidualsPXDZ_Yang->Fill(globalResidual_um.z());
  }
}

void DQMHistoModuleBase::FillHalfShellsSVD(const B2Vector3D& globalResidual_um, bool isNotMat)
{
  if (checkVariableForNANOrINF(globalResidual_um.X()) or
      checkVariableForNANOrINF(globalResidual_um.Y()) or
      checkVariableForNANOrINF(globalResidual_um.Z())) {
    return;
  }
  if (isNotMat) {
    m_UBResidualsSVDX_Pat->Fill(globalResidual_um.x());
    m_UBResidualsSVDY_Pat->Fill(globalResidual_um.y());
    m_UBResidualsSVDZ_Pat->Fill(globalResidual_um.z());
  } else {
    m_UBResidualsSVDX_Mat->Fill(globalResidual_um.x());
    m_UBResidualsSVDY_Mat->Fill(globalResidual_um.y());
    m_UBResidualsSVDZ_Mat->Fill(globalResidual_um.z());
  }
}

void DQMHistoModuleBase::FillUB1DResidualsSensor(const B2Vector3D& residual_um, int sensorIndex)
{
  if (checkVariableForNANOrINF(residual_um.X()) or checkVariableForNANOrINF(residual_um.Y())) {
    return;
  }
  m_UBResidualsSensorU[sensorIndex]->Fill(residual_um.x());
  m_UBResidualsSensorV[sensorIndex]->Fill(residual_um.y());
}

void DQMHistoModuleBase::FillUB2DResidualsSensor(const B2Vector3D& residual_um, int sensorIndex)
{
  if (checkVariableForNANOrINF(residual_um.X()) or checkVariableForNANOrINF(residual_um.Y())) {
    return;
  }
  m_UBResidualsSensor[sensorIndex]->Fill(residual_um.x(), residual_um.y());
}

void DQMHistoModuleBase::FillTRClusterHitmap(float phi_deg, float theta_deg, int layerIndex)
{
  m_TRClusterHitmap[layerIndex]->Fill(phi_deg, theta_deg);
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

void DQMHistoModuleBase::ProcessHistogramParameterChange(const std::string& name, const std::string& parameter,
                                                         const std::string& value)
{
  TH1* histogram = nullptr;

  for (auto adept : m_histograms)
    if (adept->GetName() == name) {
      histogram = adept;
      break;
    }

  if (!histogram) {
    B2WARNING("Histogram " + name + " not found, parameter change is skipped in " + getName() + ".");
  } else {
    try {
      EditHistogramParameter(histogram, parameter, value);
    } catch (const std::invalid_argument& e) {
      B2WARNING("Value " + value + " of parameter " + parameter + " for histogram " + histogram->GetName() +
                " could not be parsed, parameter change is skipped in " + getName() + ".");
    } catch (const std::out_of_range& e) {
      B2WARNING("Value " + value + " of parameter " + parameter + " for histogram " + histogram->GetName() +
                " is out of range, parameter change is skipped in " + getName() + ".");
    }
  }
}

void DQMHistoModuleBase::EditHistogramParameter(TH1* histogram, const std::string& parameter, std::string value)
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
