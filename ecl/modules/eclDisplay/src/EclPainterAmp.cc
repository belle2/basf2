#include <ecl/modules/eclDisplay/EclPainterAmp.h>
#include <TMath.h>
#include <TColor.h>
#include <TPad.h>
#include <ecl/modules/eclDisplay/geometry.h>
#include <ecl/modules/eclDisplay/viewer.h>

using namespace Belle2;

EclPainterAmp::EclPainterAmp(EclData* data, int amp_min, int amp_max) :
  EclPainter(data)
{
  m_amp_min = amp_min;
  m_amp_max = amp_max;

  InitHisto();
  m_hist->GetXaxis()->CenterTitle();
  m_hist->GetXaxis()->SetTitleOffset(1.1);
  m_hist->GetYaxis()->SetTitleOffset(1.1);
}

EclPainterAmp::~EclPainterAmp()
{
  delete m_hist;
}

int EclPainterAmp::GetMaxX()
{
  return m_amp_max - m_amp_min;
}

void EclPainterAmp::InitHisto()
{

  char obj_name[255];
  GetNewRootObjectName(obj_name, 255);
  m_hist = new TH1F(obj_name, "Amplitude histogram", GetMaxX() / 10, m_amp_min, m_amp_max);

  m_hist->SetXTitle("Amplitude");
  m_hist->SetYTitle("");
}

void EclPainterAmp::GetInformation(int px, int py, MultilineWidget* panel)
{
  EclPainter::GetInformation(px, py, panel);
}

void EclPainterAmp::Draw()
{
  EclData* data = GetData();
  //  TTree* tree = data->GetTree();

  m_hist->Reset();
  data->FillAmpHistogram(m_hist, m_amp_min, m_amp_max);

  m_hist->Draw();
}
