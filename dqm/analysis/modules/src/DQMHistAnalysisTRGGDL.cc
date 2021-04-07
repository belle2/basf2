/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kindo Haruki, Luka Santelj                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <dqm/analysis/modules/DQMHistAnalysisTRGGDL.h>

//DQM
#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TClass.h>
#include <TROOT.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(DQMHistAnalysisTRGGDL);

DQMHistAnalysisTRGGDLModule::DQMHistAnalysisTRGGDLModule()
  : DQMHistAnalysisModule()
{
  // set module description (e.g. insert text)
  setDescription("Modify and analyze the data quality histograms of TRGGDL");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("debug", m_debug, "debug mode", false);
  addParam("alert", m_enableAlert, "Enable color alert", true);
}

DQMHistAnalysisTRGGDLModule::~DQMHistAnalysisTRGGDLModule()
{
#ifdef _BELLE2_EPICS
  if (ca_current_context()) ca_context_destroy();
#endif
}

void DQMHistAnalysisTRGGDLModule::initialize()
{
  gROOT->cd();

  for (unsigned iskim = 0; iskim < nskim_gdldqm; iskim++) {
    char charhist_eff[1000];
    sprintf(charhist_eff, "hGDL_ana_eff_%s", skim_smap[iskim].c_str());
    m_h_eff[iskim] = new TH1D(charhist_eff, charhist_eff, n_eff, 0, n_eff);
    for (int i = 0; i < n_eff; i++) {
      m_h_eff[iskim]->GetXaxis()->SetBinLabel(i + 1, c_eff[i]);
    }
    char charhist_c_eff[1000];
    sprintf(charhist_c_eff, "TRGGDL/hGDL_ana_eff_%s", skim_smap[iskim].c_str());
    m_c_eff[iskim] = new TCanvas(charhist_c_eff);
  }

  m_h_eff_shifter = new TH1D("hGDL_ana_eff_shifter", "hGDL_ana_eff_shifter", n_eff_shifter, 0, n_eff_shifter);
  m_h_eff_shifter->GetYaxis()->SetTitle("efficiency");
  for (int i = 0; i < n_eff_shifter; i++) {
    m_h_eff_shifter->GetXaxis()->SetBinLabel(i + 1, c_eff_shifter[i]);
    m_line_limit_low_shifter[i]  = new TLine(i, m_limit_low_shifter[i], i + 1, m_limit_low_shifter[i]);
    m_line_limit_high_shifter[i] = new TLine(i, m_limit_high_shifter[i], i + 1, m_limit_high_shifter[i]);
  }
  m_c_eff_shifter = new TCanvas("TRGGDL/hGDL_ana_eff_shifter");

  m_h_pure_eff = new TH1D("hGDL_ana_pure_eff", "hGDL_ana_pure_eff", n_pure_eff, 0, n_pure_eff);
  for (int i = 0; i < n_pure_eff; i++) {
    m_h_pure_eff->GetXaxis()->SetBinLabel(i + 1, c_pure_eff[i]);
  }
  m_c_pure_eff = new TCanvas("TRGGDL/hGDL_ana_pure_eff");



#ifdef _BELLE2_EPICS
  if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
  for (int i = 0; i < n_eff_shifter; i++) {
    std::string aa = "TRGAna:eff_shift_" + std::to_string(i);
    SEVCHK(ca_create_channel(aa.c_str(), NULL, NULL, 10, &mychid[i]), "ca_create_channel failure");
    // Read LO and HI limits from EPICS, seems this needs additional channels?
    // SEVCHK(ca_get(DBR_DOUBLE,mychid[i],(void*)&data),"ca_get failure"); // data is only valid after ca_pend_io!!
  }
  for (int i = 0; i < nskim_gdldqm; i++) {
    std::string aa = "TRGAna:entry_" + std::to_string(i);
    SEVCHK(ca_create_channel(aa.c_str(), NULL, NULL, 10, &mychid_entry[i]), "ca_create_channel failure");
  }
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif

  B2DEBUG(20, "DQMHistAnalysisTRGGDL: initialized.");
}

void DQMHistAnalysisTRGGDLModule::beginRun()
{
}

void DQMHistAnalysisTRGGDLModule::event()
{

  for (unsigned iskim = 0; iskim < nskim_gdldqm; iskim++) {

    //get histo
    char c_psn[1000];
    sprintf(c_psn, "TRGGDL/hGDL_psn_extra_%s", skim_smap[iskim].c_str());
    m_h_psn_extra[iskim] = (TH1D*)findHist(c_psn);/**psn bits*/

    if (m_h_psn_extra[iskim] == NULL) {
      B2INFO("Histogram/canvas named hGDL_psn_extra is not found.");
      continue;
    }

    //fill efficiency values
    if (m_h_psn_extra[iskim]->GetBinContent(0 + 1) == 0)continue;
    if (m_h_psn_extra[iskim]->GetBinContent(5 + 1) == 0)continue;
    m_h_eff[iskim]->SetBinContent(1,  m_h_psn_extra[iskim]->GetBinContent(1 + 1)  / m_h_psn_extra[iskim]->GetBinContent(
                                    5 + 1)); //fff with c4|hie
    m_h_eff[iskim]->SetBinContent(2,  m_h_psn_extra[iskim]->GetBinContent(2 + 1)  / m_h_psn_extra[iskim]->GetBinContent(
                                    5 + 1)); //ffo with c4|hie
    m_h_eff[iskim]->SetBinContent(3,  m_h_psn_extra[iskim]->GetBinContent(3 + 1)  / m_h_psn_extra[iskim]->GetBinContent(
                                    5 + 1)); //ffb with c4|hie
    m_h_eff[iskim]->SetBinContent(4,  m_h_psn_extra[iskim]->GetBinContent(19 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    5 + 1)); //ffy with c4|hie
    m_h_eff[iskim]->SetBinContent(5,  m_h_psn_extra[iskim]->GetBinContent(20 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    5 + 1)); //fyo with c4|hie
    m_h_eff[iskim]->SetBinContent(6,  m_h_psn_extra[iskim]->GetBinContent(21 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    5 + 1)); //fyb with c4|hie
    m_h_eff[iskim]->SetBinError(1,  sqrt(m_h_psn_extra[iskim]->GetBinContent(1 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  5 + 1)); //fff with c4|hie
    m_h_eff[iskim]->SetBinError(2,  sqrt(m_h_psn_extra[iskim]->GetBinContent(2 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  5 + 1)); //ffo with c4|hie
    m_h_eff[iskim]->SetBinError(3,  sqrt(m_h_psn_extra[iskim]->GetBinContent(3 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  5 + 1)); //ffb with c4|hie
    m_h_eff[iskim]->SetBinError(4,  sqrt(m_h_psn_extra[iskim]->GetBinContent(19 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  5 + 1)); //ffy with c4|hie
    m_h_eff[iskim]->SetBinError(5,  sqrt(m_h_psn_extra[iskim]->GetBinContent(20 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  5 + 1)); //fyo with c4|hie
    m_h_eff[iskim]->SetBinError(6,  sqrt(m_h_psn_extra[iskim]->GetBinContent(21 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  5 + 1)); //fyb with c4|hie

    if (m_h_psn_extra[iskim]->GetBinContent(6 + 1) == 0)continue;
    m_h_eff[iskim]->SetBinContent(7,  m_h_psn_extra[iskim]->GetBinContent(27 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    6 + 1)); //hie with fff|ffo|ffb
    m_h_eff[iskim]->SetBinContent(8,  m_h_psn_extra[iskim]->GetBinContent(26 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    6 + 1)); //c4 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinContent(9,  m_h_psn_extra[iskim]->GetBinContent(28 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    6 + 1)); //lml0 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinContent(10, m_h_psn_extra[iskim]->GetBinContent(29 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    6 + 1)); //lml1 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinContent(11, m_h_psn_extra[iskim]->GetBinContent(30 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    6 + 1)); //lml2 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinContent(12, m_h_psn_extra[iskim]->GetBinContent(31 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    6 + 1)); //lml3 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinContent(13, m_h_psn_extra[iskim]->GetBinContent(32 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    6 + 1)); //lml4 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinContent(14, m_h_psn_extra[iskim]->GetBinContent(33 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    6 + 1)); //lml5 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinContent(15, m_h_psn_extra[iskim]->GetBinContent(34 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    6 + 1)); //lml6 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinContent(16, m_h_psn_extra[iskim]->GetBinContent(35 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    6 + 1)); //lml7 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinContent(17, m_h_psn_extra[iskim]->GetBinContent(36 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    6 + 1)); //lml8 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinContent(18, m_h_psn_extra[iskim]->GetBinContent(37 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    6 + 1)); //lml9 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinContent(19, m_h_psn_extra[iskim]->GetBinContent(38 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    6 + 1)); //lml10 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinContent(20, m_h_psn_extra[iskim]->GetBinContent(39 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    6 + 1)); //lml12 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinContent(21, m_h_psn_extra[iskim]->GetBinContent(40 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    6 + 1)); //lml13 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinContent(22, m_h_psn_extra[iskim]->GetBinContent(9 + 1) /  m_h_psn_extra[iskim]->GetBinContent(
                                    0 + 1)); //bha3d with all
    m_h_eff[iskim]->SetBinContent(23, m_h_psn_extra[iskim]->GetBinContent(42 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    6 + 1)); //mu_b2b with fff|ffo|ffb
    m_h_eff[iskim]->SetBinError(7,  sqrt(m_h_psn_extra[iskim]->GetBinContent(27 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  6 + 1)); //hie with fff|ffo|ffb
    m_h_eff[iskim]->SetBinError(8,  sqrt(m_h_psn_extra[iskim]->GetBinContent(26 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  6 + 1)); //c4 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinError(9,  sqrt(m_h_psn_extra[iskim]->GetBinContent(28 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  6 + 1)); //lml0 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinError(10, sqrt(m_h_psn_extra[iskim]->GetBinContent(29 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  6 + 1)); //lml1 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinError(11, sqrt(m_h_psn_extra[iskim]->GetBinContent(30 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  6 + 1)); //lml2 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinError(12, sqrt(m_h_psn_extra[iskim]->GetBinContent(31 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  6 + 1)); //lml3 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinError(13, sqrt(m_h_psn_extra[iskim]->GetBinContent(32 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  6 + 1)); //lml4 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinError(14, sqrt(m_h_psn_extra[iskim]->GetBinContent(33 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  6 + 1)); //lml5 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinError(15, sqrt(m_h_psn_extra[iskim]->GetBinContent(34 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  6 + 1)); //lml6 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinError(16, sqrt(m_h_psn_extra[iskim]->GetBinContent(35 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  6 + 1)); //lml7 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinError(17, sqrt(m_h_psn_extra[iskim]->GetBinContent(36 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  6 + 1)); //lml8 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinError(18, sqrt(m_h_psn_extra[iskim]->GetBinContent(37 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  6 + 1)); //lml9 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinError(19, sqrt(m_h_psn_extra[iskim]->GetBinContent(38 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  6 + 1)); //lml10 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinError(20, sqrt(m_h_psn_extra[iskim]->GetBinContent(39 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  6 + 1)); //lml12 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinError(21, sqrt(m_h_psn_extra[iskim]->GetBinContent(40 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  6 + 1)); //lml13 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinError(22, sqrt(m_h_psn_extra[iskim]->GetBinContent(9 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  0 + 1));  //bha3d with all
    m_h_eff[iskim]->SetBinError(23, sqrt(m_h_psn_extra[iskim]->GetBinContent(42 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  6 + 1)); //mu_b2b with fff|ffo|ffb

    if (m_h_psn_extra[iskim]->GetBinContent(14 + 1) == 0)continue;
    m_h_eff[iskim]->SetBinContent(24, m_h_psn_extra[iskim]->GetBinContent(48 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    14 + 1)); //mu_b2b with lml|eclmumu
    m_h_eff[iskim]->SetBinContent(25, m_h_psn_extra[iskim]->GetBinContent(49 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    14 + 1)); //mu_eb2b with lml|eclmumu
    m_h_eff[iskim]->SetBinContent(26, m_h_psn_extra[iskim]->GetBinContent(50 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    14 + 1)); //cdcklm1 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinContent(27, m_h_psn_extra[iskim]->GetBinContent(51 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    14 + 1)); //cdcklm2 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinContent(28, m_h_psn_extra[iskim]->GetBinContent(15 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    14 + 1)); //fff with lml|eclmumu
    m_h_eff[iskim]->SetBinContent(29, m_h_psn_extra[iskim]->GetBinContent(16 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    14 + 1)); //ffo with lml|eclmumu
    m_h_eff[iskim]->SetBinContent(30, m_h_psn_extra[iskim]->GetBinContent(17 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    14 + 1)); //ffb with lml|eclmumu
    m_h_eff[iskim]->SetBinContent(31, m_h_psn_extra[iskim]->GetBinContent(11 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    14 + 1)); //ff with lml|eclmumu
    m_h_eff[iskim]->SetBinContent(32, m_h_psn_extra[iskim]->GetBinContent(13 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    14 + 1)); //f with lml|eclmumu
    m_h_eff[iskim]->SetBinContent(33, m_h_psn_extra[iskim]->GetBinContent(23 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    14 + 1)); //ffy with lml|eclmumu
    m_h_eff[iskim]->SetBinContent(34, m_h_psn_extra[iskim]->GetBinContent(24 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    14 + 1)); //fyo with lml|eclmumu
    m_h_eff[iskim]->SetBinContent(35, m_h_psn_extra[iskim]->GetBinContent(25 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    14 + 1)); //fyb with lml|eclmumu
    m_h_eff[iskim]->SetBinError(24, sqrt(m_h_psn_extra[iskim]->GetBinContent(48 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  14 + 1)); //mu_b2b with lml|eclmumu
    m_h_eff[iskim]->SetBinError(25, sqrt(m_h_psn_extra[iskim]->GetBinContent(49 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  14 + 1)); //mu_eb2b with lml|eclmumu
    m_h_eff[iskim]->SetBinError(26, sqrt(m_h_psn_extra[iskim]->GetBinContent(50 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  14 + 1)); //cdcklm1 with lml|eclmumu
    m_h_eff[iskim]->SetBinError(27, sqrt(m_h_psn_extra[iskim]->GetBinContent(51 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  14 + 1)); //cdcklm2 with lml|eclmumu
    m_h_eff[iskim]->SetBinError(28, sqrt(m_h_psn_extra[iskim]->GetBinContent(15 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  14 + 1)); //fff with lml|eclmumu
    m_h_eff[iskim]->SetBinError(29, sqrt(m_h_psn_extra[iskim]->GetBinContent(16 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  14 + 1)); //ffo with lml|eclmumu
    m_h_eff[iskim]->SetBinError(30, sqrt(m_h_psn_extra[iskim]->GetBinContent(17 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  14 + 1)); //ffb with lml|eclmumu
    m_h_eff[iskim]->SetBinError(31, sqrt(m_h_psn_extra[iskim]->GetBinContent(11 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  14 + 1)); //ff with lml|eclmumu
    m_h_eff[iskim]->SetBinError(32, sqrt(m_h_psn_extra[iskim]->GetBinContent(13 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  14 + 1)); //f with lml|eclmumu
    m_h_eff[iskim]->SetBinError(33, sqrt(m_h_psn_extra[iskim]->GetBinContent(23 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  14 + 1)); //ffy with lml|eclmumu
    m_h_eff[iskim]->SetBinError(34, sqrt(m_h_psn_extra[iskim]->GetBinContent(24 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  14 + 1)); //fyo with lml|eclmumu
    m_h_eff[iskim]->SetBinError(35, sqrt(m_h_psn_extra[iskim]->GetBinContent(25 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  14 + 1)); //fyb with lml|eclmumu

    m_h_eff[iskim]->SetBinContent(36, m_h_psn_extra[iskim]->GetBinContent(55 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    6 + 1)); //cdcecl2 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinContent(37, m_h_psn_extra[iskim]->GetBinContent(68 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    14 + 1)); //ecleklm with lml|eclmumu
    m_h_eff[iskim]->SetBinContent(38, m_h_psn_extra[iskim]->GetBinContent(74 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    5 + 1)); //syo with c4|hie
    m_h_eff[iskim]->SetBinContent(39, m_h_psn_extra[iskim]->GetBinContent(75 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    5 + 1)); //yioiecl1 with c4|hie
    m_h_eff[iskim]->SetBinContent(40, m_h_psn_extra[iskim]->GetBinContent(76 + 1) / m_h_psn_extra[iskim]->GetBinContent(
                                    5 + 1)); //stt with c4|hie

    m_h_eff[iskim]->SetBinError(36, sqrt(m_h_psn_extra[iskim]->GetBinContent(55 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  6 + 1)); //cdcecl2 with fff|ffo|ffb
    m_h_eff[iskim]->SetBinError(37, sqrt(m_h_psn_extra[iskim]->GetBinContent(68 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  14 + 1)); //ecleklm with lml|eclmumu
    m_h_eff[iskim]->SetBinError(38, sqrt(m_h_psn_extra[iskim]->GetBinContent(74 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  5 + 1)); //syo with c4|hie
    m_h_eff[iskim]->SetBinError(39, sqrt(m_h_psn_extra[iskim]->GetBinContent(75 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  5 + 1)); //yioiecl1 with c4|hie
    m_h_eff[iskim]->SetBinError(40, sqrt(m_h_psn_extra[iskim]->GetBinContent(76 + 1)) / m_h_psn_extra[iskim]->GetBinContent(
                                  5 + 1)); //stt with c4|hie
  }
  //fill efficiency for shifter
  m_h_eff_shifter->SetBinContent(1,  m_h_eff[9] ->GetBinContent(0 + 1));  //fff with c4|hie
  m_h_eff_shifter->SetBinContent(2,  m_h_eff[9] ->GetBinContent(1 + 1));  //ffo with c4|hie
  m_h_eff_shifter->SetBinContent(3,  m_h_eff[9] ->GetBinContent(3 + 1));  //ffy with c4|hie
  m_h_eff_shifter->SetBinContent(4,  m_h_eff[9] ->GetBinContent(4 + 1));  //fyo with c4|hie
  m_h_eff_shifter->SetBinContent(5,  m_h_eff[9] ->GetBinContent(6 + 1));  //hie with fff|ffo|ffb
  m_h_eff_shifter->SetBinContent(6,  m_h_eff[9] ->GetBinContent(7 + 1));  //c4 with fff|ffo|ffb
  m_h_eff_shifter->SetBinContent(7,  m_h_eff[10]->GetBinContent(23 + 1)); //mu_b2b with lml|eclmumu
  m_h_eff_shifter->SetBinContent(8,  m_h_eff[10]->GetBinContent(24 + 1)); //mu_eb2b with lml|eclmumu
  m_h_eff_shifter->SetBinContent(9,  m_h_eff[10]->GetBinContent(26 + 1)); //cdcklm2 with lml|eclmumu
  m_h_eff_shifter->SetBinContent(10, m_h_eff[10]->GetBinContent(35 + 1)); //cdcecl2 with fff|ffo|ffb
  m_h_eff_shifter->SetBinContent(11, m_h_eff[10]->GetBinContent(36 + 1)); //ecleklm with lml|eclmumu
  m_h_eff_shifter->SetBinContent(12, m_h_eff[9] ->GetBinContent(37 + 1)); //syo with c4|hie
  m_h_eff_shifter->SetBinContent(13, m_h_eff[9] ->GetBinContent(38 + 1)); //yioiecl1 with c4|hie
  m_h_eff_shifter->SetBinContent(14, m_h_eff[9] ->GetBinContent(39 + 1)); //stt with c4|hie
  m_h_eff_shifter->SetBinError(1,    m_h_eff[9] ->GetBinError(0 + 1));  //fff with c4|hie
  m_h_eff_shifter->SetBinError(2,    m_h_eff[9] ->GetBinError(1 + 1));  //ffo with c4|hie
  m_h_eff_shifter->SetBinError(3,    m_h_eff[9] ->GetBinError(3 + 1));  //ffy with c4|hie
  m_h_eff_shifter->SetBinError(4,    m_h_eff[9] ->GetBinError(4 + 1));  //fyo with c4|hie
  m_h_eff_shifter->SetBinError(5,    m_h_eff[9] ->GetBinError(6 + 1));  //hie with fff|ffo|ffb
  m_h_eff_shifter->SetBinError(6,    m_h_eff[9] ->GetBinError(7 + 1));  //c4 with fff|ffo|ffb
  m_h_eff_shifter->SetBinError(7,    m_h_eff[10]->GetBinError(23 + 1)); //mu_b2b with lml|eclmumu
  m_h_eff_shifter->SetBinError(8,    m_h_eff[10]->GetBinError(24 + 1)); //mu_eb2b with lml|eclmumu
  m_h_eff_shifter->SetBinError(9,    m_h_eff[10]->GetBinError(26 + 1)); //cdcklm2 with lml|eclmumu
  m_h_eff_shifter->SetBinError(10,   m_h_eff[10]->GetBinError(35 + 1)); //cdcecl2 with fff|ffo|ffb
  m_h_eff_shifter->SetBinError(11,   m_h_eff[10]->GetBinError(36 + 1)); //ecleklm with lml|eclmumu
  m_h_eff_shifter->SetBinError(12,   m_h_eff[9] ->GetBinError(37 + 1)); //syo with c4|hie
  m_h_eff_shifter->SetBinError(13,   m_h_eff[9] ->GetBinError(38 + 1)); //yioiecl1 with c4|hie
  m_h_eff_shifter->SetBinError(14,   m_h_eff[9] ->GetBinError(39 + 1)); //stt with c4|hie


  //fill efficiency with offline selection
  m_h_psn_pure_extra = (TH1D*)findHist("TRGGDL/hGDL_psn_pure_extra_all");/**psn bits*/
  if (m_h_psn_pure_extra == NULL) {
    B2INFO("Histogram/canvas named hGDL_psn_pure_extra is not found.");
  } else {
    if (m_h_psn_pure_extra->GetBinContent(0 + 1) != 0)
      m_h_pure_eff->SetBinContent(1,  m_h_psn_pure_extra->GetBinContent(1 + 1) / m_h_psn_pure_extra->GetBinContent(
                                    0 + 1)); //fff with c4|hie
    if (m_h_psn_pure_extra->GetBinContent(3 + 1) != 0)
      m_h_pure_eff->SetBinContent(2,  m_h_psn_pure_extra->GetBinContent(4 + 1) / m_h_psn_pure_extra->GetBinContent(
                                    3 + 1)); //ffo with c4|hie
    if (m_h_psn_pure_extra->GetBinContent(6 + 1) != 0)
      m_h_pure_eff->SetBinContent(3,  m_h_psn_pure_extra->GetBinContent(7 + 1) / m_h_psn_pure_extra->GetBinContent(
                                    6 + 1)); //ffb with c4|hie
    if (m_h_psn_pure_extra->GetBinContent(0 + 1) != 0)
      m_h_pure_eff->SetBinContent(4,  m_h_psn_pure_extra->GetBinContent(2 + 1) / m_h_psn_pure_extra->GetBinContent(
                                    0 + 1)); //ffy with c4|hie
    if (m_h_psn_pure_extra->GetBinContent(3 + 1) != 0)
      m_h_pure_eff->SetBinContent(5,  m_h_psn_pure_extra->GetBinContent(5 + 1) / m_h_psn_pure_extra->GetBinContent(
                                    3 + 1)); //fyo with c4|hie
    if (m_h_psn_pure_extra->GetBinContent(6 + 1) != 0)
      m_h_pure_eff->SetBinContent(6,  m_h_psn_pure_extra->GetBinContent(8 + 1) / m_h_psn_pure_extra->GetBinContent(
                                    6 + 1)); //fyb with c4|hie
    if (m_h_psn_pure_extra->GetBinContent(9 + 1) != 0)
      m_h_pure_eff->SetBinContent(7,  m_h_psn_pure_extra->GetBinContent(10 + 1) / m_h_psn_pure_extra->GetBinContent(
                                    9 + 1)); //hie with fff|ffo|ffb
    if (m_h_psn_pure_extra->GetBinContent(11 + 1) != 0)
      m_h_pure_eff->SetBinContent(8,  m_h_psn_pure_extra->GetBinContent(12 + 1) / m_h_psn_pure_extra->GetBinContent(
                                    11 + 1)); //hie with fff|ffo|ffb
  }

  for (unsigned iskim = 0; iskim < nskim_gdldqm; iskim++) {
    m_c_eff[iskim]->Clear();
    m_c_eff[iskim]->cd();
    m_h_eff[iskim]->SetMinimum(0);
    m_h_eff[iskim]->Draw();
    m_c_eff[iskim]->Update();
  }

  m_c_eff_shifter->Clear();
  m_c_eff_shifter->cd();
  m_c_eff_shifter->SetFillColor(0);
  m_h_eff_shifter->SetMaximum(1.1);
  m_h_eff_shifter->SetMinimum(0);
  m_h_eff_shifter->SetLineWidth(2);
  for (int i = 0; i < n_eff_shifter; i++) {
    double eff = m_h_eff_shifter->GetBinContent(i + 1);
    double err = m_h_eff_shifter->GetBinError(i + 1);
    double eff_err_min = eff - 2 * err;
    double eff_err_max = eff + 2 * err;
    if ((eff_err_max < m_limit_low_shifter[i]) || (eff_err_min > m_limit_high_shifter[i])) m_c_eff_shifter->SetFillColor(2);
  }
  m_h_eff_shifter->Draw();
  for (int i = 0; i < n_eff_shifter; i++) {
    m_line_limit_low_shifter[i]-> SetLineColor(1);
    m_line_limit_low_shifter[i]-> SetLineStyle(2);
    m_line_limit_low_shifter[i]-> SetLineWidth(2);
    m_line_limit_low_shifter[i]-> Draw("same");
    m_line_limit_high_shifter[i]->SetLineColor(1);
    m_line_limit_high_shifter[i]->SetLineStyle(2);
    m_line_limit_high_shifter[i]-> SetLineWidth(2);
    m_line_limit_high_shifter[i]->Draw("same");
  }
  m_c_eff_shifter->Update();

  m_c_pure_eff->Clear();
  m_c_pure_eff->cd();
  m_h_pure_eff->SetMinimum(0);
  m_h_pure_eff->Draw();
  m_c_pure_eff->Update();


#ifdef _BELLE2_EPICS
  for (auto i = 0; i < n_eff_shifter; i++) {
    double data;
    data = m_h_eff_shifter->GetBinContent(i + 1);
    if (mychid[i]) SEVCHK(ca_put(DBR_DOUBLE, mychid[i], (void*)&data), "ca_set failure");
  }
  for (auto i = 0; i < nskim_gdldqm; i++) {
    double data = 0;
    if (i == 0)data = m_h_psn_extra[i]->GetBinContent(0 + 1);
    if (mychid_entry[i]) SEVCHK(ca_put(DBR_DOUBLE, mychid_entry[i], (void*)&data), "ca_set failure");
  }
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif

}

void DQMHistAnalysisTRGGDLModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisTRGGDL : endRun called");
}

void DQMHistAnalysisTRGGDLModule::terminate()
{
#ifdef _BELLE2_EPICS
  for (auto i = 0; i < n_eff_shifter; i++) {
    if (mychid[i]) SEVCHK(ca_clear_channel(mychid[i]), "ca_clear_channel failure");
  }
  for (auto i = 0; i < nskim_gdldqm; i++) {
    if (mychid_entry[i]) SEVCHK(ca_clear_channel(mychid_entry[i]), "ca_clear_channel failure");
  }
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif
  B2DEBUG(20, "terminate called");
}

TCanvas* DQMHistAnalysisTRGGDLModule::find_canvas(TString canvas_name)
{
  TIter nextckey(gROOT->GetListOfCanvases());
  TObject* cobj = NULL;

  while ((cobj = (TObject*)nextckey())) {
    if (cobj->IsA()->InheritsFrom("TCanvas")) {
      if (cobj->GetName() == canvas_name)
        break;
    }
  }
  return (TCanvas*)cobj;
}

