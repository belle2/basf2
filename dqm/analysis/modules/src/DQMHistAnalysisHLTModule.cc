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
  addParam("l1Histograms", m_l1Histograms, "Which l1 histograms to show", m_l1Histograms);
}

void DQMHistAnalysisHLTModule::initialize()
{
  // this seems to be important, or strange things happen
  gROOT->cd();

  m_hEfficiency = {
    new TCanvas("HLT/Ratio"),
    new TH1F("Ratio", "Ratio of Tags to HLT triggered events", 1, 0, 0)
  };
  m_hEfficiencyTotal = {
    new TCanvas("HLT/RatioTotal"),
    new TH1F("Ratio", "Ratio of Tags to all events", 1, 0, 0)
  };
  m_hCrossSection = {
    new TCanvas("HLT/CrossSection"),
    new TH1F("CrossSection", "Cross Section of triggered Events", 1, 0, 0)
  };
  m_hRatios = {
    new TCanvas("HLT/RatioToBahbha"),
    new TH1F("RatioToBahbha", "Ratio to bhabha events", 1, 0, 0)
  };

  for (const std::string& l1Name : m_l1Histograms) {
    m_hl1Ratios.emplace(l1Name, std::make_pair(
                          new TCanvas(("HLT/" + l1Name + "RatioToL1").c_str()),
                          // + 1 for total result
                          new TH1F((l1Name + "RatioToL1").c_str(), ("HLT Fractions for L1 " + l1Name).c_str(), 1, 0, 0)
                        ));
  }

  for (auto& canvasAndHisto : {m_hEfficiencyTotal, m_hEfficiency, m_hCrossSection, m_hRatios}) {
    auto* histogram = canvasAndHisto.second;
    histogram->SetDirectory(0);
    histogram->SetOption("bar");
    histogram->SetFillStyle(0);
    histogram->SetStats(false);
    histogram->Draw("");
  }

  for (auto& nameAndcanvasAndHisto : m_hl1Ratios) {
    auto* histogram = nameAndcanvasAndHisto.second.second;
    histogram->SetDirectory(0);
    histogram->SetOption("bar");
    histogram->SetFillStyle(0);
    histogram->SetStats(false);
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
  for (auto& canvasAndHisto : {m_hEfficiencyTotal, m_hEfficiency, m_hCrossSection, m_hRatios}) {
    auto* canvas = canvasAndHisto.first;
    canvas->Clear();
  }

  for (auto& nameAndcanvasAndHisto : m_hl1Ratios) {
    auto* canvas = nameAndcanvasAndHisto.second.first;
    canvas->Clear();
  }
}

void DQMHistAnalysisHLTModule::event()
{
  auto* filterHistogram = findHist("softwaretrigger/filter");
  auto* skimHistogram = findHist("softwaretrigger/skim");
  auto* totalResultHistogram = findHist("softwaretrigger/total_result");
  auto* eventNumberHistogram = findHist("softwaretrigger/event_number");

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
  if (not eventNumberHistogram) {
    B2ERROR("Can not find the event number histogram!");
    return;
  }

  m_hEfficiencyTotal.second->Reset();
  m_hEfficiency.second->Reset();
  m_hCrossSection.second->Reset();
  m_hRatios.second->Reset();

  double instLuminosity = 0;
  double numberOfAcceptedHLTEvents = getValue("total_result", totalResultHistogram);
  double numberOfBhabhaEvents = getValue(m_bhabhaName, skimHistogram);
  double numberOfAllEvents = eventNumberHistogram->GetEntries();

#ifdef _BELLE2_EPICS
  if (not m_pvPrefix.empty()) {
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    SEVCHK(ca_get(DBR_DOUBLE, m_epicschid, (void*)&instLuminosity), "ca_get failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  } else {
    instLuminosity = 0;
  }
#endif

  m_hEfficiencyTotal.second->Fill("total_result", numberOfAcceptedHLTEvents / numberOfAllEvents);
  if (instLuminosity != 0) {
    m_hCrossSection.second->Fill("total_result", numberOfAcceptedHLTEvents / numberOfAllEvents * instLuminosity);
  }
  m_hRatios.second->Fill("total_result", numberOfAcceptedHLTEvents / numberOfBhabhaEvents);

  for (const auto& columnMapping : m_columnMapping) {
    const auto& from = columnMapping.first;
    const auto& to = columnMapping.second;

    double value = 0;
    if (hasValue(from, filterHistogram)) {
      value = getValue(from, filterHistogram);
    } else if (hasValue(from, skimHistogram)) {
      value = getValue(from, skimHistogram);
    } else {
      B2ERROR("Can not find value " << from << ". Will not use it!");
      continue;
    }

    m_hEfficiency.second->Fill(to.c_str(), value / numberOfAcceptedHLTEvents);
    m_hEfficiencyTotal.second->Fill(to.c_str(), value / numberOfAllEvents);
    if (instLuminosity != 0) {
      m_hCrossSection.second->Fill(to.c_str(), value / numberOfAllEvents * instLuminosity);
    }
    m_hRatios.second->Fill(to.c_str(), value / numberOfBhabhaEvents);
  }

  for (const std::string& l1Name : m_l1Histograms) {
    auto* histogram = m_hl1Ratios.at(l1Name).second;
    histogram->Reset();

    auto* l1Histogram = findHist("softwaretrigger/" + l1Name);
    auto* l1TotalResultHistogram = findHist("softwaretrigger/l1_total_result");

    if (not l1Histogram or not l1TotalResultHistogram) {
      B2ERROR("Can not find L1 histograms from softwaretrigger!");
      continue;
    }

    for (const auto& columnMapping : m_columnMapping) {
      const auto& from = columnMapping.first;
      const auto& to = columnMapping.second;

      if (not hasValue(from, l1Histogram)) {
        B2ERROR("Can not find label " << from << " in l1 histogram " << l1Name);
        continue;
      }

      if (not hasValue(l1Name, l1TotalResultHistogram)) {
        B2ERROR("Can not find label " << l1Name << " in l1 total result histogram");
        continue;
      }

      const double hltValueInL1Bin = getValue(from, l1Histogram);
      const double l1TotalResult = getValue(l1Name, l1TotalResultHistogram);

      histogram->Fill(to.c_str(), hltValueInL1Bin / l1TotalResult);
    }

    // and add a total result bin
    const auto from = "hlt_result";
    const auto to = "hlt_result";

    if (not hasValue(from, l1Histogram)) {
      B2ERROR("Can not find label " << from << " in l1 histogram " << l1Name);
      continue;
    }

    if (not hasValue(l1Name, l1TotalResultHistogram)) {
      B2ERROR("Can not find label " << l1Name << " in l1 total result histogram");
      continue;
    }

    const double hltValueInL1Bin = getValue(from, l1Histogram);
    const double l1TotalResult = getValue(l1Name, l1TotalResultHistogram);

    histogram->Fill(to, hltValueInL1Bin / l1TotalResult);
  }


  for (auto& canvasAndHisto : {m_hEfficiencyTotal, m_hEfficiency, m_hCrossSection, m_hRatios}) {
    auto* canvas = canvasAndHisto.first;
    auto* histogram = canvasAndHisto.second;

    canvas->cd();
    histogram->Draw("");
    canvas->Modified();
    canvas->Update();
  }

  for (auto& nameAndCanvasAndHisto : m_hl1Ratios) {
    auto* canvas = nameAndCanvasAndHisto.second.first;
    auto* histogram = nameAndCanvasAndHisto.second.second;

    canvas->cd();
    histogram->Draw("");
    canvas->Modified();
    canvas->Update();
  }
}