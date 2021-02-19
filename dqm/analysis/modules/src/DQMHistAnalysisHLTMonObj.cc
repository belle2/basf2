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
  TH1* h_l1 = findHist("softwaretrigger_before_filter/hlt_unit_number");

  double n_hlt = 0.;
  if (h_hlt) n_hlt = (double)h_hlt->GetBinContent((h_hlt->GetXaxis())->FindFixBin("total_result"));
  m_monObj->setVariable("n_hlt", n_hlt);
  double n_l1 = 0.;
  if (h_l1) n_l1 = h_l1->GetEntries();
  m_monObj->setVariable("n_l1", n_l1);

  if (h_skim) {
    // loop bins, add variable to monObj named as "effCS_" + bin label w/o "accept"
    for (int ibin = 1; ibin < h_skim->GetXaxis()->GetNbins() + 1; ibin++) {
      double nentr = (double)h_skim->GetBinContent(ibin);
      std::string bin_name(h_skim->GetXaxis()->GetBinLabel(ibin));
      m_monObj->setVariable(bin_name.replace(0, 6, "effCS"), nentr);
    }
  }

  double bgt = 0.;
  if (h_budget) bgt = h_budget->GetMean();
  m_monObj->setVariable("budget_time", bgt);

  m_monObj->setVariable("n_l1_x_budget_time ", n_l1 * bgt);

  double procTime = 0.;
  if (h_processing) procTime = h_processing->GetMean();
  m_monObj->setVariable("processing_time", procTime);

  TH1* h_budgetUnit = nullptr;

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
  }

  B2DEBUG(20, "DQMHistAnalysisHLTMonObj : endRun called");
}

void DQMHistAnalysisHLTMonObjModule::terminate()
{
  B2DEBUG(20, "terminate called");
}
