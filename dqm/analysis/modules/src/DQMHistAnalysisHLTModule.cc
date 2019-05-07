/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <dqm/analysis/modules/DQMHistAnalysisHLTModule.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <TROOT.h>

using namespace std;
using namespace Belle2;

namespace {
  bool hasValue(const std::string& name, TH1* histogram)
  {
    return histogram->GetXaxis()->FindFixBin(name.c_str()) != -1;
  }

  double getValue(const std::string& name, TH1* histogram)
  {
    B2ASSERT("This histogram does not have this value!", hasValue(name, histogram));
    auto binNumber = histogram->GetXaxis()->FindFixBin(name.c_str());
    return histogram->GetBinContent(binNumber);
  }
}

REG_MODULE(DQMHistAnalysisHLT)

DQMHistAnalysisHLTModule::DQMHistAnalysisHLTModule()
{
  addParam("pvPrefix", m_pvPrefix, "EPICS PV Name for the inst. luminosity", m_pvPrefix);
  addParam("bhabhaName", m_bhabhaName, "Name of the bhabha trigger to do a ratio against", m_bhabhaName);
  addParam("columnMapping", m_columnMapping, "Which columns to use for calculating ratios and cross sections", m_columnMapping);
}

void DQMHistAnalysisHLTModule::initialize()
{
  // this seems to be important, or strange things happen
  gROOT->cd();

  m_hEfficiency = {
    new TCanvas("HLT/Ratio"),
    new TH1F("Ratio", "Ratio to total triggered events", m_columnMapping.size(), 0, m_columnMapping.size())
  };
  m_hCrossSection = {
    new TCanvas("HLT/CrossSection"),
    new TH1F("CrossSection", "Cross Section of triggered Events", m_columnMapping.size(), 0, m_columnMapping.size())
  };
  m_hRatios = {
    new TCanvas("HLT/RatioToBahbha"),
    new TH1F("RatioToBahbha", "Ratio to bhabha events", m_columnMapping.size(), 0, m_columnMapping.size())
  };

  for (auto& canvasAndHisto : {m_hEfficiency, m_hCrossSection, m_hRatios}) {
    auto* histogram = canvasAndHisto.second;
    histogram->SetDirectory(0);
    histogram->SetOption("bar");
    histogram->SetFillStyle(0);
    histogram->SetStats(false);

    unsigned int counter = 0;
    for (const auto& columnMapping : m_columnMapping) {
      const auto& to = columnMapping.second;

      histogram->GetXaxis()->SetBinLabel(counter + 1, to.c_str());
      counter++;
    }

    histogram->Draw("");
  }

#ifdef _BELLE2_EPICS
  if (not m_pvPrefix.empty()) {
    SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    SEVCHK(ca_create_channel(m_pvPrefix.data(), NULL, NULL, 10, &m_epicschid), "ca_create_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}


void DQMHistAnalysisHLTModule::beginRun()
{
  for (auto& canvasAndHisto : {m_hEfficiency, m_hCrossSection, m_hRatios}) {
    auto* canvas = canvasAndHisto.first;
    canvas->Clear();
  }
}

void DQMHistAnalysisHLTModule::event()
{
  auto* filterHistogram = findHist("softwaretrigger/filter");
  auto* skimHistogram = findHist("softwaretrigger/skim");
  auto* totalResultHistogram = findHist("softwaretrigger/total_result");

  if (not filterHistogram) {
    B2ERROR("Can not find the filter histogram!");
    return;
  }
  if (not skimHistogram) {
    B2ERROR("Can not find the skim histogram!");
    return;
  }
  if (not totalResultHistogram) {
    B2ERROR("Can not find the total result histogram!");
    return;
  }

  m_hEfficiency.second->Reset();
  m_hCrossSection.second->Reset();
  m_hRatios.second->Reset();

  double instLuminosity = 0;
  double totalNumberOfEvents = getValue("total_result", totalResultHistogram);
  double numberOfBhabhaEvents = getValue(m_bhabhaName, skimHistogram);

#ifdef _BELLE2_EPICS
  if (not m_pvPrefix.empty()) {
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    SEVCHK(ca_get(DBR_DOUBLE, m_epicschid, (void*)&instLuminosity), "ca_get failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  } else {
    instLuminosity = NAN;
  }
#endif

  unsigned int counter = 0;
  for (const auto& columnMapping : m_columnMapping) {
    const auto& from = columnMapping.first;

    double value = 0;
    if (hasValue(from, filterHistogram)) {
      value = getValue(from, filterHistogram);
    } else if (hasValue(from, skimHistogram)) {
      value = getValue(from, skimHistogram);
    } else {
      B2ERROR("Can not find value " << from << ". Will not use it!");

      m_hEfficiency.second->Fill(counter, 0);
      m_hCrossSection.second->Fill(counter, 0);
      m_hRatios.second->Fill(counter, 0);

      counter++;
      continue;
    }

    m_hEfficiency.second->Fill(counter, value / totalNumberOfEvents);
    m_hCrossSection.second->Fill(counter, value / totalNumberOfEvents * instLuminosity);
    m_hRatios.second->Fill(counter, value / numberOfBhabhaEvents);

    counter++;
  }

  for (auto& canvasAndHisto : {m_hEfficiency, m_hCrossSection, m_hRatios}) {
    auto* canvas = canvasAndHisto.first;
    auto* histogram = canvasAndHisto.second;

    canvas->cd();
    histogram->Draw("");
    canvas->Modified();
    canvas->Update();
  }
}