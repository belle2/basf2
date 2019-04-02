/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin, Vipin Gaur                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <TDirectory.h>

/* Belle2 headers. */
#include <klm/modules/KLMDQM/KLMDQMModule.h>

using namespace Belle2;

REG_MODULE(KLMDQM)

KLMDQMModule::KLMDQMModule() :
  HistoModule(),
  m_TimeRPC(nullptr),
  m_TimeScintillatorBKLM(nullptr),
  m_TimeScintillatorEKLM(nullptr),
  m_layerHits(nullptr),
  m_eDep(nullptr),
  m_nPixel(nullptr),
  m_zStrips(nullptr),
  m_phiStrip(nullptr),
  m_sector(nullptr),
  m_layer(nullptr),
  m_rBKLMHit2ds(nullptr),
  m_zBKLMHit2ds(nullptr),
  m_yvsxBKLMHit2ds(nullptr),
  m_xvszBKLMHit2ds(nullptr),
  m_yvszBKLMHit2ds(nullptr)
{
  setDescription("KLM data quality monitor.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histogramDirectoryName", m_HistogramDirectoryName,
           "Directory for KLM DQM histograms in ROOT file.",
           std::string("KLM"));
  addParam("histogramDirectoryNameEKLM", m_HistogramDirectoryNameEKLM,
           "Directory for EKLM DQM histograms in ROOT file.",
           std::string("EKLM"));
  addParam("histogramDirectoryNameBKLM", m_HistogramDirectoryNameBKLM,
           "Directory for BKLM DQM histograms in ROOT file.",
           std::string("BKLM"));
  addParam("outputDigitsName", m_outputDigitsName,
           "Name of BKLMDigit store array", std::string("BKLMDigits"));
  m_Elements = &(EKLM::ElementNumbersSingleton::Instance());
  m_Sector = nullptr;
  m_StripLayer = nullptr;
  for (int i = 0; i < 2; ++i) {
    m_histoLayerVsSector[i] = nullptr;
  }
}

KLMDQMModule::~KLMDQMModule()
{
  if (m_StripLayer != nullptr)
    delete m_StripLayer;
}

void KLMDQMModule::defineHistoEKLM()
{
  int i;
  /* cppcheck-suppress variableScope */
  int endcap, layer, detectorLayer, stripMin, stripMax;
  int maxLayerGlobal, maxSector, maxPlane, maxStrip;
  std::string str, str2;
  TDirectory* oldDirectory, *newDirectory;
  oldDirectory = gDirectory;
  newDirectory = oldDirectory->mkdir(m_HistogramDirectoryNameEKLM.c_str());
  newDirectory->cd();
  m_Sector = new TH1F("sector", "Sector number", 104, 0.5, 104.5);
  m_Sector->SetOption("LIVE");
  maxLayerGlobal = m_Elements->getMaximalLayerGlobalNumber();
  maxSector = m_Elements->getMaximalSectorNumber();
  maxPlane = m_Elements->getMaximalPlaneNumber();
  maxStrip = m_Elements->getMaximalStripNumber();
  m_StripLayer = new TH1F*[maxLayerGlobal];
  for (i = 0; i < maxLayerGlobal; i++) {
    detectorLayer = i + 1;
    str = "strip_layer_" + std::to_string(detectorLayer);
    str2 = "Strip number (layer " + std::to_string(detectorLayer) + ")";
    m_Elements->layerNumberToElementNumbers(detectorLayer, &endcap, &layer);
    stripMin = m_Elements->stripNumber(endcap, layer, 1, 1, 1);
    stripMax = m_Elements->stripNumber(endcap, layer,
                                       maxSector, maxPlane, maxStrip);
    m_StripLayer[i] = new TH1F(str.c_str(), str2.c_str(),
                               stripMax - stripMin + 1,
                               stripMin - 0.5, stripMax + 0.5);
    m_StripLayer[i]->SetOption("LIVE");
  }
  oldDirectory->cd();
}

void KLMDQMModule::defineHistoBKLM()
{

  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_HistogramDirectoryNameBKLM.c_str())->cd();

  m_layerHits = new TH1F("layer hits", "layer hits",
                         40, 0, 15);
  m_eDep = new TH1F("eDep", "Reconstructed pulse height",
                    25, 0, 25);
  m_eDep->GetXaxis()->SetTitle("pulse height [MeV]");
  m_eDep->SetOption("LIVE");
  m_nPixel = new TH1F("nPixel", "Reconstructed number of MPPC pixels",
                      500, 0, 500);
  m_nPixel->GetXaxis()->SetTitle("Reconstructed number of MPPC pixels");
  m_zStrips = new TH1F("zStrips", "z-measuring strip numbers of the 2D hit",
                       54, 0, 54);
  m_zStrips->GetXaxis()->SetTitle("z-measuring strip numbers of the 2D hit");
  m_zStrips->SetOption("LIVE");
  m_phiStrip = new TH1F("phiStrip", "Phi strip number of muon hit",
                        50, -0.5, 49.5);
  m_phiStrip->GetXaxis()->SetTitle("Phi strip number of muon hit");
  m_phiStrip->SetOption("LIVE");
  m_sector = new TH1F("sector", "Sector number of muon hit",
                      10, -0.5, 9.5);
  m_sector->GetXaxis()->SetTitle("Sector number of muon hit");
  m_sector->SetOption("LIVE");
  m_layer = new TH1F("layer", "Layer number of muon hit",
                     16, -0.5, 15.5);
  m_layer->GetXaxis()->SetTitle("Layer number of muon hit");
  m_layer->SetOption("LIVE");
  m_rBKLMHit2ds = new TH1F("rBKLMHit2ds", "Distance from z axis in transverse plane of muon hit",
                           30, 200.0, 350.0);
  m_rBKLMHit2ds->GetXaxis()->SetTitle("Distance from z axis in transverse plane of muon hit");
  m_rBKLMHit2ds->SetOption("LIVE");
  m_zBKLMHit2ds = new TH1F("zBKLMHit2ds", "Axial position of muon hit",
                           100, -200.0, 300.0);
  m_zBKLMHit2ds->GetXaxis()->SetTitle("Axial position of muon hit");
  m_zBKLMHit2ds->SetOption("LIVE");
  m_yvsxBKLMHit2ds = new TH2F("yvsx", "Position projected into y-x plane of muon hit",
                              140, -350.0, 350.0, 140, -350.0, 350.0);
  m_yvsxBKLMHit2ds->GetXaxis()->SetTitle("x(cm)");
  m_yvsxBKLMHit2ds->GetYaxis()->SetTitle("y(cm)");
  m_yvsxBKLMHit2ds->SetOption("LIVE");
  m_xvszBKLMHit2ds = new TH2F("xvsz", "Position projected into x-z plane of muon hit",
                              140, -300.0, 400.0, 140, -350.0, 350.0);
  m_xvszBKLMHit2ds->GetXaxis()->SetTitle("z(cm)");
  m_xvszBKLMHit2ds->GetYaxis()->SetTitle("x(cm)");
  m_xvszBKLMHit2ds->SetOption("LIVE");
  m_yvszBKLMHit2ds = new TH2F("yvsz", "Position projected into y-z plane of muon hit",
                              140, -300.0, 400.0, 140, -350.0, 350.0);
  m_yvszBKLMHit2ds->GetXaxis()->SetTitle("z(cm)");
  m_yvszBKLMHit2ds->GetYaxis()->SetTitle("y(cm)");
  m_yvszBKLMHit2ds->SetOption("LIVE");

  m_histoLayerVsSector[0]  = new TH2F("BackwardLayerVSSector", "Sector VS Layer in barrel forward",
                                      30, -0.25, 14.75, 16, -0.25, 7.75);
  m_histoLayerVsSector[0]->GetXaxis()->SetTitle("Layer");
  m_histoLayerVsSector[0]->GetYaxis()->SetTitle("Sector");
  m_histoLayerVsSector[0]->SetStats(0);
  m_histoLayerVsSector[0]->SetOption("colz");

  m_histoLayerVsSector[1]  = new TH2F("ForwardLayerVSSector", "Sector VS Layer in barrel backward",
                                      30, -0.25, 14.75, 16, -0.25, 7.75);
  m_histoLayerVsSector[1]->GetXaxis()->SetTitle("Layer");
  m_histoLayerVsSector[1]->GetYaxis()->SetTitle("Sector");
  m_histoLayerVsSector[1]->SetStats(0);
  m_histoLayerVsSector[1]->SetOption("colz");

  oldDir->cd();
}

void KLMDQMModule::defineHisto()
{
  TDirectory* oldDirectory, *newDirectory;
  oldDirectory = gDirectory;
  newDirectory = oldDirectory->mkdir(m_HistogramDirectoryName.c_str());
  newDirectory->cd();
  /* Time histograms. */
  m_TimeRPC = new TH1F("time_rpc", "RPC hit time", 128, -1023.5, 0.5);
  m_TimeRPC->GetXaxis()->SetTitle("Time, ns");
  m_TimeRPC->SetOption("LIVE");
  m_TimeScintillatorBKLM =
    new TH1F("time_scintillator_bklm", "Scintillator hit time (BKLM)",
             100, -5000, -4000);
  m_TimeScintillatorBKLM->GetXaxis()->SetTitle("Time, ns");
  m_TimeScintillatorBKLM->SetOption("LIVE");
  m_TimeScintillatorEKLM =
    new TH1F("time_scintillator_eklm", "Scintillator hit time (EKLM)",
             100, -5000, -4000);
  m_TimeScintillatorEKLM->GetXaxis()->SetTitle("Time, ns");
  m_TimeScintillatorEKLM->SetOption("LIVE");
  oldDirectory->cd();
  /* EKLM histograms. */
  defineHistoEKLM();
  /* BKLM histograms. */
  defineHistoBKLM();
}

void KLMDQMModule::initialize()
{
  REG_HISTOGRAM
  m_Digits.isRequired();
  StoreArray<BKLMDigit> digits(m_outputDigitsName);
  digits.isRequired();
}

void KLMDQMModule::beginRun()
{
  int i, n;
  /* Common histograms. */
  m_TimeRPC->Reset();
  m_TimeScintillatorBKLM->Reset();
  m_TimeScintillatorEKLM->Reset();
  /* EKLM. */
  m_Sector->Reset();
  n = m_Elements->getMaximalLayerGlobalNumber();
  for (i = 0; i < n; i++)
    m_StripLayer[i]->Reset();
  /* BKLN. */
  m_layerHits->Reset();
  m_eDep->Reset();
  m_nPixel->Reset();
  m_zStrips->Reset();
  m_phiStrip->Reset();
  m_sector->Reset();
  m_layer->Reset();
  m_rBKLMHit2ds->Reset();
  m_zBKLMHit2ds->Reset();
  m_yvsxBKLMHit2ds->Reset();
  m_xvszBKLMHit2ds->Reset();
  m_yvszBKLMHit2ds->Reset();
  m_histoLayerVsSector[0]->Reset();
  m_histoLayerVsSector[1]->Reset();
}

void KLMDQMModule::event()
{
  int i, n;
  int endcap, layer, sector, plane, strip;
  int detectorLayer, sectorGlobal, stripGlobal;
  EKLMDigit* eklmDigit;
  n = m_Digits.getEntries();
  /* EKLM. */
  for (i = 0; i < n; i++) {
    eklmDigit = m_Digits[i];
    /*
     * Reject digits that are below the threshold (such digits may appear
     * for simulated events).
     */
    if (!eklmDigit->isGood())
      continue;
    endcap = eklmDigit->getEndcap();
    layer = eklmDigit->getLayer();
    sector = eklmDigit->getSector();
    plane = eklmDigit->getPlane();
    strip = eklmDigit->getStrip();
    detectorLayer = m_Elements->detectorLayerNumber(endcap, layer);
    sectorGlobal = m_Elements->sectorNumber(endcap, layer, sector);
    stripGlobal = m_Elements->stripNumber(endcap, layer, sector, plane, strip);
    m_Sector->Fill(sectorGlobal);
    m_StripLayer[detectorLayer - 1]->Fill(stripGlobal);
    m_TimeScintillatorEKLM->Fill(eklmDigit->getTime());
  }
  /* BKLM. */
  StoreArray<BKLMDigit> digits(m_outputDigitsName);
  int nent = digits.getEntries();
  for (i = 0; i < nent; i++) {
    BKLMDigit* digit = static_cast<BKLMDigit*>(digits[i]);
    m_layerHits->Fill(digit->getModuleID());
    if (digit->inRPC())
      m_TimeRPC->Fill(digit->getTime());
    else
      m_TimeScintillatorBKLM->Fill(digit->getTime());
    m_eDep->Fill(digit->getEDep());
    m_nPixel->Fill(digit->getNPixel());
    if (digit->isForward())
      m_histoLayerVsSector[0]->Fill(digit->getLayer() - 1, digit->getSector() - 1);
    else
      m_histoLayerVsSector[1]->Fill(digit->getLayer() - 1, digit->getSector() - 1);
  }
  StoreArray<BKLMHit2d> hits(m_outputHitsName);
  int nnent = hits.getEntries();
  for (i = 0; i < nnent; i++) {
    BKLMHit2d* hit = static_cast<BKLMHit2d*>(hits[i]);
    m_zStrips->Fill(hit->getZStripAve());
    m_phiStrip->Fill(hit->getPhiStripAve());
    m_sector->Fill(hit->getSector());
    m_layer->Fill(hit->getLayer());
    m_rBKLMHit2ds->Fill(hit->getGlobalPosition().Perp());
    m_zBKLMHit2ds->Fill(hit->getGlobalPosition().Z());
    m_yvsxBKLMHit2ds->Fill(hit->getGlobalPosition().Y(), hit->getGlobalPosition().X());
    m_xvszBKLMHit2ds->Fill(hit->getGlobalPosition().X(), hit->getGlobalPosition().Z());
    m_yvszBKLMHit2ds->Fill(hit->getGlobalPosition().Y(), hit->getGlobalPosition().Z());
  }
}

void KLMDQMModule::endRun()
{
}

void KLMDQMModule::terminate()
{
}
