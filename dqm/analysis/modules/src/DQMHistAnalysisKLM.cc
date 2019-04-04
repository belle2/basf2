/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <dqm/analysis/modules/DQMHistAnalysisKLM.h>

using namespace Belle2;

REG_MODULE(DQMHistAnalysisKLM)

DQMHistAnalysisKLMModule::DQMHistAnalysisKLMModule()
  : DQMHistAnalysisModule(),
    m_eklmStripLayer{nullptr}
{
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
}

DQMHistAnalysisKLMModule::~DQMHistAnalysisKLMModule()
{
}

void DQMHistAnalysisKLMModule::initialize()
{
  std::string str;
  for (int i = 0; i < EKLMElementNumbers::getMaximalLayerGlobalNumber(); ++i) {
    str = "EKLM/c_analysed_strip_layer_" + std::to_string(i + 1);
    m_eklmStripLayer[i] = new TCanvas(str.c_str());
  }
}

void DQMHistAnalysisKLMModule::terminate()
{
  for (int i = 0; i < EKLMElementNumbers::getMaximalLayerGlobalNumber(); ++i)
    delete m_eklmStripLayer[i];
}

void DQMHistAnalysisKLMModule::beginRun()
{
  if (!m_ElectronicsMap.isValid())
    B2FATAL("No EKLM electronics map.");
}

void DQMHistAnalysisKLMModule::endRun()
{
}

void DQMHistAnalysisKLMModule::analyseStripLayerHistogram(
  int layerGlobal, TH1* histogram, TLatex& latex)
{
  int i;
  double x = 0.15;
  double y = 0.85;
  double nEvents, average;
  int endcap, layer, sector, plane, strip, sectorGlobal, sectorsWithSignal;
  double sectorEvents[EKLMElementNumbers::getMaximalSectorNumber()] = {0};
  std::string str;
  const EKLMDataConcentratorLane* lane;
  average = 0;
  m_ElementNumbers->layerNumberToElementNumbers(layerGlobal, &endcap, &layer);
  for (i = 1; i <= EKLMElementNumbers::getNStripsLayer(); ++i) {
    nEvents = histogram->GetBinContent(i);
    average = average + nEvents;
    sector = (i - 1) / EKLMElementNumbers::getNStripsSector();
    sectorEvents[sector] += nEvents;
  }
  sectorsWithSignal = 0;
  for (i = 0; i < EKLMElementNumbers::getMaximalSectorNumber(); ++i) {
    if (sectorEvents[i] != 0) {
      sectorsWithSignal++;
      continue;
    }
    sectorGlobal = m_ElementNumbers->sectorNumber(endcap, layer, sector);
    lane = m_ElectronicsMap->getLaneBySector(sectorGlobal);
    if (lane == nullptr)
      B2FATAL("Incomplete EKLM electronics map.");
    str = "No data from HSLB " +
          EKLMElementNumbers::getHSLBName(lane->getCopper(),
                                          lane->getDataConcentrator()) +
          ", lane " + std::to_string(lane->getLane());
    latex.DrawLatexNDC(x, y, str.c_str());
    y -= 0.05;
  }
  if (sectorsWithSignal == 0)
    return;
  average /= (sectorsWithSignal * EKLMElementNumbers::getNStripsSector());
  for (i = 1; i <= EKLMElementNumbers::getNStripsLayer(); ++i) {
    nEvents = histogram->GetBinContent(i);
    sector = (i - 1) / EKLMElementNumbers::getNStripsSector() + 1;
    sectorGlobal = m_ElementNumbers->sectorNumber(endcap, layer, sector);
    lane = m_ElectronicsMap->getLaneBySector(sectorGlobal);
    if (lane == nullptr)
      B2FATAL("Incomplete EKLM electronics map.");
    if ((nEvents > average * 10) && (nEvents > 50)) {
      plane = ((i - 1) % EKLMElementNumbers::getNStripsSector()) /
              EKLMElementNumbers::getMaximalStripNumber() + 1;
      strip = (i - 1) % EKLMElementNumbers::getMaximalStripNumber() + 1;
      int asic, channel;
      m_ElementNumbers->getAsicChannel(plane, strip, &asic, &channel);
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

void DQMHistAnalysisKLMModule::event()
{
  std::string str;
  TLatex latex;
  latex.SetTextColor(kRed);
  latex.SetTextAlign(11);
  for (int i = 0; i < EKLMElementNumbers::getMaximalLayerGlobalNumber(); ++i) {
    str = "EKLM/strip_layer_" + std::to_string(i + 1);
    TH1* h = findHist(str);
    if (h == nullptr) {
      B2ERROR("KLM DQM histogram " << str << " is not found.");
      continue;
    }
    m_eklmStripLayer[i]->Clear();
    m_eklmStripLayer[i]->cd();
    h->Draw();
    analyseStripLayerHistogram(i + 1, h, latex);
    m_eklmStripLayer[i]->Modified();
  }
}
