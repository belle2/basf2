#include <ecl/modules/eclDisplay/EclPainter1D.h>
#include <TMath.h>
#include <TColor.h>
#include <TPad.h>
#include <ecl/modules/eclDisplay/geometry.h>
#include <ecl/modules/eclDisplay/viewer.h>

using namespace Belle2;

EclPainter1D::EclPainter1D(EclData* data, EclPainter1D::Type type) :
  EclPainter(data)
{
  m_type = type;

  InitHisto();
  m_hist->GetXaxis()->CenterTitle();
  m_hist->GetXaxis()->SetTitleOffset(1.1);
  m_hist->GetYaxis()->SetTitleOffset(1.1);
}

EclPainter1D::~EclPainter1D()
{
  delete m_hist;
}

int EclPainter1D::ChannelToSegId(int channel)
{
  switch (m_type) {
    case CHANNEL:
      return channel;
    case SHAPER:
      return 12 * GetCrateId(channel) + GetShaperId(channel);
    case CRATE:
      return GetCrateId(channel);
  }

  return 0;
}

int EclPainter1D::GetMaxX()
{
  switch (m_type) {
    case CHANNEL:
      return 6912;
    case SHAPER:
      return 36 * 12;
    case CRATE:
      return 36;
  }

  return 1;
}

void EclPainter1D::InitHisto()
{

  const char* name[2][3] = {
    {"Events per channel", "Events per shaper", "Events per collector"},
    {"Energy per channel (MeV)", "Energy per shaper (MeV)", "Energy per collector (MeV)"}
  };
  const char* xname[3] = {
    "Channel id", "Shaper id", "Collector id"
  };
  const char* yname[2] = {
    "Events", "Energy"
  };

  char obj_name[255];
  GetNewRootObjectName(obj_name, 255);
  const char* title = name[GetMode()][(int)m_type];
  const char* xtitle = xname[(int)m_type];
  const char* ytitle = yname[GetMode()];
  m_hist = new TH1F(obj_name, title, GetMaxX(), 0, GetMaxX());


  m_hist->SetXTitle(xtitle);
  m_hist->SetYTitle(ytitle);
}

void EclPainter1D::GetInformation(int px, int py, MultilineWidget* panel)
{
  EclPainter::GetInformation(px, py, panel);

  char info[255];

  Float_t upx = gPad->AbsPixeltoX(px);
  Float_t x   = gPad->PadtoX(upx);
  int binx  = m_hist->GetXaxis()->FindBin(x) - 1;

  if (m_type == CHANNEL) {
    sprintf(info, "channel_id = %d", binx);
    panel->SetLine(1, info);
    sprintf(info, "shaper_id = %d", 12 * GetCrateId(binx) + GetShaperId(binx));
    panel->SetLine(2, info);
    sprintf(info, "crate_id = %d", GetCrateId(binx));
    panel->SetLine(3, info);
  }
  if (m_type == SHAPER) {
    sprintf(info, "shaper_id = %d", binx);
    panel->SetLine(1, info);
    sprintf(info, "crate_id = %d", binx / 12);
    panel->SetLine(2, info);
  }
  if (m_type == CRATE) {
    sprintf(info, "crate_id = %d", binx);
    panel->SetLine(1, info);
  }
}

EclPainter1D::Type EclPainter1D::GetType()
{
  return m_type;
}

EclPainter* EclPainter1D::HandleClick(int px, int py)
{
  Float_t upx = gPad->AbsPixeltoX(px);
  Float_t x   = gPad->PadtoX(upx);
  int binx  = m_hist->GetXaxis()->FindBin(x) - 1;

  Float_t upy = gPad->AbsPixeltoY(py);
  Float_t y   = gPad->PadtoY(upy);

  if (y < 0) return NULL;

  if (m_type == SHAPER) {
    EclPainter1D* ret = new EclPainter1D(GetData(), CHANNEL);
    ret->SetXRange(binx * 16, binx * 16 + 15);
    return ret;
  }
  if (m_type == CRATE) {
    EclPainter1D* ret = new EclPainter1D(GetData(), SHAPER);
    ret->SetXRange(binx * 12, binx * 12 + 11);
    return ret;
  }

  return NULL;
}

void EclPainter1D::SetXRange(int xmin, int xmax)
{
  m_hist->GetXaxis()->SetRange(xmin + 1, xmax + 1);
}

void EclPainter1D::Draw()
{
  EclData* data = GetData();
  int id;

  const int* ev_counts = data->GetEventCounts();
  const float* energy_sums = data->GetEnergySums();

  energy_sums = data->GetEnergySums();
  ev_counts = data->GetEventCounts();

  m_hist->Reset();
  for (int i = 0; i < 6912; i++) {
    id = ChannelToSegId(i);
    if (GetMode())
      m_hist->Fill(id, energy_sums[i]);
    else
      m_hist->Fill(id, ev_counts[i]);
  }

  m_hist->Draw();
}
