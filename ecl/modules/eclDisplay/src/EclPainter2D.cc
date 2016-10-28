#include <ecl/modules/eclDisplay/EclPainter2D.h>
#include <TMath.h>
#include <TColor.h>
#include <TStyle.h>
#include <TPad.h>
#include <ecl/modules/eclDisplay/geometry.h>
#include <ecl/modules/eclDisplay/viewer.h>

/// Palette id for the histogram.
#define PALETTE_ID 55

using namespace Belle2;

EclPainter2D::EclPainter2D(EclData* data, EclPainter2D::Type type) :
  EclPainter(data)
{
  this->m_type = type;

  int max_x = GetMaxX();
  int max_y = GetMaxY();
  char obj_name[255];
  GetNewRootObjectName(obj_name, 255);
  m_hist = new TH2F(obj_name, "ECL data", max_x, 0, max_x,
                    max_y, 0, max_y);

  m_hist->GetXaxis()->CenterTitle();
  m_hist->GetXaxis()->SetTitleOffset(1.1);
  m_hist->GetYaxis()->CenterTitle();
  m_hist->GetYaxis()->SetTitleOffset(-1.1);
  m_hist->GetZaxis()->CenterTitle();
  m_hist->GetZaxis()->SetTitle("Energy (MeV)");

  SetTitle();

  grid = 0;
}

EclPainter2D::~EclPainter2D()
{
  delete m_hist;
}

void EclPainter2D::SetTitle()
{
  if (m_type == CHANNEL_2D) {
    m_hist->GetXaxis()->SetTitle("Theta id");
    m_hist->GetYaxis()->SetTitle("Phi id");
  }
  if (m_type == SHAPER_2D) {
    m_hist->GetXaxis()->SetTitle("Shaper id");
    m_hist->GetYaxis()->SetTitle("Collector id");
  }
}

int EclPainter2D::GetMaxX()
{
  if (m_type == CHANNEL_2D)
    return 46;
  else if (m_type == SHAPER_2D)
    return 12;

  return 1;
}
int EclPainter2D::GetMaxY()
{
  if (m_type == CHANNEL_2D)
    return 144;
  else if (m_type == SHAPER_2D)
    return 36;

  return 1;
}

int EclPainter2D::ChannelToSegIdX(int ch)
{
  if (m_type == CHANNEL_2D)
    return GetThetaId(ch);
  else if (m_type == SHAPER_2D)
    return GetShaperId(ch);

  return 0;
}
int EclPainter2D::ChannelToSegIdY(int ch)
{
  if (m_type == CHANNEL_2D)
    return GetPhiId(ch);
  else if (m_type == SHAPER_2D)
    return GetCrateId(ch);

  return 0;
}

void EclPainter2D::GetInformation(int px, int py, MultilineWidget* panel)
{
  EclPainter::GetInformation(px, py, panel);

  char info[255];

  Float_t upx = gPad->AbsPixeltoX(px);
  Float_t x   = gPad->PadtoX(upx);
  int binx  = m_hist->GetXaxis()->FindBin(x);

  Float_t upy = gPad->AbsPixeltoY(py);
  Float_t y   = gPad->PadtoY(upy);
  int biny  = m_hist->GetYaxis()->FindBin(y);

  if (m_type == CHANNEL_2D) {
    sprintf(info, "theta_id = %d", binx);
    panel->SetLine(1, info);
    sprintf(info, "phi_id = %d", biny);
    panel->SetLine(2, info);
    sprintf(info, "channel_id = %d", GetChannel(binx - 1, biny - 1));
    panel->SetLine(3, info);
  }
  if (m_type == SHAPER_2D) {
    sprintf(info, "crate_id = %d", biny - 1);
    panel->SetLine(1, info);
    sprintf(info, "shaper_id = %d", (biny - 1) * 12 + binx - 1);
    panel->SetLine(2, info);
  }
}

EclPainter2D::Type EclPainter2D::GetType()
{
  return m_type;
}

// In case of ECL cylinder net drawing, results are intended
// to be drawn on the histogram with the following properties:
// nbinsx : 46,  xlow : 0, xup : 46
// nbinsy : 144, ylow : 0, yup : 144
void EclPainter2D::Draw()
{
  EclData* data = GetData();

  int id_x, id_y;

  const int* ev_counts = data->GetEventCounts();
  const float* energy_sums = data->GetEnergySums();

  energy_sums = data->GetEnergySums();
  ev_counts = data->GetEventCounts();

  m_hist->Reset();
  for (int i = 0; i < 6912; i++) {
    id_x = ChannelToSegIdX(i);
    id_y = ChannelToSegIdY(i);
    if (GetMode())
      m_hist->Fill(id_x, id_y, energy_sums[i]);
    else
      m_hist->Fill(id_x, id_y, ev_counts[i]);
  }

  // Code for grid drawing. Currently has some issues with the scaling.
//  hgrid = new TH2F("hgrid","",GetMaxX(),0.,GetMaxX(),
//                   GetMaxY(),0.,GetMaxY());
//  hgrid->GetXaxis()->SetTicks("+-");
//  hgrid->GetYaxis()->SetTicks("+-");
//  hgrid->GetXaxis()->SetNdivisions(23, 2, 1);
//  hgrid->GetYaxis()->SetNdivisions(18, 8, 1);
//  hgrid->Draw();

//  grid = new TPad("grid","",0,0,1,1);
//  grid->Draw();
//  grid->cd();
//  grid->SetGrid();
//  grid->SetFillStyle(0);

  gStyle->SetNumberContours(255);
  gStyle->SetPalette(55);
  // gPad->SetLogz();
  // gPad->GetCanvas()->SetGrid();

  m_hist->GetXaxis()->SetTicks("+-");
  m_hist->GetYaxis()->SetTicks("+-");
  // m_hist->GetXaxis()->SetTickLength(0);
  // m_hist->GetYaxis()->SetTickLength(0);
  // m_hist->GetXaxis()->SetNdivisions(46);
  // m_hist->GetYaxis()->SetNdivisions(72);
  // m_hist->GetXaxis()->SetLabelOffset(999.);
  // m_hist->GetYaxis()->SetLabelOffset(999.);

  m_hist->Draw("COLZ");
}
