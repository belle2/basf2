/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//THIS MODULE
#include <dqm/analysis/modules/DQMHistAnalysisECLOutOfTimeDigits.h>

//ROOT
#include <TF1.h>
#include <TH1F.h>

using namespace Belle2;

REG_MODULE(DQMHistAnalysisECLOutOfTimeDigits);

DQMHistAnalysisECLOutOfTimeDigitsModule::DQMHistAnalysisECLOutOfTimeDigitsModule()
  : DQMHistAnalysisModule()
{
  B2DEBUG(20, "DQMHistAnalysisECLOutOfTimeDigits: Constructor done.");
  setDescription("Module to collect and process 'out of time' ECL digits");
  addParam("pvPrefix", m_pvPrefix, "Prefix to use for PVs registered by this module",
           std::string("ECL:out_of_time_digits:"));
  addParam("onlyIfUpdated", m_onlyIfUpdated, "If true (default), update EPICS PVs only if there histograms were updated.",
           true);
}


void DQMHistAnalysisECLOutOfTimeDigitsModule::initialize()
{
  // Register EPICS PVs
  for (auto& event_type : {"rand", "dphy", "physics"}) {
    for (auto& ecl_part : {"All", "FWDEndcap", "Barrel", "BWDEndcap"}) {
      std::string pv_name  = event_type + std::string(":") + ecl_part;
      registerEpicsPV(m_pvPrefix + pv_name, pv_name);
    }
  }

  m_monObj = getMonitoringObject("ecl");

  B2DEBUG(20, "DQMHistAnalysisECLOutOfTimeDigits: initialized.");
}

void DQMHistAnalysisECLOutOfTimeDigitsModule::event()
{
  //== Get DQM info
  for (auto& event_type : {"rand", "dphy", "physics"}) {
    for (auto& ecl_part : {"All", "FWDEndcap", "Barrel", "BWDEndcap"}) {
      std::string pv_name = event_type + std::string(":") + ecl_part;
      std::string var_name = pv_name;
      std::replace(var_name.begin(), var_name.end(), ':', '_');

      m_out_of_time_digits[pv_name] = 0;

      std::string hist_name    = "ECL/out_of_time_" + var_name;
      auto hist = (TH1F*)findHist(hist_name, m_onlyIfUpdated);

      if (!hist) continue;

      m_out_of_time_digits[pv_name] = hist->GetMean();

      //== Set EPICS PVs

      double selected_value = m_out_of_time_digits[pv_name];
      setEpicsPV(pv_name, selected_value);
    }
  }
}

void DQMHistAnalysisECLOutOfTimeDigitsModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisECLOutOfTimeDigits: endRun called");

  auto main_hist = (TH1F*)findHist("ECL/out_of_time_physics_All");

  if (main_hist == nullptr) {
    m_monObj->setVariable("comment_out_of_time_digits", "No ECL out-of-time ECLCalDigits histograms available");
    B2INFO("Histogram named ECL/out_of_time_physics_All is not found.");
    return;
  }

  TF1 gaus("fit_func", "gaus");

  for (auto& event_type : {"rand", "dphy", "physics"}) {
    for (auto& ecl_part : {"All", "FWDEndcap", "Barrel", "BWDEndcap"}) {
      std::string pv_name   = event_type + std::string(":") + ecl_part;
      std::string hist_name = "ECL/out_of_time_" + pv_name;
      std::string var_name  = "out_of_time_digits_" + pv_name;

      std::replace(hist_name.begin(), hist_name.end(), ':', '_');
      std::replace(var_name.begin(), var_name.end(), ':', '_');

      // If enough statistics, obtain more detailed information for MiraBelle
      auto hist = (TH1F*)findHist(hist_name);
      if (hist && hist->GetEntries() > 1000) {
        // Fit the histogram to get the peak of a distribution
        hist->Fit(&gaus);
        m_monObj->setVariable(var_name, gaus.GetParameter(1));
        m_monObj->setVariable(var_name + "_stddev", gaus.GetParameter(2));
      } else {
        // Use simple mean from the histogram
        m_monObj->setVariable(var_name, m_out_of_time_digits[pv_name]);
        m_monObj->setVariable(var_name + "_stddev", 0);
      }
    }
  }
}


