/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//THIS MODULE
#include <dqm/analysis/modules/DQMHistAnalysisECLShapers.h>

//ROOT
#include <TProfile.h>

//boost
#include "boost/format.hpp"

//std
#include <numeric>

using namespace Belle2;

REG_MODULE(DQMHistAnalysisECLShapers);

DQMHistAnalysisECLShapersModule::DQMHistAnalysisECLShapersModule()
  : DQMHistAnalysisModule()
{
  B2DEBUG(20, "DQMHistAnalysisECLShapers: Constructor done.");
  setDescription("Processes information involving ECL pedestals (widths and rms).");

  addParam("pvPrefix", m_pvPrefix, "Prefix to use for PVs registered by this module",
           std::string("ECL:"));
}


DQMHistAnalysisECLShapersModule::~DQMHistAnalysisECLShapersModule() { }

void DQMHistAnalysisECLShapersModule::initialize()
{
  for (int i = 0; i < c_collector_count; i++) {
    std::string pv_name = (boost::format("logic_check:crate%02d") %
                           (i + 1)).str();
    registerEpicsPV(m_pvPrefix + pv_name, pv_name);
  }
  for (auto part_id : {"fw", "br", "bw", "al"}) {
    std::string pv_name = "pedwidth:max_";
    pv_name += part_id;
    registerEpicsPV(m_pvPrefix + pv_name, pv_name);

    pv_name = "pedwidth:avg_";
    pv_name += part_id;
    registerEpicsPV(m_pvPrefix + pv_name, pv_name);
  }

  m_monObj = getMonitoringObject("ecl");

  m_c_main = new TCanvas("ecl_main");
  m_monObj->addCanvas(m_c_main);

  B2DEBUG(20, "DQMHistAnalysisECLShapers: initialized.");
}

void DQMHistAnalysisECLShapersModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisECLShapers: beginRun called.");
}

void DQMHistAnalysisECLShapersModule::event()
{
  TH1* h_fail_crateid  = findHist("ECL/fail_crateid");
  TProfile* h_pedrms_cellid = (TProfile*)findHist("ECL/pedrms_cellid");

  if (h_pedrms_cellid != NULL) {
    // Using multiset to automatically sort the added values.
    std::multiset<double> barrel_pedwidth;
    std::multiset<double> bwd_pedwidth;
    std::multiset<double> fwd_pedwidth;

    for (int i = 0; i < 8736; i++) {
      const int cellid = i + 1;
      if (h_pedrms_cellid->GetBinEntries(cellid) < 100) continue;
      double pedrms = h_pedrms_cellid->GetBinContent(cellid);
      if (cellid < 1153) {
        fwd_pedwidth.insert(pedrms);
      } else if (cellid < 7777) {
        barrel_pedwidth.insert(pedrms);
      } else {
        bwd_pedwidth.insert(pedrms);
      }
    }

    // Use approximate conversion factor
    // to convert from ADC units to MeV.
    const double adc_to_mev = 0.05;

    m_pedwidth_max[0] = robust_max(fwd_pedwidth)    * adc_to_mev;
    m_pedwidth_max[1] = robust_max(barrel_pedwidth) * adc_to_mev;
    m_pedwidth_max[2] = robust_max(bwd_pedwidth)    * adc_to_mev;
    m_pedwidth_max[3] = *std::max_element(&m_pedwidth_max[0], &m_pedwidth_max[2]);

    // Sum of a given multiset
    auto sum = [](std::multiset<double> x) { return std::accumulate(x.begin(), x.end(), 0.0); };

    m_pedwidth_avg[0] = sum(fwd_pedwidth) / fwd_pedwidth.size() * adc_to_mev;
    m_pedwidth_avg[1] = sum(barrel_pedwidth) / barrel_pedwidth.size() * adc_to_mev;
    m_pedwidth_avg[2] = sum(bwd_pedwidth) / bwd_pedwidth.size() * adc_to_mev;
  } else {
    for (int i = 0; i < 4; i++) {
      m_pedwidth_max[i] = 0;
      m_pedwidth_avg[i] = 0;
    }
  }

  //== Set EPICS PVs

  if (h_fail_crateid != NULL) {
    // Set fit consistency check PVs
    for (int i = 0; i < c_collector_count; i++) {
      std::string pv_name = (boost::format("logic_check:crate%02d") %
                             (i + 1)).str();
      int errors_count = h_fail_crateid->GetBinContent(i + 1);
      setEpicsPV(pv_name, errors_count);
    }
    // Set pedestal width PVs
    static const char* part_id[] = {"fw", "br", "bw", "al"};
    for (int i = 0; i < 4; i++) {
      if (m_pedwidth_max[i] <= 0) continue;
      std::string pv_name = "pedwidth:max_";
      pv_name += part_id[i];
      setEpicsPV(pv_name, m_pedwidth_max[i]);
    }
    for (int i = 0; i < 4; i++) {
      if (m_pedwidth_avg[i] <= 0) continue;
      std::string pv_name = "pedwidth:avg_";
      pv_name += part_id[i];
      setEpicsPV(pv_name, m_pedwidth_avg[i]);
    }
  }
}

void DQMHistAnalysisECLShapersModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisECLShapers: endRun called");

  //= Set the contents of ECL monitoring object
  m_c_main->Clear(); // clear existing content

  TProfile* h_pedrms_cellid = (TProfile*)findHist("ECL/pedrms_cellid");
  if (h_pedrms_cellid == nullptr) {
    m_monObj->setVariable("comment", "No ECL pedestal width histograms available");
    B2INFO("Histogram named ECL/pedrms_cellid is not found.");
    return;
  }

  m_c_main->cd();
  h_pedrms_cellid->Draw();

  // set values of monitoring variables (if variable already exists this will
  // change its value, otherwise it will insert new variable)
  m_monObj->setVariable("pedwidthmaxFWD", m_pedwidth_max[0]);
  m_monObj->setVariable("pedwidthmaxBarrel", m_pedwidth_max[1]);
  m_monObj->setVariable("pedwidthmaxBWD", m_pedwidth_max[2]);
  m_monObj->setVariable("pedwidthmaxTotal", m_pedwidth_max[3]);
  m_monObj->setVariable("pedwidthavgFWD", m_pedwidth_avg[0]);
  m_monObj->setVariable("pedwidthavgBarrel", m_pedwidth_avg[1]);
  m_monObj->setVariable("pedwidthavgBWD", m_pedwidth_avg[2]);
  m_monObj->setVariable("pedwidthavgTotal", m_pedwidth_avg[3]);
}


double DQMHistAnalysisECLShapersModule::robust_max(std::multiset<double> values)
{
  int len = values.size();
  if (len < 10) {
    return 0;
  }
  // Move end iterator back by 10% to remove
  // noisy values.
  auto end_iter = std::prev(values.end(), len * 0.1);

  return *end_iter;
}

