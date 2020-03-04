/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin, Leo Piilonen, Vipin Gaur                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <dqm/analysis/modules/DQMHistAnalysisKLM.h>

/* Belle 2 headers. */
#include <klm/dataobjects/KLMChannelIndex.h>

/* ROOT headers. */
#include <TClass.h>
#include <TROOT.h>

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
  m_PlaneLine.SetLineColor(8); // dark green
  m_PlaneLine.SetLineWidth(1);
  m_PlaneLine.SetLineStyle(2); // dashed
  m_PlaneText.SetTextAlign(22); // centred, middle
  m_PlaneText.SetTextColor(8); // dark green
  m_PlaneText.SetTextFont(42); // Helvetica regular
  m_PlaneText.SetTextSize(0.02); // 2% of TPad's full height
}

DQMHistAnalysisKLMModule::~DQMHistAnalysisKLMModule()
{
}

void DQMHistAnalysisKLMModule::initialize()
{
}

void DQMHistAnalysisKLMModule::terminate()
{
}

void DQMHistAnalysisKLMModule::beginRun()
{
  if (!m_ElectronicsMap.isValid())
    B2FATAL("No KLM electronics map.");
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
    /* Channel with plane = 1, strip = 1 exists for any BKLM or EKLM module. */
    uint16_t channel = m_ElementNumbers->channelNumber(
                         channelSubdetector, channelSection, channelSector,
                         layer, 1, 1);
    const KLMElectronicsChannel* electronicsChannel =
      m_ElectronicsMap->getElectronicsChannel(channel);
    if (electronicsChannel == nullptr)
      B2FATAL("Incomplete KLM electronics map.");
    str = "No data from HSLB ";
    if (channelSubdetector == KLMElementNumbers::c_BKLM) {
      str += BKLMElementNumbers::getHSLBName(electronicsChannel->getCopper(),
                                             electronicsChannel->getSlot());
    } else {
      str += EKLMElementNumbers::getHSLBName(electronicsChannel->getCopper(),
                                             electronicsChannel->getSlot());
    }
    str += ", lane " + std::to_string(electronicsChannel->getLane());
    latex.DrawLatexNDC(x, y, str.c_str());
    y -= 0.05;
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
    if ((nEvents > average * 10) && (nEvents > 50)) {
      const KLMElectronicsChannel* electronicsChannel =
        m_ElectronicsMap->getElectronicsChannel(channelNumber);
      if (electronicsChannel == nullptr)
        B2FATAL("Incomplete BKLM electronics map.");
      str = "Hot channel: HSLB ";
      if (channelSubdetector == KLMElementNumbers::c_BKLM) {
        str += BKLMElementNumbers::getHSLBName(electronicsChannel->getCopper(),
                                               electronicsChannel->getSlot());
      } else {
        str += EKLMElementNumbers::getHSLBName(electronicsChannel->getCopper(),
                                               electronicsChannel->getSlot());
      }
      str += (", lane " + std::to_string(electronicsChannel->getLane()) +
              ", axis " + std::to_string(electronicsChannel->getAxis()) +
              ", channel " + std::to_string(electronicsChannel->getChannel()));
      latex.DrawLatexNDC(x, y, str.c_str());
      y -= 0.05;
    }
  }
  canvas->Modified();
}

void DQMHistAnalysisKLMModule::processPlaneHistogram(
  const std::string& histName)
{
  std::string name;
  const double histMinNDC = 0.1;
  const double histMaxNDC = 0.9;
  const double histRangeNDC = histMaxNDC - histMinNDC;
  TH1* histogram = findHist("KLM/" + histName);
  if (histogram == nullptr) {
    B2ERROR("KLM DQM histogram KLM/" << histName << " is not found.");
    return;
  }
  TCanvas* canvas = findCanvas("KLM/c_" + histName);
  if (canvas == nullptr) {
    B2ERROR("KLM DQM histogram canvas KLM/c_" << histName << " is not found.");
    return;
  }
  canvas->Clear();
  canvas->cd();
  histogram->SetStats(false);
  histogram->Draw();
  canvas->Modified();
  if (histName.find("bklm") != std::string::npos) {
    const double maximalSector = BKLMElementNumbers::getMaximalSectorGlobalNumber();
    for (int sector = 0; sector < BKLMElementNumbers::getMaximalSectorGlobalNumber(); ++sector) {
      double xLineNDC = histMinNDC + (histRangeNDC * sector) / maximalSector;
      double xTextNDC = histMinNDC + (histRangeNDC * (sector + 0.5)) / maximalSector;
      double yTextNDC = histMinNDC + 0.98 * histRangeNDC;
      if (sector > 0)
        m_PlaneLine.DrawLineNDC(xLineNDC, histMinNDC, xLineNDC, histMaxNDC);
      name = "B";
      if (sector < 8)
        name += "B";
      else
        name += "F";
      name += std::to_string(sector % 8);
      m_PlaneText.DrawTextNDC(xTextNDC, yTextNDC, name.c_str());
    }
  } else {
    const double maximalLayer = EKLMElementNumbers::getMaximalLayerGlobalNumber();
    for (int layerGlobal = 1; layerGlobal <= maximalLayer; ++layerGlobal) {
      double xLineNDC = histMinNDC + (histRangeNDC * layerGlobal) / maximalLayer;
      double xTextNDC = histMinNDC + (histRangeNDC * (layerGlobal - 0.5)) / maximalLayer;
      double yTextNDC = histMinNDC + 0.98 * histRangeNDC;
      if (layerGlobal < maximalLayer)
        m_PlaneLine.DrawLineNDC(xLineNDC, histMinNDC, xLineNDC, histMaxNDC);
      int section, layer;
      m_ElementNumbersEKLM->layerNumberToElementNumbers(
        layerGlobal, &section, &layer);
      if (section == EKLMElementNumbers::c_BackwardSection)
        name = "B";
      else
        name = "F";
      name += std::to_string(layer);
      m_PlaneText.DrawTextNDC(xTextNDC, yTextNDC, name.c_str());
    }
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
  processPlaneHistogram("plane_bklm_phi");
  processPlaneHistogram("plane_bklm_z");
  processPlaneHistogram("plane_eklm");
}
