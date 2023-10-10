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

using namespace Belle2;

REG_MODULE(DQMHistAnalysisECLShapers);

DQMHistAnalysisECLShapersModule::DQMHistAnalysisECLShapersModule()
  : DQMHistAnalysisModule()
{
  B2DEBUG(20, "DQMHistAnalysisECLShapers: Constructor done.");
}


DQMHistAnalysisECLShapersModule::~DQMHistAnalysisECLShapersModule()
{
#ifdef _BELLE2_EPICS
  if (getUseEpics()) {
    if (ca_current_context()) ca_context_destroy();
  }
#endif
}

void DQMHistAnalysisECLShapersModule::initialize()
{
#ifdef _BELLE2_EPICS
  if (getUseEpics()) {
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    for (int i = 0; i < c_collector_count; i++) {
      std::string pv_name = "ECL:logic_check:crate" + std::to_string(i + 1);
      SEVCHK(ca_create_channel(pv_name.c_str(), NULL, NULL, 10, &chid_logic[i]), "ca_create_channel failure");
    }
    SEVCHK(ca_create_channel("ECL:pedwidth:test:max_fw", NULL, NULL, 10,
                             &chid_pedwidth[0]), "ca_create_channel failure");
    SEVCHK(ca_create_channel("ECL:pedwidth:test:max_br", NULL, NULL, 10,
                             &chid_pedwidth[1]), "ca_create_channel failure");
    SEVCHK(ca_create_channel("ECL:pedwidth:test:max_bw", NULL, NULL, 10,
                             &chid_pedwidth[2]), "ca_create_channel failure");
    SEVCHK(ca_create_channel("ECL:pedwidth:test:max_al", NULL, NULL, 10,
                             &chid_pedwidth[3]), "ca_create_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif

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
    m_pedwidth_max[3] = *std::max(&m_pedwidth_max[0], &m_pedwidth_max[2]);
  } else {
    for (int i = 0; i < 4; i++) {
      m_pedwidth_max[i] = 0;
    }
  }

  //== Set EPICS PVs

#ifdef _BELLE2_EPICS
  if (getUseEpics()) {
    if (h_fail_crateid != NULL) {
      // Set fit consistency check PVs
      for (int i = 0; i < c_collector_count; i++) {
        int errors_count = h_fail_crateid->GetBinContent(i + 1);
        if (chid_logic[i]) SEVCHK(ca_put(DBR_LONG, chid_logic[i], (void*)&errors_count), "ca_set failure");
      }
      // Set pedestal width PVs
      for (int i = 0; i < 4; i++) {
        if (m_pedwidth_max[i] <= 0) continue;
        if (!chid_pedwidth[i]) continue;
        SEVCHK(ca_put(DBR_DOUBLE, chid_pedwidth[i], (void*)&m_pedwidth_max[i]), "ca_set failure");
      }
    }
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
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
  m_monObj->setVariable("pedwidthFWD", m_pedwidth_max[0]);
  m_monObj->setVariable("pedwidthBarrel", m_pedwidth_max[1]);
  m_monObj->setVariable("pedwidthBWD", m_pedwidth_max[2]);
  m_monObj->setVariable("pedwidthTotal", m_pedwidth_max[3]);
}


void DQMHistAnalysisECLShapersModule::terminate()
{
  B2DEBUG(20, "terminate called");

#ifdef _BELLE2_EPICS
  if (getUseEpics()) {
    for (int i = 0; i < c_collector_count; i++) {
      if (chid_logic[i]) SEVCHK(ca_clear_channel(chid_logic[i]), "ca_clear_channel failure");
    }
    for (int i = 0; i < 4; i++) {
      if (chid_pedwidth[i]) SEVCHK(ca_clear_channel(chid_pedwidth[i]), "ca_clear_channel failure");
    }
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
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

