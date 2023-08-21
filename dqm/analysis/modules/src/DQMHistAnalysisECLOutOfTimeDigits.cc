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
#include <TProfile.h>

using namespace Belle2;

REG_MODULE(DQMHistAnalysisECLOutOfTimeDigits);

DQMHistAnalysisECLOutOfTimeDigitsModule::DQMHistAnalysisECLOutOfTimeDigitsModule()
  : DQMHistAnalysisModule()
{
  B2DEBUG(20, "DQMHistAnalysisECLOutOfTimeDigits: Constructor done.");
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
  updateEpicsPVs(5.0);

  m_monObj = getMonitoringObject("ecl");

  B2DEBUG(20, "DQMHistAnalysisECLOutOfTimeDigits: initialized.");
}

void DQMHistAnalysisECLOutOfTimeDigitsModule::event()
{
  //== Get DQM info
  for (auto& event_type : {"rand", "dphy", "physics"}) {
    for (auto& ecl_part : {"All", "FWDEndcap", "Barrel", "BWDEndcap"}) {
      std::string pv_name = event_type + std::string(":") + ecl_part;

      m_out_of_time_digits[pv_name] = 0;

      std::string hist_name    = "ECL/out_of_time_" + pv_name;
      auto prof = (TProfile*)findHist(hist_name, m_onlyIfUpdated);

      if (!prof) continue;

      m_out_of_time_digits[pv_name] = prof->GetBinContent(1);

      //== Set EPICS PVs

      double selected_value = m_out_of_time_digits[pv_name];
      setEpicsPV(pv_name, selected_value);
    }
  }
  updateEpicsPVs(5.0);
}

void DQMHistAnalysisECLOutOfTimeDigitsModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisECLOutOfTimeDigits: endRun called");

  if (findHist("ECL/out_of_time_physics_All") == nullptr) {
    m_monObj->setVariable("comment_out_of_time_digits", "No ECL out-of-time ECLCalDigits histograms available");
    B2INFO("Histogram named ECL/out_of_time_physics_All is not found.");
    return;
  }

  for (auto& event_type : {"rand", "dphy", "physics"}) {
    for (auto& ecl_part : {"All", "FWDEndcap", "Barrel", "BWDEndcap"}) {
      std::string pv_name  = event_type + std::string(":") + ecl_part;
      std::string var_name = "out_of_time_digits_" + pv_name;
      std::replace(var_name.begin(), var_name.end(), ':', '_');
      // set values of monitoring variables (if variable already exists this will
      // change its value, otherwise it will insert new variable)
      m_monObj->setVariable(var_name, m_out_of_time_digits[pv_name]);
    }
  }
}


