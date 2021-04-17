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

#include <hlt/softwaretrigger/modules/dqm/SoftwareTriggerHLTDQMModule.h>

using namespace std;
using namespace Belle2;

namespace {
  bool hasValue(const std::string& name, TH1* histogram)
  {
    return histogram->GetXaxis()->FindFixBin(name.c_str()) != -1;
  }

  double getValue(const std::string& name, TH1* histogram)
  {
    if (not hasValue(name, histogram)) {
      B2ERROR("This histogram does not have this value! (fallback value = -1)"
              << LogVar("histogram", histogram->GetName())
              << LogVar("value", name));
      return -1;
    }
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
  addParam("retentionPerUnit", m_retentionPerUnit, "Which HLT filter lines to use for calculation retention rate per unit",
           m_retentionPerUnit);
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
    new TH1F("RatioTotal", "Ratio of Tags to all events", 1, 0, 0)
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

  for (const std::string& filterLine : m_retentionPerUnit) {
    m_hRetentionPerUnit.emplace(filterLine, std::make_pair(
                                  new TCanvas(("HLT/" + filterLine + "_RetentionPerUnit").c_str()),
                                  new TH1F((filterLine + "_RetentionPerUnit").c_str(), ("Retention rate per unit of: " + filterLine).c_str(),
                                           SoftwareTrigger::HLTUnit::max_hlt_units + 1, 0,
                                           SoftwareTrigger::HLTUnit::max_hlt_units + 1)
                                ));
  }

  for (auto& canvasAndHisto : {m_hEfficiencyTotal, m_hEfficiency, m_hCrossSection, m_hRatios}) {
    auto* histogram = canvasAndHisto.second;
    histogram->SetDirectory(0);
    histogram->SetOption("bar");
    histogram->SetFillStyle(0);
    histogram->SetStats(false);
    histogram->Draw("hist");
  }

  for (auto& nameAndcanvasAndHisto : m_hl1Ratios) {
    auto* histogram = nameAndcanvasAndHisto.second.second;
    histogram->SetDirectory(0);
    histogram->SetOption("bar");
    histogram->SetFillStyle(0);
    histogram->SetStats(false);
    histogram->Draw("hist");
  }

  for (auto& nameAndcanvasAndHisto : m_hRetentionPerUnit) {
    auto* histogram = nameAndcanvasAndHisto.second.second;
    histogram->SetDirectory(0);
    histogram->SetOption("histe");
    histogram->SetMinimum(0);
    histogram->SetStats(false);
    histogram->Draw();
  }

  m_hMeanTime = {
    new TCanvas("HLT/MeanTime"),
    new TH1F("MeanTime", "Mean processing time", 1, 0, 0)
  };

  m_hErrorFlagFraction = {
    new TCanvas("HLT/ErrorFlagFraction"),
    new TH1D("ErrorFlagFraction", "Fraction of events with Error Flags", 1, 0, 0)
  };

  m_hFilteredFractionPerUnit = {
    new TCanvas("HLT/FilteredFractionPerUnit"),
    new TH1D("FilteredFractionPerUnit", "Fraction of events filtered per unit", 1, 0, 0)
  };

#ifdef _BELLE2_EPICS
  if (not m_pvPrefix.empty()) {
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    SEVCHK(ca_create_channel(m_pvPrefix.data(), NULL, NULL, 10, &m_epicschid), "ca_create_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}


void DQMHistAnalysisHLTModule::beginRun()
{
  for (auto& canvasAndHisto : {m_hEfficiencyTotal, m_hEfficiency, m_hCrossSection, m_hRatios, m_hMeanTime}) {
    auto* canvas = canvasAndHisto.first;
    canvas->Clear();
  }

  for (auto& canvasAndHisto : {m_hErrorFlagFraction, m_hFilteredFractionPerUnit}) {
    auto* canvas = canvasAndHisto.first;
    canvas->Clear();
  }

  for (auto& nameAndcanvasAndHisto : m_hl1Ratios) {
    auto* canvas = nameAndcanvasAndHisto.second.first;
    canvas->Clear();
  }

  for (auto& nameAndcanvasAndHisto : m_hRetentionPerUnit) {
    auto* canvas = nameAndcanvasAndHisto.second.first;
    canvas->Clear();
  }
}

void DQMHistAnalysisHLTModule::event()
{
  auto* filterHistogram = findHist("softwaretrigger/filter");
  auto* skimHistogram = findHist("softwaretrigger/skim");
  auto* totalResultHistogram = findHist("softwaretrigger/total_result");
  auto* hltUnitNumberHistogram = findHist("softwaretrigger_before_filter/hlt_unit_number");
  auto* processesPerUnitHistogram = findHist("timing_statistics/processesPerUnitHistogram");
  auto* meanTimeHistogram = findHist("timing_statistics/meanTimeHistogram");
  auto* errorFlagHistogram = findHist("softwaretrigger_before_filter/error_flag");
  auto* hltUnitNumberHistogram_filtered = findHist("softwaretrigger/hlt_unit_number_after_filter");

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
  if (not hltUnitNumberHistogram) {
    B2ERROR("Can not find the HLT unit number histogram!");
    return;
  }
  if (not processesPerUnitHistogram) {
    B2ERROR("Can not find the processes per unit histogram!");
    return;
  }
  if (not meanTimeHistogram) {
    B2ERROR("Can not find the mean processing time histogram!");
    return;
  }
  if (not errorFlagHistogram) {
    B2ERROR("Can not find the error flag histogram!");
    return;
  }
  if (not hltUnitNumberHistogram_filtered) {
    B2ERROR("Can not find the HLT unit number after filter histogram!");
    return;
  }

  m_hEfficiencyTotal.second->Reset();
  m_hEfficiency.second->Reset();
  m_hCrossSection.second->Reset();
  m_hRatios.second->Reset();
  m_hMeanTime.second->Reset();
  m_hErrorFlagFraction.second->Reset();
  m_hFilteredFractionPerUnit.second->Reset();

  double instLuminosity = 0;
  double numberOfAcceptedHLTEvents = getValue("total_result", totalResultHistogram);
  double numberOfBhabhaEvents = getValue(m_bhabhaName, skimHistogram);
  double numberOfAllEvents = hltUnitNumberHistogram->GetEntries();
  double numberOfProcesses = processesPerUnitHistogram->GetEntries();

#ifdef _BELLE2_EPICS
  if (not m_pvPrefix.empty()) {
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

  for (const std::string& filterLine : m_retentionPerUnit) {
    auto* histogram = m_hRetentionPerUnit.at(filterLine).second;
    histogram->Reset();

    auto* filterLineHistogram = findHist("softwaretrigger/" + filterLine + "_per_unit");

    if (not filterLineHistogram) {
      B2ERROR("Can not find " << filterLineHistogram << "_per_event histograms from softwaretrigger!");
      continue;
    }

    for (unsigned int i = 1; i <= SoftwareTrigger::HLTUnit::max_hlt_units + 1; i++) {
      double totalUnitValue = hltUnitNumberHistogram->GetBinContent(i);
      if (totalUnitValue == 0) {
        histogram->Fill(i, 0);
      } else {
        double filterLineUnitValue = filterLineHistogram->GetBinContent(i);
        histogram->SetBinContent(i, filterLineUnitValue / totalUnitValue);
      }
    }
  }

  m_hMeanTime.second = (TH1F*) meanTimeHistogram->Clone("MeanTime");
  m_hMeanTime.second->Scale(1 / numberOfProcesses);

  m_hErrorFlagFraction.second = (TH1D*) errorFlagHistogram->Clone("ErrorFlagFraction");
  m_hErrorFlagFraction.second->Scale(1 / numberOfAllEvents);
  m_hErrorFlagFraction.second->SetTitle("Fraction of events with error flags");

  m_hFilteredFractionPerUnit.second = (TH1D*) hltUnitNumberHistogram_filtered->Clone("FilteredFractionPerUnit");
  m_hFilteredFractionPerUnit.second->Divide(hltUnitNumberHistogram_filtered, hltUnitNumberHistogram);
  m_hFilteredFractionPerUnit.second->SetTitle("Fraction of events filtered per unit");

  for (auto& canvasAndHisto : {m_hEfficiencyTotal, m_hEfficiency, m_hCrossSection, m_hRatios, m_hMeanTime}) {
    auto* canvas = canvasAndHisto.first;
    auto* histogram = canvasAndHisto.second;

    canvas->cd();
    histogram->LabelsDeflate("X");
    histogram->Draw("hist");
    canvas->Modified();
    canvas->Update();
  }

  for (auto& canvasAndHisto : {m_hErrorFlagFraction, m_hFilteredFractionPerUnit}) {
    auto* canvas = canvasAndHisto.first;
    auto* histogram = canvasAndHisto.second;

    canvas->cd();
    histogram->LabelsDeflate("X");
    histogram->Draw("hist");
    histogram->SetStats(false);
    canvas->Modified();
    canvas->Update();
  }

  for (auto& nameAndCanvasAndHisto : m_hl1Ratios) {
    auto* canvas = nameAndCanvasAndHisto.second.first;
    auto* histogram = nameAndCanvasAndHisto.second.second;

    canvas->cd();
    histogram->LabelsDeflate("X");
    histogram->Draw("hist");
    canvas->Modified();
    canvas->Update();
  }

  for (auto& nameAndCanvasAndHisto : m_hRetentionPerUnit) {
    auto* canvas = nameAndCanvasAndHisto.second.first;
    auto* histogram = nameAndCanvasAndHisto.second.second;

    canvas->cd();
    histogram->Draw("hist");
    canvas->Modified();
    canvas->Update();
  }
}

void DQMHistAnalysisHLTModule::terminate()
{
#ifdef _BELLE2_EPICS
  if (not m_pvPrefix.empty()) {
    SEVCHK(ca_clear_channel(m_epicschid), "ca_clear_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}
