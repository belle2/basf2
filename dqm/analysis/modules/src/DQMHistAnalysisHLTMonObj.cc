/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <dqm/analysis/modules/DQMHistAnalysisHLTMonObj.h>
// software trigger include
#include <hlt/softwaretrigger/modules/dqm/SoftwareTriggerHLTDQMModule.h>
//DQM
#include <dqm/analysis/modules/DQMHistAnalysis.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(DQMHistAnalysisHLTMonObj);

DQMHistAnalysisHLTMonObjModule::DQMHistAnalysisHLTMonObjModule()
  : DQMHistAnalysisModule()
{
  setDescription("Produces MonitoringObject for the HLT from the available DQM histograms");
  setPropertyFlags(c_ParallelProcessingCertified);
}

DQMHistAnalysisHLTMonObjModule::~DQMHistAnalysisHLTMonObjModule()
{
}

void DQMHistAnalysisHLTMonObjModule::initialize()
{
  // make monitoring object related to this module
  // if monitoring object already exists this will return pointer to it
  m_monObj = getMonitoringObject("hlt");

  // make canvases to be added to MonitoringObject
  m_c_filter = new TCanvas("Filter", "filter", 750, 400);
  m_c_skim = new TCanvas("Skim" , "skim", 400, 400);
  m_c_hardware = new TCanvas("Hardware", "hardware", 1000, 1000);
  m_c_l1 = new TCanvas("L1", "l1", 750, 400);

  // add canvases to MonitoringObject
  m_monObj->addCanvas(m_c_filter);
  m_monObj->addCanvas(m_c_skim);
  m_monObj->addCanvas(m_c_hardware);
  m_monObj->addCanvas(m_c_l1);

}

void DQMHistAnalysisHLTMonObjModule::beginRun()
{
}

void DQMHistAnalysisHLTMonObjModule::event()
{
  // can put the analysis code here or in endRun() function
  // for the start tests we will store output only end of run so better to put code there
}

void DQMHistAnalysisHLTMonObjModule::endRun()
{

  // get existing histograms produced by DQM modules
  TH1* h_hlt = findHist("softwaretrigger/total_result");
  TH1* h_skim = findHist("softwaretrigger/skim");
  TH1* h_budget = findHist("timing_statistics/fullTimeHistogram");
  TH1* h_processing = findHist("timing_statistics/processingTimeHistogram");
  TH1* h_meantime = findHist("timing_statistics/meanTimeHistogram");
  TH1* h_budg_unit = findHist("timing_statistics/fullTimeMeanPerUnitHistogram");
  TH1* h_proc_unit = findHist("timing_statistics/processingTimeMeanPerUnitHistogram");
  TH1* h_procs = findHist("timing_statistics/processesPerUnitHistogram");
  TH1* h_l1 = findHist("softwaretrigger_before_filter/hlt_unit_number");
  TH1* h_err_flag = findHist("softwaretrigger_before_filter/error_flag");
  TH1* h_hlt_triggers = findHist("softwaretrigger/filter");
  TH1* h_l1_triggers = findHist("TRGGDL/hGDL_psn_all");
  TH1* h_l1_triggers_filt = findHist("softwaretrigger/l1_total_result");
  TH1* h_l1_cat_w_overlap = findHist("TRGGDL/hGDL_psn_raw_rate_all");
  TH1* h_l1_cat_wo_overlap = findHist("TRGGDL/hGDL_psn_effect_to_l1_all");
  TH1* h_full_mem = findHist("timing_statistics/fullMemoryHistogram");

  // set the content of filter canvas
  m_c_filter->Clear(); // clear existing content
  m_c_filter->Divide(2, 2);
  m_c_filter->cd(1);
  if (h_hlt) h_hlt->Draw();
  m_c_filter->cd(2);
  if (h_hlt_triggers) h_hlt_triggers->Draw();
  m_c_filter->cd(3);
  if (h_err_flag) h_err_flag->Draw();

  // set the content of skim canvas
  m_c_skim->Clear(); // clear existing content
  m_c_skim->cd();
  if (h_skim) h_skim->Draw();

  // set the content of hardware canvas
  m_c_hardware->Clear(); // clear existing content
  m_c_hardware->Divide(3, 3);
  m_c_hardware->cd(1);
  if (h_l1) h_l1->Draw();
  m_c_hardware->cd(2);
  if (h_budget) h_budget->Draw();
  m_c_hardware->cd(3);
  if (h_processing) h_processing->Draw();
  m_c_hardware->cd(4);
  if (h_budg_unit) h_budg_unit->Draw();
  m_c_hardware->cd(5);
  if (h_proc_unit) h_proc_unit->Draw();
  m_c_hardware->cd(6);
  if (h_meantime) h_meantime->Draw();
  m_c_hardware->cd(7);
  if (h_procs) h_procs->Draw();
  m_c_hardware->cd(8);
  if (h_full_mem) h_full_mem->Draw();

  // set the content of L1 canvas
  m_c_l1->Clear(); // clear existing content
  m_c_l1->Divide(2, 2);
  m_c_l1->cd(1);
  if (h_l1_triggers) h_l1_triggers->Draw();
  m_c_l1->cd(2);
  if (h_l1_triggers_filt) h_l1_triggers_filt->Draw();
  m_c_l1->cd(3);
  if (h_l1_cat_w_overlap) h_l1_cat_w_overlap->Draw();
  m_c_l1->cd(4);
  if (h_l1_cat_wo_overlap) h_l1_cat_wo_overlap->Draw();

  double n_hlt = 0.;
  if (h_hlt) n_hlt = (double)h_hlt->GetBinContent((h_hlt->GetXaxis())->FindFixBin("total_result"));
  m_monObj->setVariable("n_hlt", n_hlt);
  double n_l1 = 0.;
  if (h_l1) n_l1 = h_l1->GetEntries();
  m_monObj->setVariable("n_l1", n_l1);
  double n_procs = 0.;
  if (h_procs) n_procs = h_procs->GetEntries();
  m_monObj->setVariable("n_procs", n_procs);

  if (h_skim) {
    // loop bins, add variable to monObj named as "effCS_" + bin label w/o "accept"
    for (int ibin = 1; ibin < h_skim->GetXaxis()->GetNbins() + 1; ibin++) {
      double nentr = (double)h_skim->GetBinContent(ibin);
      std::string bin_name(h_skim->GetXaxis()->GetBinLabel(ibin));
      m_monObj->setVariable(bin_name.replace(0, 6, "effCS"), nentr);
    }
  }

  if (h_l1_triggers) {
    // loop bins, add variable to monObj named as "effCS_l1_" + bin label
    for (int ibin = 1; ibin < h_l1_triggers->GetXaxis()->GetNbins() + 1; ibin++) {
      double nentr = (double)h_l1_triggers->GetBinContent(ibin);
      std::string bin_name(h_l1_triggers->GetXaxis()->GetBinLabel(ibin));
      if (bin_name == "") continue;
      m_monObj->setVariable(bin_name.insert(0, "effCS_l1_"), nentr);
    }
  }

  if (h_l1_triggers_filt) {
    // loop bins, add variable to monObj named as "effCS_l1_fON_" + bin label
    for (int ibin = 1; ibin < h_l1_triggers_filt->GetXaxis()->GetNbins() + 1; ibin++) {
      double nentr = (double)h_l1_triggers_filt->GetBinContent(ibin);
      std::string bin_name(h_l1_triggers_filt->GetXaxis()->GetBinLabel(ibin));
      if (bin_name == "") continue;
      m_monObj->setVariable(bin_name.insert(0, "effCS_l1_fON_"), nentr);
    }
  }

  if (h_hlt_triggers) {
    // loop bins, add variable to monObj named as "effCS_hlt_" + bin label
    for (int ibin = 1; ibin < h_hlt_triggers->GetXaxis()->GetNbins() + 1; ibin++) {
      double nentr = (double)h_hlt_triggers->GetBinContent(ibin);
      std::string bin_name(h_hlt_triggers->GetXaxis()->GetBinLabel(ibin));
      bin_name = std::regex_replace(bin_name, std::regex("=="), "_eq_");
      bin_name = std::regex_replace(bin_name, std::regex("\\."), "_");
      m_monObj->setVariable(bin_name.insert(0, "effCS_hlt_"), nentr);
    }
  }

  if (h_meantime) {
    // loop bins, add variable to monObj named as "secTime_" + bin label
    for (int ibin = 1; ibin < h_meantime->GetXaxis()->GetNbins() + 1; ibin++) {
      double nentr = (double)h_meantime->GetBinContent(ibin);
      std::string bin_name(h_meantime->GetXaxis()->GetBinLabel(ibin));
      m_monObj->setVariable(bin_name.insert(0, "secTime_"), nentr);
    }
  }

  if (h_err_flag) {
    // loop bins, add variable to monObj named as "errFlag_" + bin label
    for (int ibin = 1; ibin < h_err_flag->GetXaxis()->GetNbins() + 1; ibin++) {
      double nentr = (double)h_err_flag->GetBinContent(ibin);
      std::string bin_name(h_err_flag->GetXaxis()->GetBinLabel(ibin));
      m_monObj->setVariable(bin_name.insert(0, "errFlag_"), nentr);
    }
  }

  if (h_l1_cat_w_overlap) {
    // loop bins, add variable to monObj named as "l1_Ov_" + bin label
    for (int ibin = 1; ibin < h_l1_cat_w_overlap->GetXaxis()->GetNbins() + 1; ibin++) {
      double nentr = (double)h_l1_cat_w_overlap->GetBinContent(ibin);
      std::string bin_name(h_l1_cat_w_overlap->GetXaxis()->GetBinLabel(ibin));
      m_monObj->setVariable(bin_name.insert(0, "l1_Ov_"), nentr);
    }
  }

  if (h_l1_cat_wo_overlap) {
    // loop bins, add variable to monObj named as "l1_noOv_" + bin label
    for (int ibin = 1; ibin < h_l1_cat_wo_overlap->GetXaxis()->GetNbins() + 1; ibin++) {
      double nentr = (double)h_l1_cat_wo_overlap->GetBinContent(ibin);
      std::string bin_name(h_l1_cat_wo_overlap->GetXaxis()->GetBinLabel(ibin));
      m_monObj->setVariable(bin_name.insert(0, "l1_noOv_"), nentr);
    }
  }

  double bgt = 0.;
  if (h_budget) bgt = h_budget->GetMean();
  m_monObj->setVariable("budget_time", bgt);

  m_monObj->setVariable("n_l1_x_budget_time", n_l1 * bgt);

  double procTime = 0.;
  if (h_processing) procTime = h_processing->GetMean();
  m_monObj->setVariable("processing_time", procTime);

  double fullMemory = 0.;
  if (h_full_mem) fullMemory = h_full_mem->GetBinLowEdge(h_full_mem->FindLastBinAbove(0) + 1);
  m_monObj->setVariable("full_memory", fullMemory);

  TH1* h_budgetUnit = nullptr;
  TH1* h_memoryUnit = nullptr;

  for (unsigned int index = 1; index <= SoftwareTrigger::HLTUnit::max_hlt_units; index++) {
    // add budget time per unit
    h_budgetUnit = findHist(("timing_statistics/fullTimePerUnitHistogram_HLT" + std::to_string(index)).c_str());
    double bgunit = 0.;
    if (h_budgetUnit) bgunit = h_budgetUnit->GetMean();
    m_monObj->setVariable(("budget_time_HLT" + std::to_string(index)).c_str(), bgunit);
    // add processing time per unit
    h_budgetUnit = findHist(("timing_statistics/processingTimePerUnitHistogram_HLT" + std::to_string(index)).c_str());
    if (h_budgetUnit) bgunit = h_budgetUnit->GetMean();
    else bgunit = 0.;
    m_monObj->setVariable(("processing_time_HLT" + std::to_string(index)).c_str(), bgunit);
    // add memory per unit
    h_memoryUnit = findHist(("timing_statistics/fullMemoryPerUnitHistogram_HLT" + std::to_string(index)).c_str());
    double memunit = 0.;
    if (h_memoryUnit && bgunit > 0) memunit = h_memoryUnit->GetBinLowEdge(h_memoryUnit->FindLastBinAbove(0.) + 1);
    m_monObj->setVariable(("memory_HLT" + std::to_string(index)).c_str(), memunit);
  }

  B2DEBUG(20, "DQMHistAnalysisHLTMonObj : endRun called");
}

void DQMHistAnalysisHLTMonObjModule::terminate()
{
  B2DEBUG(20, "terminate called");
}
