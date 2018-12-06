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
#include <ecl/modules/eclDisplay/EclPainter1D.h>

//Root
#include <TPad.h>
#include <TH1.h>

//ECL
#include <ecl/modules/eclDisplay/geometry.h>
#include <ecl/utility/ECLChannelMapper.h>
#include <ecl/modules/eclDisplay/MultilineWidget.h>

using namespace Belle2;
using namespace ECLDisplayUtility;

EclPainter1D::EclPainter1D(EclData* data, EclPainter1D::Type type) :
  EclPainter(data)
{
  m_type = type;

  initHisto();
  m_hist->GetXaxis()->CenterTitle();
  m_hist->GetXaxis()->SetTitleOffset(1.1);
  m_hist->GetYaxis()->SetTitleOffset(1.1);

  m_shaper = -1;
  m_crate = -1;
}

EclPainter1D::~EclPainter1D()
{
  delete m_hist;
}

int EclPainter1D::channelToSegId(int channel)
{
  switch (m_type) {
    case CHANNEL:
      return channel;
    case SHAPER:
      return 12 * getMapper()->getCrateID(channel) +
             getMapper()->getShaperPosition(channel) - 12;
    case CRATE:
      return getMapper()->getCrateID(channel);
    case PHI:
      return getData()->getPhiId(channel);
    case THETA:
      return getData()->getThetaId(channel);
  }

  return 0;
}

int EclPainter1D::getMaxX()
{
  switch (m_type) {
    case CHANNEL:
      return getData()->getCrystalCount();
    case SHAPER:
      return 52 * 12;
    case CRATE:
      return 52;
    case PHI:
      return 144;
    case THETA:
      return 69;
  }

  return 1;
}

void EclPainter1D::initHisto()
{
  char obj_name[255];
  getNewRootObjectName(obj_name, 255);

  m_hist = new TH1F(obj_name, "title", getMaxX() + 1, 0, getMaxX() + 1);
}

void EclPainter1D::setTitles()
{
  // TODO: These extra labels might not be necessary.
  const char* name[3][5] = {
    {
      "Events per channel", "Events per shaper", "Events per collector",
      "Events per phi_id", "Events per theta_id"
    },

    {
      "Energy per channel (MeV)", "Energy per shaper (MeV)", "Energy per collector (MeV)"
      "Energy per phi_id (MeV)", "Energy per theta_id (MeV)"
    },

    {
      "Time per channel (ns)", "Time per shaper (ns)", "Time per collector (ns)",
      "Time per phi_id (ns)", "Time per theta_id (ns)"
    }
  };
  const char* xname[3] = {
    "Channel id", "Shaper id", "Collector id"
  };
  const char* yname[3] = {
    "Events", "Energy", "Time"
  };

  TString title = TString(name[GetMode()][(int)m_type]) + " (" +
                  getSubsystemTitle(getDisplayedSubsystem()) + ")";
  const char* xtitle = xname[(int)m_type];
  const char* ytitle = yname[GetMode()];

  m_hist->SetTitle(title);
  m_hist->SetXTitle(xtitle);
  m_hist->SetYTitle(ytitle);
}

void EclPainter1D::getInformation(int px, int py, MultilineWidget* panel)
{
  EclPainter::getInformation(px, py, panel);

  char info[255];

  Float_t upx = gPad->AbsPixeltoX(px);
  Float_t x   = gPad->PadtoX(upx);
  int binx  = m_hist->GetXaxis()->FindBin(x) - 1;

  if (m_type == CHANNEL) {
    sprintf(info, "channel_id = %d (%d)", binx,
            getMapper()->getShaperChannel(binx));
    panel->setLine(1, info);
    sprintf(info, "shaper_id = %d", getMapper()->getShaperPosition(binx));
    panel->setLine(2, info);
    sprintf(info, "crate_id = %d", getMapper()->getCrateID(binx));
    panel->setLine(3, info);
  }
  if (m_type == SHAPER) {
    sprintf(info, "shaper_id = %d (%d)", binx, (binx - 1) % 12 + 1);
    panel->setLine(1, info);
    sprintf(info, "crate_id = %d", (binx - 1) / 12 + 1);
    panel->setLine(2, info);
  }
  if (m_type == CRATE) {
    sprintf(info, "crate_id = %d", binx);
    panel->setLine(1, info);
  }
  if (m_type == PHI) {
    sprintf(info, "phi_id = %d", binx);
    panel->setLine(1, info);
  }
  if (m_type == THETA) {
    sprintf(info, "theta_id = %d", binx);
    panel->setLine(1, info);
  }
}

EclPainter1D::Type EclPainter1D::getType()
{
  return m_type;
}

EclPainter* EclPainter1D::handleClick(int px, int py)
{
  Float_t upx = gPad->AbsPixeltoX(px);
  Float_t x   = gPad->PadtoX(upx);
  int binx  = m_hist->GetXaxis()->FindBin(x) - 1;

  Float_t upy = gPad->AbsPixeltoY(py);
  Float_t y   = gPad->PadtoY(upy);

  if (y < 0) return NULL;

  if (m_type == SHAPER) {
    EclPainter1D* ret = new EclPainter1D(getData(), CHANNEL);
    ret->setMapper(getMapper());
    ret->setDisplayedSubsystem(getDisplayedSubsystem());
    //ret->setXRange(binx * 16, binx * 16 + 15);
    ret->setShaper((binx - 1) / 12 + 1, (binx - 1) % 12 + 1);
    return ret;
  }
  if (m_type == CRATE) {
    EclPainter1D* ret = new EclPainter1D(getData(), SHAPER);
    ret->setMapper(getMapper());
    ret->setDisplayedSubsystem(getDisplayedSubsystem());
    ret->setXRange((binx - 1) * 12 + 1, (binx - 1) * 12 + 12);
    //ret->setCrate(binx);
    return ret;
  }

  return NULL;
}

void EclPainter1D::setXRange(int xmin, int xmax)
{
  m_hist->GetXaxis()->SetRange(xmin + 1, xmax + 1);
}

void EclPainter1D::setShaper(int crate, int shaper)
{
  m_crate = crate;
  m_shaper = shaper;
}

void EclPainter1D::setCrate(int crate)
{
  m_crate = crate;
}

void EclPainter1D::Draw()
{
  setTitles();

  EclData* data = getData();

  const int* ev_counts = data->getEventCounts();
  const float* energy_sums = data->getEnergySums();

  m_hist->Reset();
  for (int i = 1; i <= getData()->getCrystalCount(); i++) {
    if (!data->isCrystalInSubsystem(i, getDisplayedSubsystem())) continue;

    // If filter is set, display only specified channels.
    if (m_type == CHANNEL && m_crate > 0 && m_shaper > 0) {
      if (m_crate != getMapper()->getCrateID(i) ||
          m_shaper != getMapper()->getShaperPosition(i)) {
        continue;
      }
    }
    if (m_type == SHAPER && m_crate > 0) {
      if (m_crate != getMapper()->getCrateID(i)) {
        continue;
      }
    }
    //
    int id = channelToSegId(i);
    if (GetMode())
      m_hist->Fill(id, energy_sums[i]);
    else
      m_hist->Fill(id, ev_counts[i]);
  }

  m_hist->Draw("HIST");
}
