#include <ecl/modules/eclDisplay/EclPainterPolar.h>
#include <TMath.h>
#include <TColor.h>
#include <ecl/modules/eclDisplay/geometry.h>
#include <ecl/modules/eclDisplay/viewer.h>

using namespace Belle2;

EclPainterPolar::EclPainterPolar(EclData* data, EclPainterPolar::Type type) :
  EclPainter(data)
{
  m_type = type;

  const char* name[2][2] = {
    {"Events per phi", "Events per theta"},
    {"Energy per phi", "Energy per theta"}
  };
  const char* zname[2] =
  {"Event count", "Energy (MeV)"};

  char obj_name[255];
  GetNewRootObjectName(obj_name, 255);
  m_hist = new TH2F(obj_name, name[GetMode()][m_type],
                    60, 0.0, 1.0, 60, 0.0, 1.0);
  m_hist->SetZTitle(zname[GetMode()]);
  m_hist->GetZaxis()->CenterTitle();

  Double_t deg2rad = TMath::Pi() / 180;

  m_segs = new TCrown*[36];
  m_labels = new TText*[36];
  char label_txt[32];
  for (int i = 0; i < 36; i++) {
    m_segs[i] = new TCrown(0.5, 0.5, 0.3, 0.4, (i - 9) * 10, (i - 8) * 10);

    float x = 0.475 + 0.45 * TMath::Cos(deg2rad * (i - 9) * 10);
    float y = 0.48 + 0.44 * TMath::Sin(deg2rad * (i - 9) * 10);
    sprintf(label_txt, "%d", i * 10);
    m_labels[i] = new TText(x, y, label_txt);
    m_labels[i]->SetTextSize(0.03);
  }
}

EclPainterPolar::~EclPainterPolar()
{
  delete m_hist;
}

void EclPainterPolar::InitHisto()
{

}

int EclPainterPolar::ChannelToSegId(int ch)
{
  if (m_type == PHI)
    return GetPhiId(ch) / 4;
  else if (m_type == THETA) {
    int theta_id = GetThetaId(ch);

    if (theta_id < 23)
      return 3 + theta_id * 12 / 23;
    else
      return 21 + (theta_id - 23) * 12 / 23;
  }

  return 0;
}

void EclPainterPolar::GetInformation(int px, int py, MultilineWidget* panel)
{
  EclPainter::GetInformation(px, py, panel);
}

EclPainterPolar::Type EclPainterPolar::GetType()
{
  return m_type;
}

void EclPainterPolar::Draw()
{
  EclData* data = GetData();
  const int* ev_counts = data->GetEventCounts();
  const float* energy_sums = data->GetEnergySums();
  int id;

  float seg_val[36];
  TCrown** segs = m_segs;
  for (int i = 0; i < 36; i++)
    seg_val[i] = 0;

  for (int i = 0; i < 6912; i++) {
    id = ChannelToSegId(i);
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
  float val;

  int palette = TColor::CreateGradientColorTable(5, stop, r, g, b, 37);
  m_hist->Reset();
  m_hist->Fill(0.5, 0.05, 0.1);
  m_hist->SetMaximum(max);
  m_hist->SetMinimum(min);
  m_hist->Draw("COLZ");

  for (int i = 0; i < 36; i++) {
    val = 36.0 * TMath::Log(1 + seg_val[i]) / TMath::Log(1 + max);
    segs[i]->SetFillColor(palette + val);
    segs[i]->Draw(/*"SAME"*/);
    m_labels[i]->Draw();
  }
}
