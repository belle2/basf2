/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin, Vipin Gaur, Leo Piilonen                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMDQM/KLMDQMModule.h>

/* KLM headers. */
#include <klm/dataobjects/KLMChannelIndex.h>

/* ROOT headers. */
#include <TDirectory.h>

using namespace Belle2;

REG_MODULE(KLMDQM)

KLMDQMModule::KLMDQMModule() :
  HistoModule(),
  m_DAQInclusion{nullptr},
  m_TimeRPC{nullptr},
  m_TimeScintillatorBKLM{nullptr},
  m_TimeScintillatorEKLM{nullptr},
  m_PlaneBKLMPhi{nullptr},
  m_PlaneBKLMZ{nullptr},
  m_PlaneEKLM{nullptr},
  m_MaskedChannelsPerSector{nullptr},
  m_bklmHit2dsZ{nullptr},
  m_BklmDigitsNumber{nullptr},
  m_KlmDigitsNumber{nullptr},
  m_KlmDigitsAfterLERInj{nullptr},
  m_TriggersLERInj{nullptr},
  m_KlmDigitsAfterHERInj{nullptr},
  m_TriggersHERInj{nullptr},
  m_ChannelArrayIndex{&(KLMChannelArrayIndex::Instance())},
  m_SectorArrayIndex{&(KLMSectorArrayIndex::Instance())},
  m_ElementNumbers{&(KLMElementNumbers::Instance())},
  m_eklmElementNumbers{&(EKLMElementNumbers::Instance())}
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

void KLMDQMModule::defineHisto()
{
  TDirectory* oldDirectory, *newDirectory;
  oldDirectory = gDirectory;
  newDirectory = oldDirectory->mkdir(m_HistogramDirectoryName.c_str());
  newDirectory->cd();
  /* DAQ inclusion. */
  m_DAQInclusion = new TH1F("daq_inclusion", "Is KLM included in DAQ?", 2, 0.0, 2.0);
  m_DAQInclusion->GetXaxis()->SetBinLabel(1, "No");
  m_DAQInclusion->GetXaxis()->SetBinLabel(2, "Yes");
  m_DAQInclusion->SetOption("LIVE");
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
  /* Number of hits per plane. */
  m_PlaneBKLMPhi = new TH1F("plane_bklm_phi",
                            "BKLM plane occupancy (#phi readout)",
                            240, 0.5, 240.5);
  m_PlaneBKLMPhi->GetXaxis()->SetTitle("Layer number");
  m_PlaneBKLMPhi->SetOption("LIVE");
  m_PlaneBKLMZ = new TH1F("plane_bklm_z",
                          "BKLM plane occupancy (z readout)",
                          240, 0.5, 240.5);
  m_PlaneBKLMZ->GetXaxis()->SetTitle("Layer number");
  m_PlaneBKLMZ->SetOption("LIVE");
  m_PlaneEKLM = new TH1F("plane_eklm", "EKLM plane occupancy (both readouts)", 208, 0.5, 208.5);
  m_PlaneEKLM->GetXaxis()->SetTitle("Plane number");
  m_PlaneEKLM->SetOption("LIVE");
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
  firstChannelNumbers[nChannelHistograms] = m_ChannelArrayIndex->getNElements();
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
      std::string title = "Sector " + std::to_string(klmSector.getSector()) + " -- " +
                          m_ElementNumbers->getSectorDAQName(klmSector.getSubdetector(), klmSector.getSection(), klmSector.getSector());
      m_ChannelHits[sectorIndex][j] = new TH1F(
        name.c_str(), title.c_str(),
        firstChannelNumbers[i + 1] - firstChannelNumbers[i],
        firstChannelNumbers[i] - 0.5, firstChannelNumbers[i + 1] - 0.5);
      m_ChannelHits[sectorIndex][j]->SetOption("LIVE");
      i++;
    }
  }
  delete[] firstChannelNumbers;
  /* Masked channels per sector:
   * it is defined here, but filled by the analysis module. */
  uint16_t totalSectors = m_SectorArrayIndex->getNElements();
  m_MaskedChannelsPerSector = new TH1F("masked_channels", "Number of masked channels per sector",
                                       totalSectors, -0.5, totalSectors - 0.5);
  for (KLMChannelIndex& klmSector : klmSectors) {
    std::string label = m_ElementNumbers->getSectorDAQName(klmSector.getSubdetector(), klmSector.getSection(), klmSector.getSector());
    uint16_t sector = klmSector.getKLMSectorNumber();
    uint16_t sectorIndex = m_SectorArrayIndex->getIndex(sector);
    m_MaskedChannelsPerSector->GetXaxis()->SetBinLabel(sectorIndex + 1, label.c_str());
  }
  m_MaskedChannelsPerSector->SetOption("LIVE");
  /* Number of digits. */
  m_BklmDigitsNumber = new TH1F("bklm_digits", "Number of BKLM Digits",
                                250.0, 0.0, 250.0);
  m_BklmDigitsNumber->GetXaxis()->SetTitle("Number of BKLM Digits");
  m_BklmDigitsNumber->SetOption("LIVE");
  m_KlmDigitsNumber = new TH1F("klm_digits", "Number of KLM Digits",
                               250.0, 0.0, 250.0);
  m_KlmDigitsNumber->GetXaxis()->SetTitle("Number of KLM Digits");
  m_KlmDigitsNumber->SetOption("LIVE");
  /* BKLM 2d hits. */
  m_bklmHit2dsZ = new TH1F("zBKLMHit2ds", "Axial position of muon hit",
                           97, -172.22, 266.22);
  m_bklmHit2dsZ->GetXaxis()->SetTitle("Axial position of muon hit");
  m_bklmHit2dsZ->SetOption("LIVE");
  /* Number of digits after injection */
  /* For the histograms below, we use the same style as for other subdetectors. */
  m_KlmDigitsAfterLERInj = new TH1F("KLMOccInjLER", "KLMOccInjLER / Time;Time in #mus;KLM Digits / Time (5 #mus bins)",
                                    4000, 0, 20000);
  m_KlmDigitsAfterLERInj->SetOption("LIVE");
  m_TriggersLERInj = new TH1F("KLMOEccInjLER", "KLMEOccInjLER / Time;Time in #mus;Triggers / Time (5 #mus bins)",
                              4000, 0, 20000);
  m_TriggersLERInj->SetOption("LIVE");
  m_KlmDigitsAfterHERInj = new TH1F("KLMOccInjHER", "KLMOccInjHER / Time;Time in #mus;KLM Digits / Time (5 #mus bins)",
                                    4000, 0, 20000);
  m_KlmDigitsAfterHERInj->SetOption("LIVE");
  m_TriggersHERInj = new TH1F("KLMEOccInjHER", "KLMEOccInjHER / Time;Time in #mus;Triggers / Time (5 #mus bins)",
                              4000, 0, 20000);
  m_TriggersHERInj->SetOption("LIVE");
  oldDirectory->cd();
}

void KLMDQMModule::initialize()
{
  REG_HISTOGRAM;
  m_RawFtsws.isOptional();
  m_RawKlms.isOptional();
  m_Digits.isOptional();
  m_BklmHit1ds.isOptional();
  m_BklmHit2ds.isOptional();
}

void KLMDQMModule::beginRun()
{
  /* DAQ inclusion. */
  m_DAQInclusion->Reset();
  if (m_RawKlms.isValid())
    m_DAQInclusion->Fill("Yes", 1);
  else
    m_DAQInclusion->Fill("No", 1);
  /* Time. */
  m_TimeRPC->Reset();
  m_TimeScintillatorBKLM->Reset();
  m_TimeScintillatorEKLM->Reset();
  /* Plane hits. */
  m_PlaneEKLM->Reset();
  m_PlaneBKLMPhi->Reset();
  m_PlaneBKLMZ->Reset();
  /* Channel hits. */
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
  /* Digits. */
  m_KlmDigitsNumber->Reset();
  m_BklmDigitsNumber->Reset();
  /* BKLM 2d hits. */
  m_bklmHit2dsZ->Reset();
  /* Injection information. */
  m_KlmDigitsAfterLERInj->Reset();
  m_TriggersLERInj->Reset();
  m_KlmDigitsAfterHERInj->Reset();
  m_TriggersHERInj->Reset();
}

void KLMDQMModule::event()
{
  int i, nDigits, nEklmDigits = 0;
  KLMDigit* eklmDigit;
  nDigits = m_Digits.getEntries();
  /* EKLM. */
  for (i = 0; i < nDigits; i++) {
    eklmDigit = m_Digits[i];
    if (eklmDigit->getSubdetector() != KLMElementNumbers::c_EKLM)
      continue;
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
    int planeGlobal = m_eklmElementNumbers->planeNumber(
                        section, layer, sector, plane);
    m_PlaneEKLM->Fill(planeGlobal);
    m_TimeScintillatorEKLM->Fill(eklmDigit->getTime());
    nEklmDigits++;
  }
  /* BKLM. */
  m_BklmDigitsNumber->Fill((double)nDigits - nEklmDigits);
  m_KlmDigitsNumber->Fill((double)nDigits);
  for (i = 0; i < nDigits; i++) {
    KLMDigit* digit = m_Digits[i];
    if (digit->getSubdetector() != KLMElementNumbers::c_BKLM)
      continue;
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
  int nBklmHits2d = m_BklmHit2ds.getEntries();
  for (i = 0; i < nBklmHits2d; i++) {
    BKLMHit2d* hit2d = static_cast<BKLMHit2d*>(m_BklmHit2ds[i]);
    TVector3 hitPosition = hit2d->getGlobalPosition();
    m_bklmHit2dsZ->Fill(hitPosition.Z());
  }
  int nBklmHits1d = m_BklmHit1ds.getEntries();
  for (i = 0; i < nBklmHits1d; i++) {
    BKLMHit1d* hit1d = static_cast<BKLMHit1d*>(m_BklmHit1ds[i]);
    int section = hit1d->getSection();
    int sector = hit1d->getSector();
    int layer = hit1d->getLayer();
    int layerGlobal = BKLMElementNumbers::layerGlobalNumber(
                        section, sector, layer);
    if (hit1d->isPhiReadout())
      m_PlaneBKLMPhi->Fill(layerGlobal);
    else
      m_PlaneBKLMZ->Fill(layerGlobal);
  }
  /* Injection information. */
  for (RawFTSW& rawFtsw : m_RawFtsws) {
    unsigned int difference = rawFtsw.GetTimeSinceLastInjection(0);
    if (difference != 0x7FFFFFFF) {
      /* 127 MHz clock ticks to us, inexact rounding. */
      float differenceInUs = difference / 127.;
      if (rawFtsw.GetIsHER(0)) {
        m_KlmDigitsAfterHERInj->Fill(differenceInUs, nDigits);
        m_TriggersHERInj->Fill(differenceInUs);
      } else {
        m_KlmDigitsAfterLERInj->Fill(differenceInUs, nDigits);
        m_TriggersLERInj->Fill(differenceInUs);
      }
    }
  }
}

void KLMDQMModule::endRun()
{
}

void KLMDQMModule::terminate()
{
}
