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
  : DQMHistAnalysisModule()
{
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


void DQMHistAnalysisKLMModule::beginRun()
{
}

void DQMHistAnalysisKLMModule::event()
{
  std::string str;
  for (int i = 0; i < EKLMElementNumbers::getMaximalLayerGlobalNumber(); ++i) {
    str = "EKLM/strip_layer_" + std::to_string(i + 1);
    TH1* h = findHist(str);
    if (h == NULL) {
      B2ERROR("KLM DQM histogram " << str << " is not found.");
      continue;
    }
    m_eklmStripLayer[i]->Clear();
    m_eklmStripLayer[i]->cd();
    h->Draw();
    m_eklmStripLayer[i]->Modified();
  }
}

void DQMHistAnalysisKLMModule::endRun()
{
}

void DQMHistAnalysisKLMModule::terminate()
{
  for (int i = 0; i < EKLMElementNumbers::getMaximalLayerGlobalNumber(); ++i)
    delete m_eklmStripLayer[i];
}
