/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisSVDClustersOnTrack.cc
// Description : module for DQM histogram analysis of SVD sensors occupancies
//-


#include <dqm/analysis/modules/DQMHistAnalysisSVD.h>
#include <vxd/geometry/GeoCache.h>

#include <TROOT.h>
#include <TStyle.h>
#include <TString.h>
#include <TAxis.h>

#include <TMath.h>
#include <iostream>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisSVD);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisSVDModule:: DQMHistAnalysisSVDModule(bool panelTop, bool online)
  : DQMHistAnalysisModule()
{
  //Parameter definition
  B2DEBUG(10, "DQMHistAnalysisSVD: Constructor done.");

  setDescription("DQM base SVD Analysis.");

  float x1 = 0;
  float x2 = 0;
  float y1 = 0;
  float y2 = 0;

  // cluster time on tracks legend
  if (panelTop) {
    x1 = 0.15;
    y1 = 0.65;
    x2 = 0.35;
    y2 = 0.80;
  } else {
    x1 = 0.62;
    y1 = 0.22;
    x2 = 0.88;
    y2 = 0.35;
  }

  m_legProblem = new TPaveText(x1, y1, x2, y2, "brNDC");
  m_legProblem->SetFillColor(c_ColorDefault);
  m_legProblem->SetTextColor(kBlack);

  m_legWarning = new TPaveText(x1, y1, x2, y2, "brNDC");
  m_legWarning->SetFillColor(c_ColorDefault);
  m_legWarning->SetTextColor(kBlack);

  m_legNormal = new TPaveText(x1, y1, x2, y2, "brNDC");
  m_legNormal->SetFillColor(c_ColorDefault);
  m_legNormal->SetTextColor(kBlack);

  m_legEmpty = new TPaveText(x1, y1, x2, y2, "brNDC");
  m_legEmpty->SetFillColor(c_ColorDefault);
  m_legEmpty->SetTextColor(kBlack);

  if (online) {
    m_legOnlineProblem = new TPaveText(x1, y1, x2, y2, "brNDC");
    m_legOnlineProblem->SetFillColor(c_ColorDefault);
    m_legOnlineProblem->SetTextColor(kBlack);

    m_legOnlineWarning = new TPaveText(x1, y1, x2, y2, "brNDC");
    m_legOnlineWarning->SetFillColor(c_ColorDefault);
    m_legOnlineWarning->SetTextColor(kBlack);

    m_legOnlineNormal = new TPaveText(x1, y1, x2, y2, "brNDC");
    m_legOnlineNormal->SetFillColor(c_ColorDefault);
    m_legOnlineNormal->SetTextColor(kBlack);
  }
}

DQMHistAnalysisSVDModule::~ DQMHistAnalysisSVDModule()
{
  delete m_legProblem;
  delete m_legNormal;
  delete m_legWarning;
  delete m_legEmpty;

  if (m_legOnlineProblem) delete m_legOnlineProblem;
  if (m_legOnlineNormal)  delete m_legOnlineNormal;
  if (m_legOnlineWarning) delete m_legOnlineWarning;
}

void  DQMHistAnalysisSVDModule::setStatusOfCanvas(int status, TCanvas* canvas, bool plotLeg, bool online)
{
  switch (status) {
    case good: {
      colorizeCanvas(canvas, c_StatusGood);
      if (plotLeg) {
        if (online)
          m_legOnlineNormal->Draw();
        else
          m_legNormal->Draw();
      }
      break;
    }
    case warning: {
      colorizeCanvas(canvas, c_StatusWarning);
      if (plotLeg) {
        if (online)
          m_legOnlineWarning->Draw();
        else
          m_legWarning->Draw();
      }
      break;
    }
    case error: {
      colorizeCanvas(canvas, c_StatusError);
      if (plotLeg) {
        if (online)
          m_legOnlineProblem->Draw();
        else
          m_legProblem->Draw();
      }
      break;
    }
    case lowStat: {
      colorizeCanvas(canvas, c_StatusTooFew);
      if (plotLeg) m_legEmpty->Draw();
      break;
    }
    case noStat: {
      colorizeCanvas(canvas, c_StatusDefault);
      break;
    }
    default: {
      B2INFO("efficiency status not set properly: " << status);
      break;
    }
  }

  canvas->Modified();
  canvas->Update();
}
