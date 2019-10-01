/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin, Leo Piilonen, Vipin Gaur                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <TClass.h>
#include <TROOT.h>

/* Belle2 headers. */
#include <dqm/analysis/modules/DQMHistAnalysisKLM.h>
#include <klm/dataobjects/KLMChannelIndex.h>

using namespace Belle2;

REG_MODULE(DQMHistAnalysisKLM)

DQMHistAnalysisKLMModule::DQMHistAnalysisKLMModule()
  : DQMHistAnalysisModule(),
    m_eklmStripLayer{nullptr}
{
  m_ChannelArrayIndex = &(KLMChannelArrayIndex::Instance());
  m_SectorArrayIndex = &(KLMSectorArrayIndex::Instance());
  m_ElementNumbers = &(KLMElementNumbers::Instance());
  m_ElementNumbersEKLM = &(EKLM::ElementNumbersSingleton::Instance());
  char label[8];
  const double histMinNDC = 0.1;
  const double histMaxNDC = 0.9;
  const double histRangeNDC = histMaxNDC - histMinNDC;
  for (int sectorFB = 0; sectorFB < BKLMElementNumbers::getMaximalSectorGlobalNumber(); ++sectorFB) {
    double xLineNDC = histMinNDC + (histRangeNDC * sectorFB) / BKLMElementNumbers::getMaximalSectorGlobalNumber();
    double xTextNDC = histMinNDC + (histRangeNDC * (sectorFB + 0.5)) / BKLMElementNumbers::getMaximalSectorGlobalNumber();
    double yTextNDC = histMinNDC + 0.98 * histRangeNDC;
    m_sectorLine[sectorFB] = new TLine(xLineNDC, histMinNDC, xLineNDC, (sectorFB == 0 ? histMinNDC : histMaxNDC));
    m_sectorLine[sectorFB]->SetNDC(true);
    m_sectorLine[sectorFB]->SetLineColor(8); // dark green
    m_sectorLine[sectorFB]->SetLineWidth(1);
    m_sectorLine[sectorFB]->SetLineStyle(2); // dashed
    sprintf(label, "B%c%d", (sectorFB < 8 ? 'B' : 'F'), sectorFB % 8);
    m_sectorText[sectorFB] = new TText(xTextNDC, yTextNDC, label);
    m_sectorText[sectorFB]->SetNDC(true);
    m_sectorText[sectorFB]->SetTextAlign(22); // centred, middle
    m_sectorText[sectorFB]->SetTextColor(8); // dark green
    m_sectorText[sectorFB]->SetTextFont(42); // Helvetica regular
    m_sectorText[sectorFB]->SetTextSize(0.02); // 2% of TPad's full height
  }
}

DQMHistAnalysisKLMModule::~DQMHistAnalysisKLMModule()
{
  for (int sectorFB = 0; sectorFB < BKLMElementNumbers::getMaximalSectorGlobalNumber(); ++sectorFB) {
    delete m_sectorLine[sectorFB];
    delete m_sectorText[sectorFB];
  }
}

void DQMHistAnalysisKLMModule::initialize()
{
}

void DQMHistAnalysisKLMModule::terminate()
{
}

void DQMHistAnalysisKLMModule::beginRun()
{
  if (!m_bklmElectronicsMap.isValid())
    B2FATAL("No BKLM electronics map.");
  if (!m_eklmElectronicsMap.isValid())
    B2FATAL("No EKLM electronics map.");
}

void DQMHistAnalysisKLMModule::endRun()
{
}

void DQMHistAnalysisKLMModule::analyseChannelHitHistogram(
  int subdetector, int section, int sector,
  TH1* histogram, TCanvas* canvas, TLatex& latex)
{
  double x = 0.15;
  double y = 0.85;
  int i, n;
  std::map<uint16_t, double> moduleHitMap;
  std::map<uint16_t, double>::iterator it;
  double nEvents, average;
  int channelSubdetector, channelSection, channelSector;
  int layer, plane, strip;
  std::string str;
  const EKLMDataConcentratorLane* lane;
  canvas->Clear();
  canvas->cd();
  histogram->Draw();
  average = 0;
  n = histogram->GetXaxis()->GetNbins();
  for (i = 1; i <= n; i++) {
    uint16_t channelIndex = std::round(histogram->GetBinCenter(i));
    uint16_t channelNumber = m_ChannelArrayIndex->getNumber(channelIndex);
    nEvents = histogram->GetBinContent(i);
    average = average + nEvents;
    m_ElementNumbers->channelNumberToElementNumbers(
      channelNumber, &channelSubdetector, &channelSection, &channelSector,
      &layer, &plane, &strip);
    if ((channelSubdetector != subdetector) ||
        (channelSection != section) ||
        (channelSector != sector))
      B2FATAL("Inconsistent element numbers.");
    uint16_t module = m_ElementNumbers->moduleNumber(
                        subdetector, section, sector, layer);
    it = moduleHitMap.find(module);
    if (it == moduleHitMap.end())
      moduleHitMap.insert(std::pair<uint16_t, double>(module, nEvents));
    else
      it->second += nEvents;
  }
  unsigned int activeModuleChannels = 0;
  for (it = moduleHitMap.begin(); it != moduleHitMap.end(); ++it) {
    if (it->second != 0) {
      activeModuleChannels += m_ElementNumbers->getNChannelsModule(it->first);
      continue;
    }
    m_ElementNumbers->moduleNumberToElementNumbers(
      it->first, &channelSubdetector, &channelSection, &channelSector, &layer);
    if (m_ElementNumbers->isBKLMChannel(it->first)) {
      uint16_t channel = BKLMElementNumbers::channelNumber(
                           channelSection, channelSector, layer, 0, 1);
      const BKLMElectronicsChannel* electronicsChannel =
        m_bklmElectronicsMap->getElectronicsChannel(channel);
      if (electronicsChannel == nullptr)
        B2FATAL("Incomplete BKLM electronics map.");
      str = "No data from HSLB " +
            BKLMElementNumbers::getHSLBName(
              electronicsChannel->getCopper(), electronicsChannel->getSlot()) +
            ", lane " + std::to_string(electronicsChannel->getLane());
      latex.DrawLatexNDC(x, y, str.c_str());
      y -= 0.05;
    } else {
      int eklmSector = m_ElementNumbers->localChannelNumberEKLM(it->first);
      lane = m_eklmElectronicsMap->getLaneBySector(eklmSector);
      if (lane == nullptr)
        B2FATAL("Incomplete EKLM electronics map.");
      str = "No data from HSLB " +
            EKLMElementNumbers::getHSLBName(lane->getCopper(),
                                            lane->getDataConcentrator()) +
            ", lane " + std::to_string(lane->getLane());
      latex.DrawLatexNDC(x, y, str.c_str());
      y -= 0.05;
    }
  }
  if (activeModuleChannels == 0)
    return;
  average /= activeModuleChannels;
  for (i = 1; i <= n; ++i) {
    uint16_t channelIndex = std::round(histogram->GetBinCenter(i));
    uint16_t channelNumber = m_ChannelArrayIndex->getNumber(channelIndex);
    nEvents = histogram->GetBinContent(i);
    m_ElementNumbers->channelNumberToElementNumbers(
      channelNumber, &channelSubdetector, &channelSection, &channelSector,
      &layer, &plane, &strip);
    uint16_t module = m_ElementNumbers->moduleNumber(
                        subdetector, section, sector, layer);
    if ((nEvents > average * 10) && (nEvents > 50)) {
      if (m_ElementNumbers->isBKLMChannel(channelNumber)) {
        uint16_t detectorChannel = BKLMElementNumbers::channelNumber(
                                     channelSection, channelSector,
                                     layer, plane, strip);
        const BKLMElectronicsChannel* electronicsChannel =
          m_bklmElectronicsMap->getElectronicsChannel(detectorChannel);
        if (electronicsChannel == nullptr)
          B2FATAL("Incomplete BKLM electronics map.");
        str = "Hot channel: HSLB " +
              BKLMElementNumbers::getHSLBName(
                electronicsChannel->getCopper(), electronicsChannel->getSlot()) +
              ", lane " + std::to_string(electronicsChannel->getLane()) +
              ", axis " + std::to_string(electronicsChannel->getAxis()) +
              ", channel " + std::to_string(electronicsChannel->getChannel());
        latex.DrawLatexNDC(x, y, str.c_str());
        y -= 0.05;
      } else {
        int eklmSector = m_ElementNumbers->localChannelNumberEKLM(module);
        lane = m_eklmElectronicsMap->getLaneBySector(eklmSector);
        if (lane == nullptr)
          B2FATAL("Incomplete EKLM electronics map.");
        int asic, channel;
        m_ElementNumbersEKLM->getAsicChannel(plane, strip, &asic, &channel);
        str = "Hot channel: HSLB " +
              EKLMElementNumbers::getHSLBName(lane->getCopper(),
                                              lane->getDataConcentrator()) +
              ", lane " + std::to_string(lane->getLane()) +
              ", asic " + std::to_string(asic) +
              ", channel " + std::to_string(channel);
        latex.DrawLatexNDC(x, y, str.c_str());
        y -= 0.05;
      }
    }
  }
  canvas->Modified();
}

void DQMHistAnalysisKLMModule::processBKLMSectorLayerHistogram(const std::string& histName)
{
  TH1* histogram = findHist("BKLM/" + histName);
  if (histogram == nullptr) {
    B2ERROR("KLM DQM histogram BKLM/" << histName << " is not found.");
    return;
  }
  TCanvas* canvas = findCanvas("BKLM/c_" + histName);
  if (canvas == nullptr) {
    B2ERROR("KLM DQM histogram canvas BKLM/c_" << histName << " is not found.");
    return;
  }
  canvas->Clear();
  canvas->cd();
  histogram->SetStats(false);
  histogram->Draw();
  canvas->Modified();
  for (int sectorFB = 0; sectorFB < BKLMElementNumbers::getMaximalSectorGlobalNumber(); ++sectorFB) {
    m_sectorLine[sectorFB]->Draw();
    m_sectorText[sectorFB]->Draw();
  }
}

TCanvas* DQMHistAnalysisKLMModule::findCanvas(const std::string& canvasName)
{
  TIter nextkey(gROOT->GetListOfCanvases());
  TObject* obj = nullptr;
  while ((obj = (TObject*)nextkey())) {
    if (obj->IsA()->InheritsFrom("TCanvas")) {
      if (obj->GetName() == canvasName) return (TCanvas*)obj;
    }
  }
  return nullptr;
}


void DQMHistAnalysisKLMModule::event()
{
  std::string str, histogramName, canvasName;
  TLatex latex;
  latex.SetTextColor(kRed);
  latex.SetTextAlign(11);
  KLMChannelIndex klmSectors(KLMChannelIndex::c_IndexLevelSector);
  for (KLMChannelIndex& klmSector : klmSectors) {
    int nHistograms;
    if (klmSector.getSubdetector() == KLMElementNumbers::c_BKLM)
      nHistograms = 2;
    else
      nHistograms = 3;
    for (int j = 0; j < nHistograms; j++) {
      str = "strip_hits_subdetector_" +
            std::to_string(klmSector.getSubdetector()) +
            "_section_" + std::to_string(klmSector.getSection()) +
            "_sector_" + std::to_string(klmSector.getSector()) +
            "_" + std::to_string(j);
      histogramName = "KLM/" + str;
      canvasName = "KLM/c_" + str;
      TH1* histogram = findHist(histogramName);
      if (histogram == nullptr) {
        B2ERROR("KLM DQM histogram " << histogramName << " is not found.");
        continue;
      }
      TCanvas* canvas = findCanvas(canvasName);
      if (canvas == nullptr) {
        B2ERROR("KLM DQM histogram canvas " << canvasName << " is not found.");
        continue;
      }
      analyseChannelHitHistogram(
        klmSector.getSubdetector(), klmSector.getSection(),
        klmSector.getSector(), histogram, canvas, latex);
    }
  }
  processBKLMSectorLayerHistogram("SectorLayerPhi");
  processBKLMSectorLayerHistogram("SectorLayerZ");
}
