/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisEpicsExample.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisEpicsExample);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisEpicsExampleModule::DQMHistAnalysisEpicsExampleModule()
  : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!
  setDescription("Example module for EPICS");

  //Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("test"));
  addParam("histogramName", m_histogramName, "Name of Histogram", std::string("testHist"));
  addParam("Function", m_function, "Fit function definition", std::string("gaus"));
  addParam("Parameters", m_parameters, "Number of fit function parameters for EPICS", 3);
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("DQM:TEST:"));
  B2DEBUG(20, "DQMHistAnalysisEpicsExample: Constructor done.");
}

void DQMHistAnalysisEpicsExampleModule::initialize()
{
  B2DEBUG(20, "DQMHistAnalysisEpicsExample: initialized.");

  m_c1 = new TCanvas(TString(m_histogramDirectoryName + "_c_" + m_histogramName));
  m_f1 = new TF1(TString(m_histogramDirectoryName + "_f_" + m_histogramName), TString(m_function), -30, 300);
  m_f1->SetParameter(0, 1000);
  m_f1->SetParameter(1, 0);
  m_f1->SetParameter(2, 10);
  m_f1->SetLineColor(4);
  m_f1->SetNpx(512);
  m_f1->SetNumberFitPoints(512);

  m_line = new TLine(0, 10, 0, 0);
  m_line->SetVertical(true);
  m_line->SetLineColor(8);
  m_line->SetLineWidth(3);

  m_line_lo = new TLine(0, 10, 0, 0);
  m_line_lo->SetVertical(true);
  m_line_lo->SetLineColor(2);
  m_line_lo->SetLineWidth(3);

  m_line_hi = new TLine(0, 10, 0, 0);
  m_line_hi->SetVertical(true);
  m_line_hi->SetLineColor(2);
  m_line_hi->SetLineWidth(3);

  m_line_lo->SetX1(5);// get from epics
  m_line_lo->SetX2(5);

  m_line_hi->SetX1(50);// get from epics
  m_line_hi->SetX2(50);

  // need the function to get parameter names
  if (m_parameters > 0) {
    if (m_parameters > 10) m_parameters = 10; // hard limit
    for (auto i = 0; i < m_parameters; i++) {
      std::string aa;
      aa = m_f1->GetParName(i);
      if (aa == "") aa = std::string("par") + std::string(TString::Itoa(i, 10).Data());
      mypv.push_back(aa);
      registerEpicsPV(m_pvPrefix + aa, aa);
      // Read LO and HI limits from EPICS if needed, like
      // requestLimitsFromEpicsPVs("mean", m_meanLowerAlarm, m_meanLowerWarn, m_meanUpperWarn, m_meanUpperAlarm);
    }
  } else {
    m_parameters = 0;
  }
}


void DQMHistAnalysisEpicsExampleModule::beginRun()
{
  //m_serv->SetTimer(100, kFALSE);
  B2DEBUG(20, "DQMHistAnalysisEpicsExample: beginRun called.");
  m_c1->Clear();

  if (auto hh1 = findHist(m_histogramDirectoryName, m_histogramName); hh1 != nullptr) {
    m_c1->cd();
    hh1->Draw();
    m_line->Draw();
    m_line_lo->Draw();
    m_line_hi->Draw();
  } else {
    B2DEBUG(20, "Histo " << m_histogramName << " not found");
  }
}

void DQMHistAnalysisEpicsExampleModule::event()
{
  bool flag = false;

  if (auto hh1 = findHist(m_histogramDirectoryName, m_histogramName); hh1 != nullptr) {
    m_c1->cd();// necessary!
    hh1->Fit(m_f1, "");
    double y1 = hh1->GetMaximum();
    double y2 = hh1->GetMinimum();
    m_line->SetY1(y1 + (y1 - y2) * 0.05);
    m_line_lo->SetY1(y1 + (y1 - y2) * 0.05);
    m_line_hi->SetY1(y1 + (y1 - y2) * 0.05);
//     m_line->SetY2(y2-(y1-y2)*0.05);
//     m_line_lo->SetY2(y2-(y1-y2)*0.05);
//     m_line_hi->SetY2(y2-(y1-y2)*0.05);
    double x = m_f1->GetParameter(1);
    m_line->SetX1(x);
    m_line->SetX2(x);
    if (!flag) {
      // dont add another line...
      m_line->Draw();
      m_line_lo->Draw();
      m_line_hi->Draw();
    }
    m_c1->Modified();
    m_c1->Update();
    UpdateCanvas(m_c1->GetName());
  } else {
    B2DEBUG(20, "Histo " << m_histogramDirectoryName << "/" << m_histogramName << " not found");
  }

  if (m_parameters > 0) {
    for (auto i = 0; i < m_parameters; i++) {
      double data;
      data = m_f1->GetParameter(i);
      setEpicsPV(mypv[i], data);
    }
  }
}

void DQMHistAnalysisEpicsExampleModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisEpicsExample : endRun called");
}


void DQMHistAnalysisEpicsExampleModule::terminate()
{
  B2DEBUG(20, "DQMHistAnalysisEpicsExample: terminate called");
}

