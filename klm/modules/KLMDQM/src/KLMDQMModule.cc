/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin, Vipin Gaur, Leo Piilonen                *
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
  m_bklmHit2dsZ(nullptr),
  m_bklmSectorLayerPhi(nullptr),
  m_bklmSectorLayerZ(nullptr),
  m_bklmDigitsN(nullptr)
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
  addParam("inputDigitsName", m_inputDigitsName,
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


  m_bklmSectorLayerPhi = new TH1F("SectorLayerPhi", "Sector and layer number occupancy for phi-readout hits",
                                  240, 0.0, 239.0);
  m_bklmSectorLayerPhi->GetXaxis()->SetTitle("sector*15 + layer (0..120 = backward, 120..240 = forward)");
  m_bklmSectorLayerPhi->SetOption("LIVE");
  m_bklmSectorLayerZ = new TH1F("SectorLayerZ", "Sector and layer number occupancy for Z-readout hits",
                                240, 0.0, 239.0);
  m_bklmSectorLayerZ->GetXaxis()->SetTitle("sector*15 + layer (0..120 = backward, 120..240 = forward)");
  m_bklmSectorLayerZ->SetOption("LIVE");
  m_bklmHit2dsZ = new TH1F("zBKLMHit2ds", "Axial position of muon hit",
                           97, -172.22, 266.22);
  m_bklmHit2dsZ->GetXaxis()->SetTitle("Axial position of muon hit");
  m_bklmHit2dsZ->SetOption("LIVE");
  m_bklmDigitsN = new TH1F("bklmDigitsN", "Number of BKLM Digits",
                           250.0, 0.0, 250.0);
  m_bklmDigitsN->GetXaxis()->SetTitle("Number of BKLM Digits");
  m_bklmDigitsN->SetOption("LIVE");
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
  StoreArray<BKLMDigit> digits(m_inputDigitsName);
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
  m_bklmHit2dsZ->Reset();
  m_bklmSectorLayerPhi->Reset();
  m_bklmSectorLayerZ->Reset();
  m_bklmDigitsN->Reset();
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
  StoreArray<BKLMDigit> digits(m_inputDigitsName);
  int nent = digits.getEntries();
  m_bklmDigitsN->Fill((double)digits.getEntries());
  for (i = 0; i < nent; i++) {
    BKLMDigit* digit = static_cast<BKLMDigit*>(digits[i]);
    if (digit->inRPC())
      m_TimeRPC->Fill(digit->getTime());
    else
      m_TimeScintillatorBKLM->Fill(digit->getTime());
  }
  StoreArray<BKLMHit2d> hits(m_inputHitsName2d);
  int nnent = hits.getEntries();
  for (i = 0; i < nnent; i++) {
    BKLMHit2d* hit = static_cast<BKLMHit2d*>(hits[i]);
    TVector3 hitPosition = hit->getGlobalPosition();
    m_bklmHit2dsZ->Fill(hitPosition.Z());
  }
  StoreArray<BKLMHit1d> hits1d(m_inputHitsName1d);
  int nent1d = hits1d.getEntries();
  for (i = 0; i < nent1d; i++) {
    BKLMHit1d* hit1d = static_cast<BKLMHit1d*>(hits1d[i]);
    if (hit1d->isPhiReadout()) {
      m_bklmSectorLayerPhi->Fill(hit1d->getForward() * 120 + (hit1d->getSector() - 1) * 15 + (hit1d->getLayer() - 1));
    } else {
      m_bklmSectorLayerZ->Fill(hit1d->getForward() * 120 + (hit1d->getSector() - 1) * 15 + (hit1d->getLayer() - 1));
    }
  }
}

void KLMDQMModule::endRun()
{
}

void KLMDQMModule::terminate()
{
}
