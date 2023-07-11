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
}


DQMHistAnalysisECLOutOfTimeDigitsModule::~DQMHistAnalysisECLOutOfTimeDigitsModule()
{
#ifdef _BELLE2_EPICS
  if (getUseEpics()) {
    if (ca_current_context()) ca_context_destroy();
  }
#endif
}

void DQMHistAnalysisECLOutOfTimeDigitsModule::initialize()
{
#ifdef _BELLE2_EPICS
  if (getUseEpics()) {
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    // Register EPICS PVs
    for (auto& event_type : {"rand", "dphy", "physics"}) {
      for (auto& ecl_part : {"All", "FWDEndcap", "Barrel", "BWDEndcap"}) {
        std::string key_name = event_type + std::string("_") + ecl_part;
        std::string pv_name = std::string("ECL:out_of_time_digits:") + event_type + ":" + ecl_part;

        // chid_out_of_time_digits[key_name] = 0;
        SEVCHK(ca_create_channel(pv_name.c_str(), NULL, NULL, 10,
                                 &chid_out_of_time_digits[key_name]),
               "ca_create_channel failure");
      }
    }
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif

  m_monObj = getMonitoringObject("ecl");

  m_c_main = new TCanvas("ecl_main");
  m_monObj->addCanvas(m_c_main);

  B2DEBUG(20, "DQMHistAnalysisECLOutOfTimeDigits: initialized.");
}

void DQMHistAnalysisECLOutOfTimeDigitsModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisECLOutOfTimeDigits: beginRun called.");
}

void DQMHistAnalysisECLOutOfTimeDigitsModule::event()
{
  //== Get DQM info
  for (auto& event_type : {"rand", "dphy", "physics"}) {
    for (auto& ecl_part : {"All", "FWDEndcap", "Barrel", "BWDEndcap"}) {
      std::string key_name = event_type + std::string("_") + ecl_part;

      m_out_of_time_digits[key_name] = 0;

      std::string hist_name    = "ECL/out_of_time_" + key_name;
      TProfile* prof = (TProfile*)findHist(hist_name);

      if (!prof) continue;

      m_out_of_time_digits[key_name] = prof->GetBinContent(1);
    }
  }

  //== Set EPICS PVs

#ifdef _BELLE2_EPICS
  if (getUseEpics()) {
    for (auto& event_type : {"rand", "dphy", "physics"}) {
      for (auto& ecl_part : {"All", "FWDEndcap", "Barrel", "BWDEndcap"}) {
        std::string key_name = event_type + std::string("_") + ecl_part;
        chid selected_chid = chid_out_of_time_digits[key_name];
        double selected_value =  m_out_of_time_digits[key_name];
        if (!selected_chid) continue;
        SEVCHK(ca_put(DBR_DOUBLE, selected_chid, (void*)&selected_value), "ca_set failure");
      }
    }
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
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
      std::string key_name = event_type + std::string("_") + ecl_part;
      // set values of monitoring variables (if variable already exists this will
      // change its value, otherwise it will insert new variable)
      m_monObj->setVariable("out_of_time_digits_" + key_name,
                            m_out_of_time_digits[key_name]);
    }
  }

}


void DQMHistAnalysisECLOutOfTimeDigitsModule::terminate()
{
  B2DEBUG(20, "terminate called");

#ifdef _BELLE2_EPICS
  if (getUseEpics()) {
    for (auto& event_type : {"rand", "dphy", "physics"}) {
      for (auto& ecl_part : {"All", "FWDEndcap", "Barrel", "BWDEndcap"}) {
        std::string key_name = event_type + std::string("_") + ecl_part;

        chid selected_chid = chid_out_of_time_digits[key_name];
        if (!selected_chid) continue;
        SEVCHK(ca_clear_channel(selected_chid), "ca_clear_channel failure");
      }
    }
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

