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
#include <ecl/modules/eclDisplay/EclPainterCommon.h>

//Root
#include <TH1.h>

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

int EclPainterCommon::getMinX()
{
  switch (m_type) {
    case ENERGY:
      return 0;
    case ENERGY_SUM:
      return 0;
    case TIME:
      return -2048;
  }

  return 0;
}

int EclPainterCommon::getMaxX()
{
  switch (m_type) {
    case ENERGY:
      return 150;
    case ENERGY_SUM:
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
    case ENERGY:
      data->fillEnergyHistogram(m_hist, getMinX(), getMaxX(), getDisplayedSubsystem());
      break;
    case ENERGY_SUM:
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
    "Energy per channel",
    "Energy sum per event",
    "Time"
  };
  const char* xname[3] = {
    "Energy (MeV)",
    "Energy (MeV)",
    "Time (ns)"
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
