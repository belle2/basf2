/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/

#include <ecl/modules/eclDisplay/EclPainter2D.h>
#include <TMath.h>
#include <TColor.h>
#include <TStyle.h>
#include <TPad.h>
#include <TCanvas.h>
#include <ecl/modules/eclDisplay/geometry.h>

/// Palette id for the histogram.
#define PALETTE_ID 55

using namespace Belle2;
using namespace ECLDisplayUtility;

EclPainter2D::EclPainter2D(EclData* data, EclPainter2D::Type type) :
  EclPainter(data)
{
  this->m_type = type;

  int max_x = getMaxX();
  int max_y = getMaxY();
  char obj_name[255];
  getNewRootObjectName(obj_name, 255);

  m_hist = new TH2F(obj_name, "title", max_x, 1, max_x + 1,
                    max_y, 1, max_y + 1);

  m_hist->GetXaxis()->CenterTitle();
  m_hist->GetXaxis()->SetTitleOffset(1.1);
  m_hist->GetYaxis()->CenterTitle();
  m_hist->GetYaxis()->SetTitleOffset(-1.1);
  m_hist->GetZaxis()->CenterTitle();

  grid = 0;
  hgrid = 0;
}

EclPainter2D::~EclPainter2D()
{
  delete m_hist;
  delete grid;
  if (hgrid)
    delete hgrid;
}

void EclPainter2D::setTitles()
{
  const char* name[3][3] = {
    {"Events per channel", "Events per shaper"},
    {"Energy per channel (MeV)", "Energy per shaper (MeV)"},
    {"Time per channel (ns)", "Time per shaper (ns)"}
  };
  const char* zname[3] = {
    "Events", "Energy (MeV)", "Time"
  };

  TString title = TString(name[GetMode()][(int)m_type]) + " (" +
                  getSubsystemTitle(getDisplayedSubsystem()) + ")";

  m_hist->SetTitle(title);

  if (m_type == CHANNEL_2D) {
    m_hist->GetXaxis()->SetTitle("Theta id");
    m_hist->GetYaxis()->SetTitle("Phi id");
  }
  if (m_type == SHAPER_2D) {
    m_hist->GetXaxis()->SetTitle("Shaper id");
    m_hist->GetYaxis()->SetTitle("Collector id");
  }
  m_hist->GetZaxis()->SetTitle(zname[GetMode()]);
}

int EclPainter2D::getMaxX()
{
  if (m_type == CHANNEL_2D)
    return 68;
  else if (m_type == SHAPER_2D)
    return 12;

  return 1;
}
int EclPainter2D::getMaxY()
{
  if (m_type == CHANNEL_2D)
    return 144;
  else if (m_type == SHAPER_2D)
    return 52;

  return 1;
}

int EclPainter2D::channelToSegIdX(int ch)
{
  if (m_type == CHANNEL_2D)
    return getData()->getThetaId(ch);
  else if (m_type == SHAPER_2D)
    return getMapper()->getShaperPosition(ch);

  return 0;
}
int EclPainter2D::channelToSegIdY(int ch)
{
  if (m_type == CHANNEL_2D)
    return getData()->getPhiId(ch);
  else if (m_type == SHAPER_2D)
    return getMapper()->getCrateID(ch);

  return 0;
}

void EclPainter2D::initGrid()
{
  /* Adding second TPad (layer) for the grid overlay */
  grid = new TPad("grid", "", 0, 0, 1, 1);
  grid->SetGrid();
  grid->SetRightMargin(gPad->GetRightMargin());
  // Setting transparent fill style.
  grid->SetFillStyle(4000);
  grid->SetFrameFillStyle(0);
}

void EclPainter2D::drawGrid()
{
  TVirtualPad* main = gPad;
  grid->Draw("COLZ");
  grid->cd();

  /* Creating grid */
  char obj_name[255];
  getNewRootObjectName(obj_name, 255);

  // NOTE: Root can't divide the histogram axes into more than 99
  // primary sections, limiting the maximum grid cell size.
  // To change this, new grid implementation might be necessary.
  double max_y = getMaxY();
  // Reducing the number of grid lines.
  while (max_y >= 100) {
    max_y /= 2;
  }

  hgrid = new TH2C(obj_name, "", getMaxX(), 0, getMaxX(), max_y, 0, max_y);
  hgrid->GetXaxis()->SetNdivisions(getMaxX());
  hgrid->GetYaxis()->SetNdivisions(max_y);
  // Hiding axis labels.
  hgrid->GetYaxis()->SetLabelOffset(1e3);
  hgrid->GetXaxis()->SetLabelOffset(1e3);
  // Hiding axis ticks.
  hgrid->GetYaxis()->SetTickLength(0.);
  hgrid->GetXaxis()->SetTickLength(0.);

  hgrid->Draw();

  main->cd();
}

void EclPainter2D::getInformation(int px, int py, MultilineWidget* panel)
{
  EclPainter::getInformation(px, py, panel);

  char info[255];

  Float_t upx = gPad->AbsPixeltoX(px);
  Float_t x   = gPad->PadtoX(upx);
  int binx  = m_hist->GetXaxis()->FindBin(x);

  Float_t upy = gPad->AbsPixeltoY(py);
  Float_t y   = gPad->PadtoY(upy);
  int biny  = m_hist->GetYaxis()->FindBin(y);

  if (m_type == CHANNEL_2D) {
    sprintf(info, "theta_id = %d", binx);
    panel->setLine(1, info);
    sprintf(info, "phi_id = %d", biny);
    panel->setLine(2, info);
    sprintf(info, "channel_id = %d", getData()->getChannel(biny, binx));
    panel->setLine(3, info);
  }
  if (m_type == SHAPER_2D) {
    sprintf(info, "crate_id = %d", biny);
    panel->setLine(1, info);
    sprintf(info, "shaper_id = %d (%d)", (biny - 1) * 12 + binx, binx);
    panel->setLine(2, info);
  }
}

EclPainter2D::Type EclPainter2D::getType()
{
  return m_type;
}

// In case of ECL cylinder net drawing, results are intended
// to be drawn on the histogram with the following properties:
// nbinsx : 46,  xlow : 0, xup : 46
// nbinsy : 144, ylow : 0, yup : 144
void EclPainter2D::Draw()
{
  setTitles();

  EclData* data = getData();

  const int* ev_counts = data->getEventCounts();
  const float* energy_sums = data->getEnergySums();

  m_hist->Reset();
  for (int i = 1; i <= getData()->getCrystalCount(); i++) {
    if (!data->isCrystalInSubsystem(i, getDisplayedSubsystem())) continue;

    int id_x = channelToSegIdX(i);
    int id_y = channelToSegIdY(i);
    if (GetMode())
      m_hist->Fill(id_x, id_y, energy_sums[i]);
    else
      m_hist->Fill(id_x, id_y, ev_counts[i]);
  }

  gStyle->SetNumberContours(255);
  gStyle->SetPalette(PALETTE_ID);

  m_hist->GetXaxis()->SetTicks("+-");
  m_hist->GetYaxis()->SetTicks("+-");

  m_hist->Draw("COLZ");

  initGrid();
  drawGrid();
}
