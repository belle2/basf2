/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/

#include <ecl/modules/eclDisplay/EclPainterCommon.h>
#include <ecl/modules/eclDisplay/geometry.h>

using namespace Belle2;

EclPainterCommon::EclPainterCommon(EclData* data, Type type) :
  EclPainter(data)
{
  m_type = type;

  initHisto();
  m_hist->GetXaxis()->CenterTitle();
  m_hist->GetXaxis()->SetTitleOffset(1.1);
  m_hist->GetYaxis()->SetTitleOffset(1.1);
}

EclPainterCommon::~EclPainterCommon()
{
  delete m_hist;
}

int EclPainterCommon::getBinCount()
{
  switch (m_type) {
    case AMP:
      return 295;
    case AMP_SUM:
      return 50;
    case TIME:
      return 128;
  }

  return 1;
}

int EclPainterCommon::getMinX()
{
  switch (m_type) {
    case AMP:
      return 0;
    case AMP_SUM:
      return 0;
    case TIME:
      return -2048;
  }

  return 0;
}

int EclPainterCommon::getMaxX()
{
  switch (m_type) {
    case AMP:
      return 150;
    case AMP_SUM:
      return 2500;
    case TIME:
      return 2048;
  }

  return 0;
}

void EclPainterCommon::initHisto()
{

  char obj_name[255];
  getNewRootObjectName(obj_name, 255);
  m_hist = new TH1F(obj_name, "title", getMaxX() / 10,
                    getMinX(), getMaxX());
}

void EclPainterCommon::getInformation(int px, int py, MultilineWidget* panel)
{
  EclPainter::getInformation(px, py, panel);
}

void EclPainterCommon::Draw()
{
  EclData* data = getData();

  setTitles();
  m_hist->Reset();

  switch (getType()) {
    case AMP:
      data->fillEnergyHistogram(m_hist, getMinX(), getMaxX(), getDisplayedSubsystem());
      break;
    case AMP_SUM:
      data->fillEnergySumHistogram(m_hist, getMinX(), getMaxX(), getDisplayedSubsystem());
      break;
    case TIME:
      data->fillTimeHistogram(m_hist, getMinX(), getMaxX(), getDisplayedSubsystem());
      break;
  }

  m_hist->Draw();
}


void Belle2::EclPainterCommon::setTitles()
{
  const char* name[3] = {
    "Amplitude per channel",
    "Amplitude sum per event",
    "Time"
  };
  const char* xname[3] = {
    "Amplitude (ADC counts)",
    "Amplitude (ADC counts)",
    "Time (ADC time)"
  };

  int type = (int)getType();

  TString title = TString(name[type]) + " (" +
                  getSubsystemTitle(getDisplayedSubsystem()) + ")";
  m_hist->SetTitle(title);

  m_hist->SetXTitle(xname[type]);
  m_hist->SetYTitle("");
}

EclPainterCommon::Type EclPainterCommon::getType()
{
  return m_type;
}
