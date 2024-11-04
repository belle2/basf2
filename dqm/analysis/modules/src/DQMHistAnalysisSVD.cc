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

  addParam("setColzRange", m_setColzRange,
           "If true you can set the range of the histogram in Z with 'ColzMax' and 'ColzMin' parameters.",
           bool(false));
  addParam("ColzMin", m_colzMinimum, "Minimum of Colz histogram", int(0));
  addParam("ColzMax", m_colzMaximum, "Maximum of Colz histogram", int(-1111)); //-1111 set the maximum depending on the content

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

  m_legLowStat = new TPaveText(x1, y1, x2, y2, "brNDC");
  m_legLowStat->SetFillColor(c_ColorDefault);
  m_legLowStat->SetTextColor(kBlack);

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

  // text module numbers
  pair<vector<TText*>, vector<TText*>> moduleNumbers = textModuleNumbers();
  m_laddersText = moduleNumbers.first;
  m_sensorsText = moduleNumbers.second;

  // axes
  m_ly = new TLine(0, 0, 0, 210);
  m_ly->SetLineStyle(kDashed);
  m_ly->SetLineWidth(2);

  m_lx = new TLine(0, 0, 210, 0);
  m_lx->SetLineStyle(kDashed);
  m_lx->SetLineWidth(2);

  m_arrowy = new TArrow(0, 0, 0, 10, 0.01, "|>");
  m_arrowy->SetAngle(40);
  m_arrowy->SetFillColor(1);
  m_arrowy->SetLineWidth(2);

  m_arrowx = new TArrow(0, 0, 10, 0, 0.01, "|>");
  m_arrowx->SetAngle(40);
  m_arrowx->SetFillColor(1);
  m_arrowx->SetLineWidth(2);
}

DQMHistAnalysisSVDModule::~ DQMHistAnalysisSVDModule()
{
  delete m_legProblem;
  delete m_legNormal;
  delete m_legWarning;
  delete m_legLowStat;
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
      if (plotLeg) m_legLowStat->Draw();
      break;
    }
    case noStat: {
      colorizeCanvas(canvas, c_StatusDefault);
      if (plotLeg) m_legEmpty->Draw();
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

// This function is used both for efficiency and occupancy (flag online used onlhy for occupancy)
void DQMHistAnalysisSVDModule::updateCanvases(SVDSummaryPlots* histo, TCanvas* canvas, TCanvas* canvasRPhi, svdStatus status,
                                              bool isU, bool online)
{
  canvas->Draw();
  canvas->cd();
  if (histo) {
    if (!m_setColzRange && m_valueMinimum > 0) histo->setMinimum(m_valueMinimum * 99.9);
    histo->getHistogram(isU)->Draw("text colz");
  }
  setStatusOfCanvas(status, canvas, true, online);

  canvas->Modified();
  canvas->Update();

  canvasRPhi->Draw();
  canvasRPhi->cd();
  if (histo) {
    if (m_setColzRange) histo->getPoly(isU, m_colzMinimum, m_colzMaximum)->Draw("colz l");
    else histo->getPoly(isU)->Draw("colz l");
    drawText();
  }
  setStatusOfCanvas(status, canvasRPhi, false);

  canvasRPhi->Modified();
  canvasRPhi->Update();
}

void DQMHistAnalysisSVDModule::updateErrCanvases(SVDSummaryPlots* histo, TCanvas* canvas, TCanvas* canvasRPhi, bool isU)
{
  canvas->Draw();
  canvas->cd();
  if (histo)
    histo->getHistogram(isU)->Draw("text colz");

  canvas->Modified();
  canvas->Update();

  canvasRPhi->Draw();
  canvasRPhi->cd();
  if (histo) {
    histo->getPoly(isU, 0)->Draw("colz l");
    drawText();
  }

  canvasRPhi->Modified();
  canvasRPhi->Update();
}

pair<vector<TText*>, vector<TText*>> DQMHistAnalysisSVDModule::textModuleNumbers()
{
  vector<TText*> ladders;
  vector<TText*> sensors;

  const double rLayer[4] = {40, 70, 110, 160}; // layer position
  const double nLadders[4] = {7, 10, 12, 16}; // per layer
  const double nSensors[4] = {2, 3, 4, 5}; // per ladder
  const double position[4] = {0.8, 1.2, 1., 0.8}; // text position
  const double delta[4] = {9, 8, 8, 8}; // width of sensr bins
  const double inclination[4] = {-17, -5, -13, -12}; // inclination

  double pi = TMath::Pi();

  for (int layer = 0; layer < 4; layer ++) {
    for (int ladder = 1; ladder <= nLadders[layer]; ladder++) {
      double deltaText = delta[layer] + position[layer];
      double r = rLayer[layer] + (deltaText) * nSensors[layer];
      double phi = 2 * pi / nLadders[layer];
      double dphiThisPoint = (ladder - 1) * phi - phi / 2 + inclination[layer] * pi / 180.;
      double dphiNextPoint = dphiThisPoint + phi;
      double minX = r * TMath::Cos(dphiThisPoint);
      double maxX = (r + deltaText) * TMath::Cos(dphiNextPoint);
      double minY = r * TMath::Sin(dphiThisPoint);
      double maxY = (r + deltaText) * TMath::Sin(dphiNextPoint);

      double xcen = (minX + maxX) / 2.;
      double ycen = (minY + maxY) / 2.;

      double angle = TMath::ATan2(ycen, xcen) * 180. / TMath::Pi() - 90.;
      if (ycen < 0) angle = TMath::ATan2(ycen, xcen) * 180. / TMath::Pi() + 90;

      TText* t = new TText(xcen, ycen, Form("%d.%d", layer + 3, ladder));
      t->SetTextAlign(22);
      t->SetTextAngle(angle);
      t->SetTextSize(0.025);

      ladders.push_back(t);

      for (int sensor = 1; sensor <= nSensors[layer]; sensor++) {
        if ((layer == 0 && ladder == 4) || (layer == 1 && ladder == 5) || (layer == 2 && ladder == 6) || (layer == 3 && ladder == 7)) {
          double rs = rLayer[layer] + (delta[layer]) * (sensor - 1);
          double xcens = rs * TMath::Cos(dphiThisPoint);
          double ycens = rs * TMath::Sin(dphiThisPoint);

          double angles = TMath::ATan2(ycens, xcens) * 180. / pi - 90.;
          if (ycen < 0) angles = TMath::ATan2(ycens, xcens) * 180. / pi + 90;

          TText* ts = new TText(xcens, ycens, Form("%d ", sensor));
          ts->SetTextAlign(31);
          ts->SetTextAngle(angles);
          ts->SetTextSize(0.018);

          sensors.push_back(ts);
        }
      }
    }
  }

  return std::make_pair(ladders, sensors);
}

void DQMHistAnalysisSVDModule::drawText()
{
  m_ly->Draw("same");
  m_lx->Draw("same");
  m_arrowx->Draw();
  m_arrowy->Draw();
  for (int i = 0; i < (int)m_laddersText.size(); i++) m_laddersText[i]->Draw("same");
  for (int i = 0; i < (int)m_sensorsText.size(); i++) m_sensorsText[i]->Draw("same");
}
