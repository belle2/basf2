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
#include <klm/dataobjects/KLMDigitRaw.h>

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
  m_DigitsKLM{nullptr},
  m_DigitsRPC{nullptr},
  m_DigitsScintillatorBKLM{nullptr},
  m_DigitsScintillatorEKLM{nullptr},
  m_DigitsMultiStripBKLM{nullptr},
  m_DigitsMultiStripEKLM{nullptr},
  m_TriggerBitsBKLM{nullptr},
  m_TriggerBitsEKLM{nullptr},
  m_DigitsAfterLERInj{nullptr},
  m_TriggersLERInj{nullptr},
  m_DigitsAfterHERInj{nullptr},
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
}

KLMDQMModule::~KLMDQMModule()
{
  KLMChannelIndex klmIndex(KLMChannelIndex::c_IndexLevelSector);
  for (KLMChannelIndex& klmSector : klmIndex) {
    uint16_t sector = klmSector.getKLMSectorNumber();
    uint16_t sectorIndex = m_SectorArrayIndex->getIndex(sector);
    if (m_ChannelHits[sectorIndex] != nullptr)
      delete[] m_ChannelHits[sectorIndex];
  }
  klmIndex.setIndexLevel(KLMChannelIndex::c_IndexLevelSection);
  for (KLMChannelIndex& klmSection : klmIndex) {
    uint16_t subdetector = klmSection.getSubdetector();
    if (subdetector == KLMElementNumbers::c_EKLM) {
      uint16_t section = klmSection.getSection();
      if (m_Spatial2DHitsEKLM[section - 1] != nullptr)
        delete[] m_Spatial2DHitsEKLM[section - 1];
    }
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
  KLMChannelIndex klmIndex(KLMChannelIndex::c_IndexLevelSector);
  for (KLMChannelIndex& klmSector : klmIndex) {
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
  klmIndex.setIndexLevel(KLMChannelIndex::c_IndexLevelSector);
  for (KLMChannelIndex& klmSector : klmIndex) {
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
  klmIndex.setIndexLevel(KLMChannelIndex::c_IndexLevelSector);
  for (KLMChannelIndex& klmSector : klmIndex) {
    std::string label = m_ElementNumbers->getSectorDAQName(klmSector.getSubdetector(), klmSector.getSection(), klmSector.getSector());
    uint16_t sector = klmSector.getKLMSectorNumber();
    uint16_t sectorIndex = m_SectorArrayIndex->getIndex(sector);
    m_MaskedChannelsPerSector->GetXaxis()->SetBinLabel(sectorIndex + 1, label.c_str());
  }
  m_MaskedChannelsPerSector->SetOption("LIVE");
  /* Number of digits. */
  m_DigitsKLM = new TH1F("digits_klm", "Number of KLM digits",
                         250.0, 0.0, 250.0);
  m_DigitsKLM->GetXaxis()->SetTitle("Number of digits");
  m_DigitsKLM->SetOption("LIVE");
  m_DigitsRPC = new TH1F("digits_rpc", "Number of RPC digits",
                         250.0, 0.0, 250.0);
  m_DigitsRPC->GetXaxis()->SetTitle("Number of digits");
  m_DigitsRPC->SetOption("LIVE");
  m_DigitsScintillatorBKLM = new TH1F("digits_scintillator_bklm", "Number of scintillator digits (BKLM)",
                                      250.0, 0.0, 250.0);
  m_DigitsScintillatorBKLM->GetXaxis()->SetTitle("Number of digits");
  m_DigitsScintillatorBKLM->SetOption("LIVE");
  m_DigitsScintillatorEKLM = new TH1F("digits_scintillator_eklm", "Number of scintillator digits (EKLM)",
                                      250.0, 0.0, 250.0);
  m_DigitsScintillatorEKLM->GetXaxis()->SetTitle("Number of digits");
  m_DigitsScintillatorEKLM->SetOption("LIVE");
  m_DigitsMultiStripBKLM = new TH1F("digits_multi_bklm", "Number of multi-strip digits (BKLM)",
                                    50.0, 0.0, 50.0);
  m_DigitsMultiStripBKLM->GetXaxis()->SetTitle("Number of multi-strip digits");
  m_DigitsMultiStripBKLM->SetOption("LIVE");
  m_DigitsMultiStripEKLM = new TH1F("digits_multi_eklm", "Number of multi-strip digits (EKLM)",
                                    50.0, 0.0, 50.0);
  m_DigitsMultiStripEKLM->GetXaxis()->SetTitle("Number of multi-strip digits");
  m_DigitsMultiStripEKLM->SetOption("LIVE");
  /* Trigger bits. */
  m_TriggerBitsBKLM = new TH1F("trigger_bits_bklm", "Trigger bits of multi-strip digits (BKLM)",
                               (double)c_0x1, (double)c_0x8, (double)c_0x1 + 1.0);
  m_TriggerBitsBKLM->GetXaxis()->SetBinLabel(c_0x8, "0x8");
  m_TriggerBitsBKLM->GetXaxis()->SetBinLabel(c_0x4, "0x4");
  m_TriggerBitsBKLM->GetXaxis()->SetBinLabel(c_0x2, "0x2");
  m_TriggerBitsBKLM->GetXaxis()->SetBinLabel(c_0x1, "0x1");
  m_TriggerBitsBKLM->SetOption("LIVE");
  m_TriggerBitsEKLM = new TH1F("trigger_bits_eklm", "Trigger bits of multi-strip digits (EKLM)",
                               (double)c_0x1, (double)c_0x8, (double)c_0x1 + 1.0);
  m_TriggerBitsEKLM->GetXaxis()->SetBinLabel(c_0x8, "0x8");
  m_TriggerBitsEKLM->GetXaxis()->SetBinLabel(c_0x4, "0x4");
  m_TriggerBitsEKLM->GetXaxis()->SetBinLabel(c_0x2, "0x2");
  m_TriggerBitsEKLM->GetXaxis()->SetBinLabel(c_0x1, "0x1");
  m_TriggerBitsEKLM->SetOption("LIVE");
  /* Number of digits after injection */
  /* For the histograms below, we use the same style as for other subdetectors. */
  m_DigitsAfterLERInj = new TH1F("KLMOccInjLER", "KLM digits after LER injection / Time;Time [#mus];Number of KLM digits / (5 #mus)",
                                 4000, 0, 20000);
  m_DigitsAfterLERInj->SetOption("LIVE");
  m_TriggersLERInj = new TH1F("KLMTrigInjLER", "Triggers after KER injection / Time;Time [#mus];Number of triggers / (5 #mus)",
                              4000, 0, 20000);
  m_TriggersLERInj->SetOption("LIVE");
  m_DigitsAfterHERInj = new TH1F("KLMOccInjHER", "KLM digits after HER injection / Time;Time [#mus];Number of KLM digits / (5 #mus)",
                                 4000, 0, 20000);
  m_DigitsAfterHERInj->SetOption("LIVE");
  m_TriggersHERInj = new TH1F("KLMTrigInjHER", "Triggers after HER injection / Time;Time [#mus];Number of triggers / (5 #mus)",
                              4000, 0, 20000);
  m_TriggersHERInj->SetOption("LIVE");
  /* Spatial distribution of EKLM 2d hits per layer. */
  klmIndex.setIndexLevel(KLMChannelIndex::c_IndexLevelSection);
  for (KLMChannelIndex& klmSection : klmIndex) {
    uint16_t subdetector = klmSection.getSubdetector();
    if (subdetector == KLMElementNumbers::c_EKLM) {
      uint16_t section = klmSection.getSection();
      int maximalLayerNumber = m_eklmElementNumbers->getMaximalDetectorLayerNumber(section);
      m_Spatial2DHitsEKLM[section - 1] = new TH2F*[maximalLayerNumber];
      std::string sectionName = (section == EKLMElementNumbers::c_ForwardSection) ? "Forward" : "Backward";
      for (int j = 1; j <= maximalLayerNumber; ++j) {
        std::string name = "spatial_2d_hits_subdetector_" + std::to_string(subdetector) +
                           "_section_" + std::to_string(section) +
                           "_layer_" + std::to_string(j);
        std::string title = "Endcap " + sectionName + " , Layer " + std::to_string(j);
        /* Use bins with a size of 10 cm per side. */
        m_Spatial2DHitsEKLM[section - 1][j - 1] = new TH2F(name.c_str(), title.c_str(),
                                                           340 * 2 / 10, -340, 340,
                                                           340 * 2 / 10, -340, 340);
        m_Spatial2DHitsEKLM[section - 1][j - 1]->GetXaxis()->SetTitle("X coordinate [cm]");
        m_Spatial2DHitsEKLM[section - 1][j - 1]->GetYaxis()->SetTitle("Y coordinate [cm]");
        m_Spatial2DHitsEKLM[section - 1][j - 1]->SetOption("LIVE");
      }
    }
  }
  oldDirectory->cd();
}

void KLMDQMModule::initialize()
{
  REG_HISTOGRAM;
  m_RawFtsws.isOptional();
  m_RawKlms.isOptional();
  m_Digits.isOptional();
  m_BklmHit1ds.isOptional();
  m_EklmHit2ds.isOptional();
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
  KLMChannelIndex klmIndex(KLMChannelIndex::c_IndexLevelSector);
  for (KLMChannelIndex& klmSector : klmIndex) {
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
  m_DigitsKLM->Reset();
  m_DigitsRPC->Reset();
  m_DigitsScintillatorBKLM->Reset();
  m_DigitsScintillatorEKLM->Reset();
  m_DigitsMultiStripBKLM->Reset();
  m_DigitsMultiStripEKLM->Reset();
  /* Trigger bits. */
  m_TriggerBitsBKLM->Reset();
  m_TriggerBitsEKLM->Reset();
  /* Injection information. */
  m_DigitsAfterLERInj->Reset();
  m_TriggersLERInj->Reset();
  m_DigitsAfterHERInj->Reset();
  m_TriggersHERInj->Reset();
  /* Spatial 2D hits distributions. */
  klmIndex.setIndexLevel(KLMChannelIndex::c_IndexLevelSection);
  for (KLMChannelIndex& klmSection : klmIndex) {
    uint16_t subdetector = klmSection.getSubdetector();
    if (subdetector == KLMElementNumbers::c_EKLM) {
      uint16_t section = klmSection.getSection();
      int maximalLayerNumber = m_eklmElementNumbers->getMaximalDetectorLayerNumber(section);
      for (int j = 1; j <= maximalLayerNumber; ++j)
        m_Spatial2DHitsEKLM[section - 1][j - 1]->Reset();
    }
  }
}

void KLMDQMModule::event()
{
  int nDigits = m_Digits.getEntries();
  int nDigitsRPC = 0, nDigitsScintillatorBKLM = 0, nDigitsScintillatorEKLM = 0;
  int nDigitsMultiStripBKLM = 0, nDigitsMultiStripEKLM = 0;
  for (const KLMDigit& digit : m_Digits) {
    /*
     * Reject digits that are below the threshold (such digits may appear
     * for simulated events).
     */
    if (!digit.isGood())
      continue;
    if (digit.getSubdetector() == KLMElementNumbers::c_EKLM) {
      nDigitsScintillatorEKLM++;
      int section = digit.getSection();
      int sector = digit.getSector();
      int layer = digit.getLayer();
      int plane = digit.getPlane();
      int strip = digit.getStrip();
      if (not digit.isMultiStrip()) {
        uint16_t klmSector = m_ElementNumbers->sectorNumberEKLM(section, sector);
        uint16_t klmSectorIndex = m_SectorArrayIndex->getIndex(klmSector);
        uint16_t channel = m_ElementNumbers->channelNumberEKLM(section, sector, layer, plane, strip);
        uint16_t channelIndex = m_ChannelArrayIndex->getIndex(channel);
        for (int j = 0; j < m_ChannelHitHistogramsEKLM; j++) {
          double xMin = m_ChannelHits[klmSectorIndex][j]->GetXaxis()->GetXmin();
          double xMax = m_ChannelHits[klmSectorIndex][j]->GetXaxis()->GetXmax();
          if ((xMin > channelIndex) || (xMax < channelIndex))
            continue;
          m_ChannelHits[klmSectorIndex][j]->Fill(channelIndex);
        }
      } else
        nDigitsMultiStripEKLM++;
      int planeGlobal = m_eklmElementNumbers->planeNumber(section, layer, sector, plane);
      m_PlaneEKLM->Fill(planeGlobal);
      m_TimeScintillatorEKLM->Fill(digit.getTime());
      if (digit.isMultiStrip()) {
        KLMDigitRaw* digitRaw = digit.getRelated<KLMDigitRaw>();
        if (digitRaw) {
          uint16_t triggerBits = digitRaw->getTriggerBits();
          if ((triggerBits & 0x1) != 0)
            m_TriggerBitsEKLM->Fill(c_0x1);
          if ((triggerBits & 0x2) != 0)
            m_TriggerBitsEKLM->Fill(c_0x2);
          if ((triggerBits & 0x4) != 0)
            m_TriggerBitsEKLM->Fill(c_0x4);
          if ((triggerBits & 0x8) != 0)
            m_TriggerBitsEKLM->Fill(c_0x8);
        }
      }
    } else if (digit.getSubdetector() == KLMElementNumbers::c_BKLM) {
      int section = digit.getSection();
      int sector = digit.getSector();
      int layer = digit.getLayer();
      int plane = digit.getPlane();
      int strip = digit.getStrip();
      if (not digit.isMultiStrip()) {
        uint16_t klmSector = m_ElementNumbers->sectorNumberBKLM(section, sector);
        uint16_t klmSectorIndex = m_SectorArrayIndex->getIndex(klmSector);
        uint16_t channel = m_ElementNumbers->channelNumberBKLM(section, sector, layer, plane, strip);
        uint16_t channelIndex = m_ChannelArrayIndex->getIndex(channel);
        for (int j = 0; j < m_ChannelHitHistogramsBKLM; j++) {
          double xMin = m_ChannelHits[klmSectorIndex][j]->GetXaxis()->GetXmin();
          double xMax = m_ChannelHits[klmSectorIndex][j]->GetXaxis()->GetXmax();
          if ((xMin > channelIndex) || (xMax < channelIndex))
            continue;
          m_ChannelHits[klmSectorIndex][j]->Fill(channelIndex);
        }
      } else
        nDigitsMultiStripBKLM++;
      if (digit.inRPC()) {
        nDigitsRPC++;
        m_TimeRPC->Fill(digit.getTime());
      } else {
        nDigitsScintillatorBKLM++;
        m_TimeScintillatorBKLM->Fill(digit.getTime());
      }
      if (digit.isMultiStrip()) {
        KLMDigitRaw* digitRaw = digit.getRelated<KLMDigitRaw>();
        if (digitRaw) {
          uint16_t triggerBits = digitRaw->getTriggerBits();
          if ((triggerBits & 0x1) != 0)
            m_TriggerBitsBKLM->Fill(c_0x1);
          if ((triggerBits & 0x2) != 0)
            m_TriggerBitsBKLM->Fill(c_0x2);
          if ((triggerBits & 0x4) != 0)
            m_TriggerBitsBKLM->Fill(c_0x4);
          if ((triggerBits & 0x8) != 0)
            m_TriggerBitsBKLM->Fill(c_0x8);
        }
      }
    } else
      B2FATAL("Not a BKLM or a EKLM digit, something went really wrong.");
  }
  for (const BKLMHit1d& hit1d : m_BklmHit1ds) {
    int section = hit1d.getSection();
    int sector = hit1d.getSector();
    int layer = hit1d.getLayer();
    int layerGlobal = BKLMElementNumbers::layerGlobalNumber(
                        section, sector, layer);
    if (hit1d.isPhiReadout())
      m_PlaneBKLMPhi->Fill(layerGlobal);
    else
      m_PlaneBKLMZ->Fill(layerGlobal);
  }
  /* Digits. */
  m_DigitsKLM->Fill((double)nDigits);
  m_DigitsRPC->Fill((double)nDigitsRPC);
  m_DigitsScintillatorBKLM->Fill((double)nDigitsScintillatorBKLM);
  m_DigitsScintillatorEKLM->Fill((double)nDigitsScintillatorEKLM);
  if (nDigitsMultiStripBKLM > 0)
    m_DigitsMultiStripBKLM->Fill((double)nDigitsMultiStripBKLM);
  if (nDigitsMultiStripEKLM > 0)
    m_DigitsMultiStripEKLM->Fill((double)nDigitsMultiStripEKLM);
  /* Injection information. */
  for (RawFTSW& rawFtsw : m_RawFtsws) {
    unsigned int difference = rawFtsw.GetTimeSinceLastInjection(0);
    if (difference != 0x7FFFFFFF) {
      /* 127 MHz clock ticks to us, inexact rounding. */
      float differenceInUs = difference / 127.;
      if (rawFtsw.GetIsHER(0)) {
        m_DigitsAfterHERInj->Fill(differenceInUs, nDigits);
        m_TriggersHERInj->Fill(differenceInUs);
      } else {
        m_DigitsAfterLERInj->Fill(differenceInUs, nDigits);
        m_TriggersLERInj->Fill(differenceInUs);
      }
    }
    /*
     * Usually, only one RawFTSW object is stored per event.
     * If there are more, ignore the others.
     */
    break;
  }
  /* Spatial 2D hits distributions. */
  for (const EKLMHit2d& hit2d : m_EklmHit2ds) {
    int section = hit2d.getSection();
    int layer = hit2d.getLayer();
    m_Spatial2DHitsEKLM[section - 1][layer - 1]->Fill(hit2d.getPositionX(), hit2d.getPositionY());
  }
}

void KLMDQMModule::endRun()
{
}

void KLMDQMModule::terminate()
{
}
