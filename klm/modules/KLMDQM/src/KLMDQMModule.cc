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
  m_eklmSector(nullptr),
  m_eklmStripLayer(nullptr),
  m_bklmLayerHits(nullptr),
  m_bklmCtime(nullptr),
  m_bklmEDep(nullptr),
  m_bklmNPixel(nullptr),
  m_bklmModuleID(nullptr),
  m_bklmZStrips(nullptr),
  m_bklmPhiStrip(nullptr),
  m_bklmSector(nullptr),
  m_bklmLayer(nullptr),
  m_bklmHit2dsR(nullptr),
  m_bklmHit2dsZ(nullptr),
  m_bklmHit2dsYvsx(nullptr),
  m_bklmHit2dsXvsz(nullptr),
  m_bklmHit2dsYvsz(nullptr)
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
}

KLMDQMModule::~KLMDQMModule()
{
  if (m_eklmStripLayer != nullptr)
    delete m_eklmStripLayer;
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
  m_eklmSector = new TH1F("sector", "Sector number", 104, 0.5, 104.5);
  m_eklmSector->SetOption("LIVE");
  maxLayerGlobal = m_Elements->getMaximalLayerGlobalNumber();
  maxSector = m_Elements->getMaximalSectorNumber();
  maxPlane = m_Elements->getMaximalPlaneNumber();
  maxStrip = m_Elements->getMaximalStripNumber();
  m_eklmStripLayer = new TH1F*[maxLayerGlobal];
  for (i = 0; i < maxLayerGlobal; i++) {
    detectorLayer = i + 1;
    str = "strip_layer_" + std::to_string(detectorLayer);
    str2 = "Strip number (layer " + std::to_string(detectorLayer) + ")";
    m_Elements->layerNumberToElementNumbers(detectorLayer, &endcap, &layer);
    stripMin = m_Elements->stripNumber(endcap, layer, 1, 1, 1);
    stripMax = m_Elements->stripNumber(endcap, layer,
                                       maxSector, maxPlane, maxStrip);
    m_eklmStripLayer[i] = new TH1F(str.c_str(), str2.c_str(),
                                   stripMax - stripMin + 1,
                                   stripMin - 0.5, stripMax + 0.5);
    m_eklmStripLayer[i]->SetOption("LIVE");
  }
  oldDirectory->cd();
}

void KLMDQMModule::defineHistoBKLM()
{

  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_HistogramDirectoryNameBKLM.c_str())->cd();
  m_bklmLayerHits = new TH1F("layer hits", "layer hits", 40, 0, 15);
  m_bklmCtime = new TH1F("ctime", "Lowest 16 bits of the B2TT CTime signal",
                         100, -2, 2);
  m_bklmCtime->GetXaxis()->SetTitle("ctime");
  m_bklmEDep = new TH1F("eDep", "Reconstructed pulse height",
                        25, 0, 25);
  m_bklmEDep->GetXaxis()->SetTitle("pulse height [MeV]");
  m_bklmEDep->SetOption("LIVE");
  m_bklmNPixel = new TH1F("nPixel", "Reconstructed number of MPPC pixels",
                          500, 0, 500);
  m_bklmNPixel->GetXaxis()->SetTitle("Reconstructed number of MPPC pixels");
  m_bklmModuleID = new TH1F("module ID", "detector-module identifier",
                            40, 0, 200000000);
  m_bklmModuleID->GetXaxis()->SetTitle("detector-module identifier");
  m_bklmZStrips = new TH1F("zStrips", "z-measuring strip numbers of the 2D hit",
                           54, 0, 54);
  m_bklmZStrips->GetXaxis()->SetTitle("z-measuring strip numbers of the 2D hit");
  m_bklmZStrips->SetOption("LIVE");
  m_bklmPhiStrip = new TH1F("phiStrip", "Phi strip number of muon hit",
                            50, -0.5, 49.5);
  m_bklmPhiStrip->GetXaxis()->SetTitle("Phi strip number of muon hit");
  m_bklmPhiStrip->SetOption("LIVE");
  m_bklmSector = new TH1F("sector", "Sector number of muon hit",
                          10, -0.5, 9.5);
  m_bklmSector->GetXaxis()->SetTitle("Sector number of muon hit");
  m_bklmSector->SetOption("LIVE");
  m_bklmLayer = new TH1F("layer", "Layer number of muon hit",
                         16, -0.5, 15.5);
  m_bklmLayer->GetXaxis()->SetTitle("Layer number of muon hit");
  m_bklmLayer->SetOption("LIVE");
  m_bklmHit2dsR = new TH1F("rBKLMHit2ds", "Distance from z axis in transverse plane of muon hit",
                           30, 200.0, 350.0);
  m_bklmHit2dsR->GetXaxis()->SetTitle("Distance from z axis in transverse plane of muon hit");
  m_bklmHit2dsR->SetOption("LIVE");
  m_bklmHit2dsZ = new TH1F("zBKLMHit2ds", "Axial position of muon hit",
                           100, -200.0, 300.0);
  m_bklmHit2dsZ->GetXaxis()->SetTitle("Axial position of muon hit");
  m_bklmHit2dsZ->SetOption("LIVE");
  m_bklmHit2dsYvsx = new TH2F("yvsx", "Position projected into y-x plane of muon hit",
                              140, -350.0, 350.0, 140, -350.0, 350.0);
  m_bklmHit2dsYvsx->GetXaxis()->SetTitle("x, cm");
  m_bklmHit2dsYvsx->GetYaxis()->SetTitle("y, cm");
  m_bklmHit2dsYvsx->SetOption("LIVE");
  m_bklmHit2dsXvsz = new TH2F("xvsz", "Position projected into x-z plane of muon hit",
                              140, -300.0, 400.0, 140, -350.0, 350.0);
  m_bklmHit2dsXvsz->GetXaxis()->SetTitle("z, cm");
  m_bklmHit2dsXvsz->GetYaxis()->SetTitle("x, cm");
  m_bklmHit2dsXvsz->SetOption("LIVE");
  m_bklmHit2dsYvsz = new TH2F("yvsz", "Position projected into y-z plane of muon hit",
                              140, -300.0, 400.0, 140, -350.0, 350.0);
  m_bklmHit2dsYvsz->GetXaxis()->SetTitle("z, cm");
  m_bklmHit2dsYvsz->GetYaxis()->SetTitle("y, cm");
  m_bklmHit2dsYvsz->SetOption("LIVE");

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
  m_eklmSector->Reset();
  n = m_Elements->getMaximalLayerGlobalNumber();
  for (i = 0; i < n; i++)
    m_eklmStripLayer[i]->Reset();
  /* BKLM. */
  m_bklmLayerHits->Reset();
  m_bklmCtime->Reset();
  m_bklmEDep->Reset();
  m_bklmNPixel->Reset();
  m_bklmModuleID->Reset();
  m_bklmZStrips->Reset();
  m_bklmPhiStrip->Reset();
  m_bklmSector->Reset();
  m_bklmLayer->Reset();
  m_bklmHit2dsR->Reset();
  m_bklmHit2dsZ->Reset();
  m_bklmHit2dsYvsx->Reset();
  m_bklmHit2dsXvsz->Reset();
  m_bklmHit2dsYvsz->Reset();
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
    m_eklmSector->Fill(sectorGlobal);
    m_eklmStripLayer[detectorLayer - 1]->Fill(stripGlobal);
    m_TimeScintillatorEKLM->Fill(eklmDigit->getTime());
  }
  /* BKLM. */
  StoreArray<BKLMDigit> digits(m_outputDigitsName);
  int nent = digits.getEntries();
  for (i = 0; i < nent; i++) {
    BKLMDigit* digit = static_cast<BKLMDigit*>(digits[i]);
    m_bklmLayerHits->Fill(digit->getModuleID());
    m_bklmCtime->Fill(digit->getCTime());
    if (digit->inRPC())
      m_TimeRPC->Fill(digit->getTime());
    else
      m_TimeScintillatorBKLM->Fill(digit->getTime());
    m_bklmEDep->Fill(digit->getEDep());
    m_bklmNPixel->Fill(digit->getNPixel());
  }
  StoreArray<BKLMHit2d> hits(m_outputHitsName);
  int nnent = hits.getEntries();
  for (i = 0; i < nnent; i++) {
    BKLMHit2d* hit = static_cast<BKLMHit2d*>(hits[i]);
    m_bklmModuleID->Fill(hit->getModuleID());
    m_bklmZStrips->Fill(hit->getZStripAve());
    m_bklmPhiStrip->Fill(hit->getPhiStripAve());
    m_bklmSector->Fill(hit->getSector());
    m_bklmLayer->Fill(hit->getLayer());
    TVector3 hitPosition = hit->getGlobalPosition();
    m_bklmHit2dsR->Fill(hitPosition.Perp());
    m_bklmHit2dsZ->Fill(hitPosition.Z());
    m_bklmHit2dsYvsx->Fill(hitPosition.Y(), hitPosition.X());
    m_bklmHit2dsXvsz->Fill(hitPosition.X(), hitPosition.Z());
    m_bklmHit2dsYvsz->Fill(hitPosition.Y(), hitPosition.Z());
  }
}

void KLMDQMModule::endRun()
{
}

void KLMDQMModule::terminate()
{
}
