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
#include <klm/dataobjects/KLMChannelIndex.h>
#include <klm/modules/KLMDQM/KLMDQMModule.h>

using namespace Belle2;

REG_MODULE(KLMDQM)

KLMDQMModule::KLMDQMModule() :
  HistoModule(),
  m_TimeRPC(nullptr),
  m_TimeScintillatorBKLM(nullptr),
  m_TimeScintillatorEKLM(nullptr),
  m_eklmSector(nullptr),
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
  m_ChannelArrayIndex = &(KLMChannelArrayIndex::Instance());
  m_SectorArrayIndex = &(KLMSectorArrayIndex::Instance());
  m_ElementNumbers = &(KLMElementNumbers::Instance());
  m_Elements = &(EKLM::ElementNumbersSingleton::Instance());
}

KLMDQMModule::~KLMDQMModule()
{
  KLMChannelIndex klmSectors(KLMChannelIndex::c_IndexLevelSector);
  for (KLMChannelIndex& klmSector : klmSectors) {
    uint16_t sector = klmSector.getKLMSectorNumber();
    uint16_t sectorIndex = m_SectorArrayIndex->getIndex(sector);
    if (m_ChannelHits[sectorIndex] != nullptr)
      delete[] m_ChannelHits[sectorIndex];
  }
}

void KLMDQMModule::defineHistoEKLM()
{
  TDirectory* oldDirectory, *newDirectory;
  oldDirectory = gDirectory;
  newDirectory = oldDirectory->mkdir(m_HistogramDirectoryNameEKLM.c_str());
  newDirectory->cd();
  m_eklmSector = new TH1F("sector", "Sector number", 104, 0.5, 104.5);
  m_eklmSector->SetOption("LIVE");
  oldDirectory->cd();
}

void KLMDQMModule::defineHistoBKLM()
{

  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_HistogramDirectoryNameBKLM.c_str())->cd();


  m_bklmSectorLayerPhi = new TH1F("SectorLayerPhi", "Sector and layer number occupancy for phi-readout hits",
                                  240, 0.0, 239.0);
  m_bklmSectorLayerPhi->GetXaxis()->SetTitle("sector*15 + layer (0..120 = backward, 120..240 = section)");
  m_bklmSectorLayerPhi->SetOption("LIVE");
  m_bklmSectorLayerZ = new TH1F("SectorLayerZ", "Sector and layer number occupancy for Z-readout hits",
                                240, 0.0, 239.0);
  m_bklmSectorLayerZ->GetXaxis()->SetTitle("sector*15 + layer (0..120 = backward, 120..240 = section)");
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
  /* Number of hits per channel. */
  int nChannelHistograms =
    BKLMElementNumbers::getMaximalSectorGlobalNumber() *
    m_ChannelHitHistogramsBKLM +
    EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder() *
    m_ChannelHitHistogramsEKLM;
  uint16_t* firstChannelNumbers = new uint16_t[nChannelHistograms + 1];
  int i = 0;
  KLMChannelIndex klmSectors(KLMChannelIndex::c_IndexLevelSector);
  for (KLMChannelIndex& klmSector : klmSectors) {
    KLMChannelIndex klmChannel(klmSector);
    klmChannel.setIndexLevel(KLMChannelIndex::c_IndexLevelStrip);
    uint16_t channel = klmChannel.getKLMChannelNumber();
    firstChannelNumbers[i] = m_ChannelArrayIndex->getIndex(channel);
    if (klmSector.getSubdetector() == KLMElementNumbers::c_BKLM) {
      channel = m_ElementNumbers->channelNumberBKLM(
                  klmChannel.getSection(), klmChannel.getSector(), 8, 0, 1);
      firstChannelNumbers[i + 1] = m_ChannelArrayIndex->getIndex(channel);
      i += 2;
    } else {
      int layerIncrease = (klmSector.getSection() == 1) ? 4 : 5;
      channel = m_ElementNumbers->channelNumberEKLM(
                  klmChannel.getSection(), klmChannel.getSector(),
                  1 + layerIncrease, 1, 1);
      firstChannelNumbers[i + 1] = m_ChannelArrayIndex->getIndex(channel);
      channel = m_ElementNumbers->channelNumberEKLM(
                  klmChannel.getSection(), klmChannel.getSector(),
                  1 + layerIncrease * 2, 1, 1);
      firstChannelNumbers[i + 2] = m_ChannelArrayIndex->getIndex(channel);
      i += 3;
    }
  }
  firstChannelNumbers[nChannelHistograms] = m_ChannelArrayIndex->getNChannels();
  i = 0;
  for (KLMChannelIndex& klmSector : klmSectors) {
    int nHistograms;
    if (klmSector.getSubdetector() == KLMElementNumbers::c_BKLM)
      nHistograms = m_ChannelHitHistogramsBKLM;
    else
      nHistograms = m_ChannelHitHistogramsEKLM;
    uint16_t sector = klmSector.getKLMSectorNumber();
    uint16_t sectorIndex = m_SectorArrayIndex->getIndex(sector);
    m_ChannelHits[sectorIndex] = new TH1F*[nHistograms];
    for (int j = 0; j < nHistograms; j++) {
      std::string name =
        "strip_hits_subdetector_" + std::to_string(klmSector.getSubdetector()) +
        "_section_" + std::to_string(klmSector.getSection()) +
        "_sector_" + std::to_string(klmSector.getSector()) +
        "_" + std::to_string(j);
      std::string title = "Sector " + std::to_string(klmSector.getSector());
      m_ChannelHits[sectorIndex][j] = new TH1F(
        name.c_str(), title.c_str(),
        firstChannelNumbers[i + 1] - firstChannelNumbers[i],
        firstChannelNumbers[i] - 0.5, firstChannelNumbers[i + 1] - 0.5);
      m_ChannelHits[sectorIndex][j]->SetOption("LIVE");
      i++;
    }
  }
  delete[] firstChannelNumbers;
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
  /* Common histograms. */
  m_TimeRPC->Reset();
  m_TimeScintillatorBKLM->Reset();
  m_TimeScintillatorEKLM->Reset();
  KLMChannelIndex klmSectors(KLMChannelIndex::c_IndexLevelSector);
  for (KLMChannelIndex& klmSector : klmSectors) {
    int nHistograms;
    if (klmSector.getSubdetector() == KLMElementNumbers::c_BKLM)
      nHistograms = m_ChannelHitHistogramsBKLM;
    else
      nHistograms = m_ChannelHitHistogramsEKLM;
    uint16_t sector = klmSector.getKLMSectorNumber();
    uint16_t sectorIndex = m_SectorArrayIndex->getIndex(sector);
    for (int j = 0; j < nHistograms; j++)
      m_ChannelHits[sectorIndex][j]->Reset();
  }
  /* EKLM. */
  m_eklmSector->Reset();
  /* BKLM. */
  m_bklmHit2dsZ->Reset();
  m_bklmSectorLayerPhi->Reset();
  m_bklmSectorLayerZ->Reset();
  m_bklmDigitsN->Reset();
}

void KLMDQMModule::event()
{
  int i, n;
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
    int section = eklmDigit->getSection();
    int sector = eklmDigit->getSector();
    int layer = eklmDigit->getLayer();
    int plane = eklmDigit->getPlane();
    int strip = eklmDigit->getStrip();
    uint16_t klmSector = m_ElementNumbers->sectorNumberEKLM(section, sector);
    uint16_t klmSectorIndex = m_SectorArrayIndex->getIndex(klmSector);
    uint16_t channel = m_ElementNumbers->channelNumberEKLM(
                         section, sector, layer, plane, strip);
    uint16_t channelIndex = m_ChannelArrayIndex->getIndex(channel);
    for (int j = 0; j < m_ChannelHitHistogramsEKLM; j++) {
      double xMin = m_ChannelHits[klmSectorIndex][j]->GetXaxis()->GetXmin();
      double xMax = m_ChannelHits[klmSectorIndex][j]->GetXaxis()->GetXmax();
      if ((xMin > channelIndex) || (xMax < channelIndex))
        continue;
      m_ChannelHits[klmSectorIndex][j]->Fill(channelIndex);
    }
    int sectorGlobal = m_Elements->sectorNumber(section, layer, sector);
    m_eklmSector->Fill(sectorGlobal);
    m_TimeScintillatorEKLM->Fill(eklmDigit->getTime());
  }
  /* BKLM. */
  StoreArray<BKLMDigit> digits(m_inputDigitsName);
  int nent = digits.getEntries();
  m_bklmDigitsN->Fill((double)digits.getEntries());
  for (i = 0; i < nent; i++) {
    BKLMDigit* digit = static_cast<BKLMDigit*>(digits[i]);
    int section = digit->getSection();
    int sector = digit->getSector();
    int layer = digit->getLayer();
    int plane = digit->getPlane();
    int strip = digit->getStrip();
    uint16_t klmSector = m_ElementNumbers->sectorNumberBKLM(section, sector);
    uint16_t klmSectorIndex = m_SectorArrayIndex->getIndex(klmSector);
    uint16_t channel = m_ElementNumbers->channelNumberBKLM(
                         section, sector, layer, plane, strip);
    uint16_t channelIndex = m_ChannelArrayIndex->getIndex(channel);
    for (int j = 0; j < m_ChannelHitHistogramsBKLM; j++) {
      double xMin = m_ChannelHits[klmSectorIndex][j]->GetXaxis()->GetXmin();
      double xMax = m_ChannelHits[klmSectorIndex][j]->GetXaxis()->GetXmax();
      if ((xMin > channelIndex) || (xMax < channelIndex))
        continue;
      m_ChannelHits[klmSectorIndex][j]->Fill(channelIndex);
    }
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
      m_bklmSectorLayerPhi->Fill(hit1d->getSection() * 120 + (hit1d->getSector() - 1) * 15 + (hit1d->getLayer() - 1));
    } else {
      m_bklmSectorLayerZ->Fill(hit1d->getSection() * 120 + (hit1d->getSector() - 1) * 15 + (hit1d->getLayer() - 1));
    }
  }
}

void KLMDQMModule::endRun()
{
}

void KLMDQMModule::terminate()
{
}
