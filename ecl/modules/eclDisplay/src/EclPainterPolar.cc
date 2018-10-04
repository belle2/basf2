/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/
//This module
#include <ecl/modules/eclDisplay/EclPainterPolar.h>

//Root
#include <TMath.h>
#include <TColor.h>
#include <TH2.h>
#include <TCrown.h>
#include <TText.h>

//ECL
#include <ecl/modules/eclDisplay/geometry.h>


using namespace Belle2;
using namespace ECLDisplayUtility;

EclPainterPolar::EclPainterPolar(EclData* data, EclPainterPolar::Type type) :
  EclPainter(data)
{
  m_type = type;

  char obj_name[255];
  getNewRootObjectName(obj_name, 255);
  m_hist = new TH2F(obj_name, "title",
                    60, 0.0, 1.0, 60, 0.0, 1.0);

  Double_t deg2rad = TMath::Pi() / 180;

  m_segs = new TCrown*[36];
  m_labels = new TText*[36];
  char label_txt[32];
  for (int i = 0; i < 36; i++) {
    m_segs[i] = new TCrown(0.5, 0.5, 0.3, 0.4, (i - 9) * 10, (i - 8) * 10);

    float x = 0.475 + 0.45 * TMath::Cos(deg2rad * (i - 9) * 10);
    float y = 0.48 + 0.44 * TMath::Sin(deg2rad * (i - 9) * 10);
    snprintf(label_txt, 32, "%d", i * 10);
    m_labels[i] = new TText(x, y, label_txt);
    m_labels[i]->SetTextSize(0.03);
  }
}

EclPainterPolar::~EclPainterPolar()
{
  delete m_hist;
}

void EclPainterPolar::initHisto()
{

}

int EclPainterPolar::channelToSegId(int ch)
{
  if (m_type == PHI)
    return getData()->getPhiId(ch) / 4;
  else if (m_type == THETA) {
    int theta_id = getData()->getThetaId(ch);

    if (theta_id < 23)
      return 3 + theta_id * 12 / 23;
    else
      return 21 + (theta_id - 23) * 12 / 23;
  }

  return 0;
}

void EclPainterPolar::setTitles()
{
  const char* name[2][2] = {
    {"Events per phi", "Events per theta"},
    {"Energy per phi", "Energy per theta"}
  };
  const char* zname[2] = {"Event count", "Energy (MeV)"};

  TString title = TString(name[GetMode()][m_type]) + " (" +
                  getSubsystemTitle(getDisplayedSubsystem()) + ")";

  m_hist->SetTitle(title);
  m_hist->SetZTitle(zname[GetMode()]);
  m_hist->GetZaxis()->CenterTitle();
}

void EclPainterPolar::getInformation(int px, int py, MultilineWidget* panel)
{
  EclPainter::getInformation(px, py, panel);
}

EclPainterPolar::Type EclPainterPolar::getType()
{
  return m_type;
}

void EclPainterPolar::Draw()
{
  setTitles();

  EclData* data = getData();
  const int* ev_counts = data->getEventCounts();
  const float* energy_sums = data->getEnergySums();

  float seg_val[36];
  TCrown** segs = m_segs;
  for (int i = 0; i < 36; i++)
    seg_val[i] = 0;

  for (int i = 1; i <= getData()->getCrystalCount(); i++) {
    if (!data->isCrystalInSubsystem(i, getDisplayedSubsystem())) continue;
    int id = channelToSegId(i);
    if (GetMode())
      seg_val[id] += energy_sums[i];
    else
      seg_val[id] += ev_counts[i];
  }

  float max = 0;
  float min = seg_val[0];
  for (int i = 0; i < 36; i++) {
    if (max < seg_val[i])
      max = seg_val[i];
    if (min > seg_val[i])
      min = seg_val[i];
  }

  Double_t r[5]    = { 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t g[5]    = { 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t b[5]    = { 0.51, 1.00, 0.12, 0.00, 0.00 };
  Double_t stop[5] = { 0.00, 0.34, 0.61, 0.84, 1.00 };

  int palette = TColor::CreateGradientColorTable(5, stop, r, g, b, 37);
  m_hist->Reset();
  m_hist->Fill(0.5, 0.05, 0.1);
  m_hist->SetMaximum(max);
  m_hist->SetMinimum(min);
  m_hist->Draw("COLZ");

  for (int i = 0; i < 36; i++) {
    float val = 36.0 * TMath::Log(1 + seg_val[i]) / TMath::Log(1 + max);
    segs[i]->SetFillColor(palette + val);
    segs[i]->Draw(/*"SAME"*/);
    m_labels[i]->Draw();
  }
}
