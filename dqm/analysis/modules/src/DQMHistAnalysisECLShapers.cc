/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//THIS MODULE
#include <dqm/analysis/modules/DQMHistAnalysisECLShapers.h>

using namespace Belle2;

REG_MODULE(DQMHistAnalysisECLShapers)

DQMHistAnalysisECLShapersModule::DQMHistAnalysisECLShapersModule()
  : DQMHistAnalysisModule()
{
  B2DEBUG(20, "DQMHistAnalysisECLShapers: Constructor done.");

  addParam("useEpics", m_useEpics, "Whether to update EPICS PVs.", false);
}


DQMHistAnalysisECLShapersModule::~DQMHistAnalysisECLShapersModule()
{
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (ca_current_context()) ca_context_destroy();
  }
#endif
}

void DQMHistAnalysisECLShapersModule::initialize()
{
  B2DEBUG(20, "DQMHistAnalysisECLShapers: initialized.");

#ifdef _BELLE2_EPICS
  if (m_useEpics) {

    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    for (int i = 0; i < c_collector_count; i++) {
      std::string pv_name = "ECL:logic_check:crate" + std::to_string(i + 1);
      SEVCHK(ca_create_channel(pv_name.c_str(), NULL, NULL, 10, &chid_logic[i]), "ca_create_channel failure");
    }
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

void DQMHistAnalysisECLShapersModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisECLShapers: beginRun called.");
}

void DQMHistAnalysisECLShapersModule::event()
{
  TH1* h_fail_crateid  = findHist("ECL/fail_crateid");
  TH1* h_pedrms_cellid = findHist("ECL/pedrms_cellid");

#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (h_fail_crateid != NULL) {
      for (int i = 0; i < c_collector_count; i++) {
        int errors_count = h_fail_crateid->GetBinContent(i + 1);
        if (chid_logic[i]) SEVCHK(ca_put(DBR_LONG, chid_logic[i], (void*)&errors_count), "ca_set failure");
      }
    }
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

void DQMHistAnalysisECLShapersModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisECLShapers: endRun called");
}


void DQMHistAnalysisECLShapersModule::terminate()
{
  B2DEBUG(20, "terminate called");

#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    for (int i = 0; i < c_collector_count; i++) {
      if (chid_logic[i]) SEVCHK(ca_clear_channel(chid_logic[i]), "ca_clear_channel failure");
    }
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}
